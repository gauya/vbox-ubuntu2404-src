// 서버 측 (Node.js)
const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8080 });

wss.on('connection', (ws) => {
  ws.on('message', (message) => {
    // 클라이언트에서 보낸 제어 명령 브로드캐스트
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(message);
      }
    });
  });
});

// 클라이언트 측 (플레이어 페이지)
const ws = new WebSocket('ws://localhost:8080');

ws.onmessage = (event) => {
  const command = JSON.parse(event.data);
  const audio = document.getElementById('audio-player');
  
  switch(command.action) {
    case 'play':
      if (command.songId) audio.src = `/song/${command.songId}`;
      audio.play();
      break;
    // 다른 명령들 처리...
  }
};

// 다른 페이지에서 명령 보내기
function sendControlCommand(action, songId = null) {
  const ws = new WebSocket('ws://localhost:8080');
  ws.onopen = () => {
    ws.send(JSON.stringify({ action, songId }));
    ws.close();
  };
}
