//const { app } = require('electron');

const { app, BrowserWindow } = require('electron');
const path = require('path');

console.log('Electron 버전:', app.getVersion());
console.log('Chrome 버전:', process.versions.chrome);
console.log('Node.js 버전:', process.versions.node);

function createWindow() {
  // 새로운 브라우저 창 생성
  const mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'), // (선택 사항) preload 스크립트 경로
    },
  });

  // index.html 파일 로드
  mainWindow.loadFile('index.html');

  // 개발자 도구 열기 (선택 사항)
  // mainWindow.webContents.openDevTools();
}

// Electron 앱이 준비되면 createWindow 함수 호출
app.whenReady().then(createWindow);

// 모든 윈도우가 닫히면 앱 종료 (macOS 제외)
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

// 앱이 활성화될 때 새로운 윈도우 생성 (macOS)
app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
