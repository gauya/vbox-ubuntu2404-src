const { Client } = require('pg');
const express = require('express');
const path = require('path');
const app = express();
require('dotenv').config();

// PostgreSQL 연결 설정
const client = new Client({
  host: process.env.DB_HOST,
  port: process.env.DB_PORT,
  database: process.env.DB_NAME,
  user: process.env.DB_USER,
  password: process.env.DB_PASSWORD,
});

// 데이터베이스 연결
client.connect();

// 기본 경로
app.get('/ctrl', (req, res) => {
  //res.send('Welcome to the Music Player API. Use /player/:id to play a song.');
  res.sendFile(path.join(__dirname, 'public', 'play_ctrl.html'));
});

// 음악 파일 제공 API
app.get('/song/:id', async (req, res) => {
  try {
    const songId = req.params.id;
    
    // 데이터베이스에서 bytea 데이터 조회
    const result = await client.query(
      'SELECT audio_data, dtype FROM public.songs WHERE id = $1', 
      [songId]
    );
    
    if (result.rows.length === 0) {
      return res.status(404).send('Song not found');
    }
    
    const { audio_data, dtype } = result.rows[0];
    
    // 적절한 Content-Type 설정
    const contentType = `audio/${dtype}`; // 예: 'audio/mp3', 'audio/wav' 등
    res.setHeader('Content-Type', contentType);
    
    // bytea 데이터 전송
    res.send(audio_data);
    
  } catch (err) {
    console.error('Error:', err);
    res.status(500).send('Server error');
  }
});

// 음악 파일 목록 API
app.get('/', async (req, res) => {
  try {
    const result = await client.query('SELECT id, name FROM public.songs');
    const songsList = result.rows.map(row =>
      `<li><a href="/player/${row.id}">${row.name}</a></li>`
    ).join('');

    res.send(`
    <!DOCTYPE html>
    <html>
    <head>
      <title>Music Player</title>
    </head>
    <body>
      <h1>Song List</h1>
      <ul>${songsList}</ul>
    </body>
    </html>
    `);
  } catch (err) {
    res.status(500).send('Database error');
  }
});

// HTML 페이지 (재생기 포함)
app.get('/player/:id', (req, res) => {
  const songId = req.params.id;
  res.send(`
    <!DOCTYPE html>
    <html>
    <head>
      <title>Music Player</title>
    </head>
    <body>
      <h1>Music Player</h1>
      <audio controls>
        <source src="/song/${songId}" type="audio/mp3">
        Your browser does not support the audio element.
      </audio>
    </body>
    </html>
  `);
});

// 서버 시작
const PORT = 3000;
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
