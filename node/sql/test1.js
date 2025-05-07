const ex = require('express');
const app = ex();

// npm install dotenv
require('dotenv').config(); // .env 파일 로드

const { Pool } = require('pg');
const pool = new Pool({
  user: process.env.DB_USER,
  host: process.env.DB_HOST,
  database: process.env.DB_NAME,
  password: process.env.DB_PASSWORD,
  port: process.env.DB_PORT,
  max: 20, // 최대 연결 수
  idleTimeoutMillis: 30000 // 유휴 연결 타임아웃(ms)
});

async function createTable() {
  try {
    await pool.query(`
      CREATE TABLE IF NOT EXISTS users (
        id SERIAL PRIMARY KEY,
        name VARCHAR(100) NOT NULL,
        created_at TIMESTAMP DEFAULT NOW()
      )
    `);
    console.log('Table created successfully');
  } catch (err) {
    console.error('Error creating table:', err);
  }
}

async function insertUser(name) {
  try {
    const res = await pool.query(
      'INSERT INTO users (name) VALUES ($1) RETURNING *',
      [name]
    );
    console.log('Inserted user:', res.rows[0]);
    return res.rows[0];
  } catch (err) {
    console.error('Error inserting user:', err);
    throw err;
  }
}


async function getUsers() {
    const client = await pool.connect(); // 풀에서 연결 가져오기
  try {
    const res = await client.query('SELECT * FROM users');
    return res.rows;
  } catch (err) {
    console.log(`Database ${err}`);
    throw `Database error : ${err}`;
  } finally {
    client.release(); // 연결 반환
  }
}

/* node version 20+
async function getUsers() {
  using client = await pool.connect(); // [1]
  const res = await client.query('SELECT * FROM users');
  return res.rows;
}
*/

async function getUserById(id) {
  try {
    const res = await pool.query('SELECT * FROM users WHERE id = $1', [id]);
    if (res.rows.length === 0) {
      console.log('User not found');
      return null;
    }
    console.log('User found:', res.rows[0]);
    return res.rows[0];
  } catch (err) {
    console.error('Error fetching user:', err);
    throw err;
  }
}

async function updateUser(id, newName) {
  try {
    const res = await pool.query(
      `UPDATE users SET name = case when $1 = name then $1||'+' else $1 end WHERE id = $2 RETURNING *`,
      [newName, id]
    );
    if (res.rowCount === 0) {
      console.log('User not found for update');
      return null;
    }
    console.log('Updated user:', res.rows[0]);
    return res.rows[0];
  } catch (err) {
    console.error('Error updating user:', err);
    throw err;
  }
}

async function transferUserData(oldUserId, newUserName) {
  const client = await pool.connect();
  try {
    await client.query('BEGIN');

    // 기존 사용자 조회
    const oldUser = (await client.query(
      'SELECT * FROM users WHERE id = $1 FOR UPDATE',
      [oldUserId]
    )).rows[0];

    if (!oldUser) {
      throw new Error('User not found');
    }

    // 새 사용자 생성
    const newUser = (await client.query(
      'INSERT INTO users (name) VALUES ($1) RETURNING *',
      [newUserName]
    )).rows[0];

    // 기존 사용자 업데이트
    await client.query(
      'UPDATE users SET name = $1 WHERE id = $2',
      [`${oldUser.name}(m)`, oldUserId]
    );

    await client.query('COMMIT');
    console.log('Transfer completed:', { oldUser, newUser });
    return { oldUser, newUser };
  } catch (err) {
    await client.query('ROLLBACK');
    console.error('Transfer failed:', err);
    throw err;
  } finally {
    client.release();
  }
}


app.get('/init', async (req, res) => {
  try {
    const re = createTable();
    res.json(re);
  } catch (err) {
    res.status(500).send(`${err}`);
  }
});
app.get('/users', async (req, res) => {
  try {
    insertUser('John Doe');
    insertUser('babo');
    updateUser(1, 'Jane Smith');
    transferUserData(2, 'Hong GilDong');
  } catch (err) {
    res.status(500).send(`${err}`);
  }
});
app.get('/', async (req, res) => {
  try {
    const users = await getUsers();
    res.json(users);
  } catch (err) {
    res.status(500).send(`서버 오류 ${err}`);
  }
});
// 서버 오류 ReferenceError: client is not defined    

const PORT = 3000;
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});


