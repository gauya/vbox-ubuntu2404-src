const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const axios = require('axios');

let mainWindow;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
            preload: path.join(__dirname, 'preload.js')
        }
    });

    mainWindow.loadFile('index.html');
    mainWindow.webContents.openDevTools(); // 개발자 도구 열기

    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

app.on('ready', createWindow);

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('activate', () => {
    if (mainWindow === null) {
        createWindow();
    }
});

ipcMain.handle('select-mp3-file', async () => {
    try {
    const { filePaths } = await dialog.showOpenDialog({
        filters: [{ name: 'MP3 Files', extensions: ['mp3'] }]
    });

    } catch (e) {
        console.error('select mp3', e);
    } finally {
        console.log('select-mp3-file');
    }
    return filePaths[0];
});

ipcMain.handle('get-metadata', async (event, filePath) => {
    try {
        const response = await axios.get(`http://localhost:5000/metadata/${encodeURIComponent(filePath)}`);
        return response.data;
    } catch (error) {
        console.error('메타데이터 가져오기 오류:', error.response ? error.response.data : error.message);
        return { error: error.response ? error.response.data.error : error.message };
    }
});

ipcMain.handle('update-metadata', async (event, filePath, metadata) => {
    try {
        const response = await axios.post(`http://localhost:5000/metadata/${encodeURIComponent(filePath)}`, metadata);
        return response.data;
    } catch (error) {
        console.error('메타데이터 업데이트 오류:', error.response ? error.response.data : error.message);
        return { error: error.response ? error.response.data.error : error.message };
    }
});
