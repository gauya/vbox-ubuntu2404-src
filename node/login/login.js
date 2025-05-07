// npm install dotenv

require('dotenv').config(); // .env 파일 로드
const { Client } = require('pg');

const client = new Client({
  host: process.env.DB_HOST,
  port: process.env.DB_PORT,
  database: process.env.DB_NAME,
  user: process.env.DB_USER,
  password: process.env.DB_PASSWORD,
});

client.connect()
  .then(() => console.log('DB 연결 성공!'))
  .catch(err => console.error('연결 실패:', err))
  .finally(() => client.end());

/*
async function runQuery() {
  try {
    await client.connect();
    console.log('DB 연결 성공!');

    const res = await client.query('SELECT * FROM users WHERE id = $1', [1]);
    console.log(res.rows);
  } catch (err) {
    console.error('오류 발생:', err);
  } finally {
    await client.end();
    console.log('연결이 종료되었습니다.');
  }
}

runQuery();
*/

