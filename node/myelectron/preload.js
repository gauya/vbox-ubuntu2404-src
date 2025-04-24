// preload.js
const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  sendMessage: (channel, data) => ipcRenderer.send(channel, data),
  onReceiveMessage: (channel, callback) => ipcRenderer.on(channel, callback),
  requestSystemInfo: (command) => ipcRenderer.invoke('system-info-request', command),
  queryDatabase: (whereClause) => ipcRenderer.invoke('database-query', whereClause), // DB 쿼리 요청
});
