const { app, BrowserWindow, ipcMain, shell } = require('electron');
const path = require('path');
const { Pool } = require('pg');
const fs = require('fs');

// 데이터베이스 연결 설정
const pool = new Pool({
  user: process.env.DATABASE_USER || 'postgres',
  host: process.env.DATABASE_HOST || 'localhost',
  database: process.env.DATABASE_NAME || 'gau',
  password: process.env.DATABASE_PASSWORD,
  port: process.env.DATABASE_PORT || 5432,
});

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1524,
    height: 1068,
    //minWidth: 800,
    //minHeight: 600,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
      //contentSecurityPolicy: "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline';", // 안전하지 않은 Inline 스타일 허용 (개발 편의)
    },
    resizable: true,
    movable: true,
  });

  mainWindow.loadFile('index.html');

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

// 데이터베이스 쿼리 핸들러
ipcMain.handle('database-query', async (event, queryOptions) => {
  const { columns, where, orderBy, orderDirection, limit } = queryOptions;
  
  const selectedColumns = columns.map(column => {
    if (column === 'year') {
      return `to_char(release_date,'YYYY') as year`;
    } else if (column === 'release_date') {
      return `to_char(release_date,'YYYY-MM-DD') as release_date`;
    } else if (column === 'duration') {
      return `to_char(duration, 'HH24:MI:SS') as duration`;
    } else if (column === 'cover_image') {
      return `encode(cover_image, 'base64') as cover_image`;
    } else {
      return column;
    }
  }).join(', ');

  let query = `SELECT id,${selectedColumns} FROM mp3_schema.mp3_library`;

  if (where && where.trim() !== '') {
    query += ` WHERE ${where}`;
  }

  let orderByClause = '';
  if (orderBy && orderBy !== '') {
    let orderByColumn = orderBy;
    if (orderBy === 'year') {
      orderByColumn = `to_char(release_date,'YYYY')`;
    } 
    orderByClause = ` ORDER BY ${orderByColumn} ${orderDirection || 'ASC'}`;
    query += orderByClause;
  }

  if (limit && limit.trim() !== '') {
    query += ` LIMIT ${limit}`;
  }

  try {
    const client = await pool.connect();
    const result = await client.query(query);
    client.release();
    return result.rows;
  } catch (error) {
    console.error('데이터베이스 쿼리 오류:', error);
    return null;
  }
});

// 데이터 업데이트 핸들러 (중요!)
ipcMain.handle('update-database', async (event, updates) => {
  console.log("update event :",updates);
  
  const client = await pool.connect();
  try {
    await client.query('BEGIN'); // 트랜잭션 시작

    // 여러 업데이트를 한 번에 처리
    for (const update of updates) {
      let query;
      let params = [update.value, update.id];
      
      // 컬럼 타입에 따라 다른 쿼리 생성
      if (update.column === 'release_date') {
        query = `UPDATE mp3_schema.mp3_library SET ${update.column} = TO_DATE($1, 'YYYY-MM-DD') WHERE id = $2`;
      } else if (update.column === 'duration') {
        query = `UPDATE mp3_schema.mp3_library SET ${update.column} = $1::interval WHERE id = $2`;
      } else {
        query = `UPDATE mp3_schema.mp3_library SET ${update.column} = $1 WHERE id = $2`;
      }
      
      await client.query(query, params);
      console.log(`Updated ${update.column} for record ${update.id}`);
    }

    await client.query('COMMIT'); // 트랜잭션 커밋
    return { success: true, message: `${updates.length} records updated` };
  } catch (error) {
    await client.query('ROLLBACK'); // 에러 시 롤백
    console.error('데이터베이스 업데이트 오류:', error);
    return { success: false, error: error.message };
  } finally {
    client.release();
  }
});

// MP3 재생 핸들러
ipcMain.handle('play-mp3', async (event, filename) => {
  try {
    const mp3Path = path.join('file', filename);
    
    if (!fs.existsSync(mp3Path)) {
      console.error('파일이 존재하지 않습니다:', mp3Path);
      return { success: false, error: 'File not found' };
    }

    await shell.openPath(mp3Path);
    return { success: true };
  } catch (error) {
    console.error('MP3 재생 오류:', error);
    return { success: false, error: error.message };
  }
});

// 파일 열기 핸들러
ipcMain.handle('open-file', async (event, filePath) => {
  try {
    await shell.openPath(filePath);
    return { success: true };
  } catch (error) {
    console.error('파일 열기 오류:', error);
    return { success: false, error: error.message };
  }
});
