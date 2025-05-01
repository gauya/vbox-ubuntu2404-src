const { contextBridge, ipcRenderer } = require('electron');

// 안전한 IPC 통신을 위한 유효 채널 목록
const validChannels = [
  'database-query',
  'update-database',
  'play-mp3'
];

contextBridge.exposeInMainWorld('electronAPI', {
  // 데이터베이스 관련 API
  queryDatabase: (queryOptions) => {
    if (!queryOptions || typeof queryOptions !== 'object') {
      throw new Error('Invalid query options');
    }
    return ipcRenderer.invoke('database-query', queryOptions);
  },
  
  updateDatabase: (updates) => {
    if (!Array.isArray(updates)) {
      console.log('preload: updateDatabase 호출됨', updates);
      throw new Error('Updates must be an array');
    }
    return ipcRenderer.invoke('update-database', updates);
  },

  // 파일 관련 API
  playMp3: (filename) => {
    if (typeof filename !== 'string') {
      throw new Error('Filename must be a string');
    }
    return ipcRenderer.invoke('play-mp3', filename);
  },
  
  openFile: (filePath) => {
    if (typeof filePath !== 'string') {
      throw new Error('File path must be a string');
    }
    return ipcRenderer.invoke('open-file', filePath);
  },

  // 시스템 정보 API
  requestSystemInfo: (command) => {
    if (typeof command !== 'string') {
      throw new Error('Command must be a string');
    }
    return ipcRenderer.invoke('system-info-request', command);
  },

  // 메시지 전송 API (보안 강화)
  sendMessage: (channel, data) => {
    if (!validChannels.includes(channel)) {
      throw new Error(`Invalid IPC channel: ${channel}`);
    }
    ipcRenderer.send(channel, data);
  },
  
  onReceiveMessage: (channel, callback) => {
    if (!validChannels.includes(channel)) {
      throw new Error(`Invalid IPC channel: ${channel}`);
    }
    ipcRenderer.on(channel, (event, ...args) => callback(...args));
  }
});

// 개발용 로깅 (프로덕션에서는 제거)
console.log("Preload script initialized");
console.log("Available IPC channels:", validChannels);
