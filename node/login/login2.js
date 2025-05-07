const fs = require('fs');
const { Client } = require('pg');

const config = JSON.parse(fs.readFileSync('.config.json', 'utf8'));
const dbConfig = config.database;

const client = new Client({
  host: dbConfig.host,
  port: dbConfig.port,
  database: dbConfig.name,
  user: dbConfig.user,
  password: dbConfig.password,
});

client.connect()
  .then(() => console.log('DB 연결 성공!'))
  .catch(err => console.error('연결 실패:', err))
  .finally(() => client.end());
