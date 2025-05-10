const config = require('./configManager');

// 데이터베이스 설정 전체 가져오기
const dbConfig = config.getSection('database');
console.log(dbConfig);

// 특정 값만 가져오기
const dbHost = config.get('database', 'host');
console.log(dbHost);

// 웹 서버 설정 가져오기
const serverPort = config.get('server', 'port') || 3000;
