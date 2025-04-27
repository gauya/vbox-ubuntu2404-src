const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { Pool } = require('pg');
const fs = require('fs'); // fs 모듈 추가

// 데이터베이스 연결 설정 (기존 설정 유지)
const pool = new Pool({
  user: process.env.DATABASE_USER || 'postgres',
  host: process.env.DATABASE_HOST || 'localhost',
  database: process.env.DATABASE_NAME || 'gau',
  password: process.env.DATABASE_PASSWORD,
  port: process.env.DATABASE_PORT || 5432,
});

let mainWindow;

function createWindow() {
  //console.log('createWindow 호출됨');
  mainWindow = new BrowserWindow({
    width: 1524,
    height: 1068,
    minWidth: 800,  // 최소 너비
    minHeight: 600, // 최소 높이
    //maxWidth: 2000, // 최대 너비 (필요한 경우)
    //maxHeight: 1500, // 최대 높이 (필요한 경우)    
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
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
      return `encode(cover_image, 'base64') as cover_image`; // Base64로 인코딩
    } else {
      return column;
    }

//    return column === 'year' ? `to_char(release_date,'YYYY') as year` : column;
  }).join(', ');

  let query = `SELECT ${selectedColumns} FROM mp3_schema.mp3_library`;

  if (where && where.trim() !== '') {
    query += ` WHERE ${where}`;
  }

    let orderByClause = ''
  if (orderBy && orderBy !== '') {
    let orderByColumn = orderBy;
    if ( orderBy === 'year') {
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
  } finally {
    console.log(orderBy + ':' + query);
    mainWindow.webContents.send('query :', query);
  }
});

ipcMain.handle('play-mp3', async (event, filename) => {
  try {
    const mp3Path = path.join('/db/mp3/file', filename);
    console.log(mp3Path);

    // 파일 존재 여부 확인
    if (!fs.existsSync(mp3Path)) {
      console.error('파일이 존재하지 않습니다:', mp3Path);
      return { success: false, error: 'File not found' };
    }

    // 기본 음악 플레이어로 파일 열기
    await shell.openPath(mp3Path);

    console.log('MP3 재생 성공:', mp3Path);
    return { success: true };
  } catch (error) {
    console.error('MP3 재생 오류:', error);
    return { success: false, error: error.message };
  }
});

ipcMain.handle('open-file', async (event, filePath) => { // 추가
  try {
    await shell.openPath(filePath);
    return { success: true };
  } catch (error) {
    console.error('파일 열기 오류:', error);
    return { success: false, error: error.message };
  }
});
