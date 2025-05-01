// preload.js
const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  sendMessage: (channel, data) => ipcRenderer.send(channel, data),
  onReceiveMessage: (channel, callback) => ipcRenderer.on(channel, callback),
  requestSystemInfo: (command) => ipcRenderer.invoke('system-info-request', command),
  queryDatabase: (queryOptions) => ipcRenderer.invoke('database-query', queryOptions), // 쿼리 옵션 객체 전달
  playMp3: (filename) => ipcRenderer.invoke('play-mp3', filename),
  //checkFileExists: (filePath) => ipcRenderer.invoke('check-file-exists', filePath),
  openFile: (filePath) => ipcRenderer.invoke('open-file', filePath),
});

console.log("Running on:", process.platform);
console.log("Session type:", process.env.XDG_SESSION_TYPE);
console.log("Ozone platform:", process.argv.includes('--ozone-platform=x11') ? 'X11' : 'Unknown');

