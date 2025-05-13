const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    selectMp3File: () => ipcRenderer.invoke('select-mp3-file'),
    getMetadata: (filePath) => ipcRenderer.invoke('get-metadata', filePath),
    updateMetadata: (filePath, metadata) => ipcRenderer.invoke('update-metadata', filePath, metadata),
});
