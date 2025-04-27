const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const os = require('os');
const { Pool } = require('pg');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// PostgreSQL 연결 설정
const pool = new Pool({
  user: 'gau',
  host: 'localhost',
  database: 'gau',
  password: 'qjemf',
  port: 5432,
});

// CPU 사용률 계산
let previousCPUMeasure = getCPUUsage();
const cpuHistory = [];
const memHistory = [];

// 경고 상태 변수
let alertActive = false;
const ALERT_THRESHOLD = 30; // 경고 임계값 (%)

// CPU 사용률 계산 함수
function getCPUUsage() {
  const cpus = os.cpus();
  let totalIdle = 0, totalTick = 0;
  
  cpus.forEach(cpu => {
    for (let type in cpu.times) {
      totalTick += cpu.times[type];
    }
    totalIdle += cpu.times.idle;
  });
  
  return {
    idle: totalIdle / cpus.length,
    total: totalTick / cpus.length
  };
}

// 메모리 사용률 계산
function getMemoryUsage() {
  const totalMem = os.totalmem();
  const freeMem = os.freemem();
  return ((totalMem - freeMem) / totalMem) * 100;
}

// 웹소켓 연결 처리
io.on('connection', (socket) => {
  console.log('Client connected');
  
  const interval = setInterval(async () => {
    // CPU 사용률 계산
    const currentCPUMeasure = getCPUUsage();
    const idleDifference = currentCPUMeasure.idle - previousCPUMeasure.idle;
    const totalDifference = currentCPUMeasure.total - previousCPUMeasure.total;
    const cpuUsage = 100 - (100 * idleDifference / totalDifference);
    
    previousCPUMeasure = currentCPUMeasure;
    
    // 메모리 사용률 계산
    const memUsage = getMemoryUsage();
    
    // 데이터베이스에 저장
    try {
      await pool.query(
        'INSERT INTO system_metrics (cpu_usage, mem_usage) VALUES ($1, $2)',
        [cpuUsage, memUsage]
      );
    } catch (err) {
      console.error('Database error:', err);
    }
    
    // 최근 60개 데이터만 유지
    cpuHistory.push(cpuUsage);
    memHistory.push(memUsage);
    if (cpuHistory.length > 60) {
      cpuHistory.shift();
      memHistory.shift();
    }
    
    // 경고 상태 확인
    const maxUsage = Math.max(cpuUsage, memUsage);
    alertActive = maxUsage > ALERT_THRESHOLD;
    
    socket.emit('systemUpdate', {
      cpu: cpuUsage.toFixed(2),
      memory: memUsage.toFixed(2),
      cpuHistory: [...cpuHistory],
      memHistory: [...memHistory],
      alert: alertActive,
      maxUsage: maxUsage.toFixed(2)
    });
    
  }, 1000);

  socket.on('disconnect', () => {
    console.log('Client disconnected');
    clearInterval(interval);
  });
});

app.use(express.static('public'));

server.listen(3000, () => {
  console.log('Server running on http://localhost:3000');
});
