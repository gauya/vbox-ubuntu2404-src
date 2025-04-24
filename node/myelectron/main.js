const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { Pool } = require('pg');

// 데이터베이스 연결 설정
const pool = new Pool({
  user: 'gau',
  host: 'localhost',
  database: 'gau',
  password: 'qjemf', // 필요하다면 비밀번호를 입력하세요
  port: 5432, // 기본 PostgreSQL 포트
});

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1524,
    height: 1068,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
    },
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

ipcMain.handle('database-query', async (event, whereClause) => {
  const query = `SELECT title, artist, album, to_char(release_date,'YYYY') as year,sample_rate FROM mp3_schema.mp3_library`;
  
  if (whereClause && whereClause.trim() !== '') {
    query += ` WHERE ${whereClause}`;
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
