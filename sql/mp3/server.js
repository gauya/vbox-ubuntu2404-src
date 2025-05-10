// server.js
const express = require('express');
const multer = require('multer');
const path = require('path');
const fs = require('fs');
//const mm = require('music-metadata');
const { Pool } = require('pg');

const app = express();
const port = 3000;

require('dotenv').config();
// PostgreSQL 연결 설정
const pool = new Pool({
  host: process.env.DB_HOST,
  port: process.env.DB_PORT,
  database: process.env.DB_NAME,
  user: process.env.DB_USER,
  password: process.env.DB_PASSWORD,
});

// 업로드 디렉토리
const uploadDir = path.join(__dirname, 'uploads');
if (!fs.existsSync(uploadDir)) fs.mkdirSync(uploadDir);

const storage = multer.diskStorage({
  destination: (req, file, cb) => cb(null, uploadDir),
  filename: (req, file, cb) => cb(null, Date.now() + path.extname(file.originalname)),
});

const upload = multer({ storage });

app.use(express.json());

// 노래 업로드 및 메타데이터 저장
app.post('/upload', upload.single('mp3'), async (req, res) => {
  try {
    const filePath = req.file.path;
    const metadata = await mm.parseFile(filePath);

    const {
      title = '',
      artist = '',
      album = '',
      genre = [],
      duration = 0,
      track = {},
      year = null,
    } = metadata.common;

    const sampleRate = metadata.format.sampleRate;
    const fileSize = req.file.size;

    await pool.query(
      `INSERT INTO mp3_library (
        title, artist, album, genre1, sample_rate,
        duration, file_size, file_path, track, release_date
      ) VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10)`,
      [
        title,
        artist,
        album,
        genre[0] || null,
        sampleRate,
        duration,
        fileSize,
        filePath,
        track.no || null,
        year,
      ]
    );

    res.status(200).json({ message: '업로드 완료' });
  } catch (err) {
    console.error(err);
    res.status(500).json({ error: '업로드 실패' });
  }
});

// mp3 메타데이터 목록 조회
app.get('/songs', async (req, res) => {
  const result = await pool.query('SELECT id, title, artist, album FROM mp3_library ORDER BY id DESC');
  res.json(result.rows);
});

// mp3 파일 재생 (다운로드 or 스트리밍)
//app.get('/songs/:id/play', async (req, res) => {
app.get('/songs/:id', async (req, res) => {
  const result = await pool.query('SELECT file_path FROM mp3_library WHERE id = $1', [req.params.id]);
  if (result.rowCount === 0) return res.status(404).send('Not found');
  const filepath = result.rows[0].file_path;
  res.sendFile(path.resolve(filepath));
});

app.listen(port, () => {
  console.log(`서버 실행 중 http://localhost:${port}`);
});

