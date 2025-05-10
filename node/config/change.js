const config = require('./configManager');

// 단일 값 수정
config.set('database', 'host', 'new.db.server.com');

// 여러 값 수정
config.set('server', 'port', '8080');
config.set('server', 'timeout', '30000');

// 수정 후 리로드 없이 바로 반영됨
