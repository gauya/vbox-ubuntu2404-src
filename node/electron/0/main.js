const { app, BrowserWindow, ipcMain,shell } = require('electron');
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
      return `encode(cover_image, 'base64') as cover_image`; // Base64로 인코딩
    } else {
      return column;
    }
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

// 데이터 업데이트 핸들러 (중요!)
ipcMain.handle('update-database', async (event, updates) => {
  console.log("update event :",updates);

  const client = await pool.connect();
  try {
//    await client.query('BEGIN'); // 트랜잭션 시작

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

       try {
        await client.query(query, params);
       } catch(e) {
         console.log(`ERROR!  Update(main.rs)`, e); 
       } finally {
         console.log(`Update(main.rs) ${update.column} ${update.id} : `,query,params);
       }
    }
//    await client.query('COMMIT'); // 트랜잭션 커밋
    return { success: true, message: `${updates.length} records updated` };
  } catch (error) {
    await client.query('ROLLBACK'); // 에러 시 롤백
    console.error('데이터베이스 업데이트 오류:', error);
    return { success: false, error: error.message };
  } finally {
    client.release();
    //mainWindow.webContents.send('update :', query);
  }
});

ipcMain.handle('play-mp3', async (event, filename) => {
  try {
    const mp3Path = path.join(__dirname, 'file/'+filename); // 수정: __dirname 사용
    console.log(mp3Path);

    // 파일 존재 여부 확인
    if (!fs.existsSync(mp3Path)) {
      console.error('파일이 존재하지 않습니다:', mp3Path);
      return { success: false, error: 'File not found' };
    }

    // 기본 음악 플레이어로 파일 열기
    try{
      await shell.openPath(mp3Path);
      console.log('MP3 재생 성공:', mp3Path);
      return { success: true };
    }
    catch(error){
      console.error('MP3 재생 실패:', error);
      return {success: false, error: error.message}
    }

  } catch (error) {
    console.error('MP3 재생 오류:', error);
    return { success: false, error: error.message };
  }
});

