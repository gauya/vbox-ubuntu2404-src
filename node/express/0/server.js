const express = require('express');
const WebSocket = require('ws');
const path = require('path');

const app = express();
const PORT = 3000;

// 정적 파일 서빙
app.use(express.static(path.join(__dirname, 'public')));

// Express 서버 시작
const server = app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});

// WebSocket 서버 생성
const wss = new WebSocket.Server({ server });

// 채팅방 저장 객체
const chatRooms = {
  '일반': new Set(),
  '게임': new Set(),
  '음악': new Set()
};

// 사용자 정보 저장
const users = new Map();

// WebSocket 연결 이벤트 처리
wss.on('connection', (ws) => {
  console.log('New client connected');

  // 클라이언트로부터 메시지 수신 이벤트 처리
  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message);
      
      // 사용자 등록
      if (data.type === 'register') {
        users.set(ws, {
          username: data.username,
          room: null
        });
        sendRoomList(ws);
        return;
      }
      
      // 방 입장
      if (data.type === 'join') {
        const user = users.get(ws);
        if (!user) return;

        // 기존 방에서 제거
        if (user.room && chatRooms[user.room]) {
          chatRooms[user.room].delete(ws);
          broadcastRoomMessage(user.room, {
            type: 'notification',
            message: `${user.username}님이 방을 나갔습니다.`
          });
        }

        // 새 방에 추가
        user.room = data.room;
        chatRooms[data.room].add(ws);
        
        broadcastRoomMessage(data.room, {
          type: 'notification',
          message: `${user.username}님이 방에 입장했습니다.`
        });
        
        sendRoomList(ws);
        return;
      }
      
      // 일반 메시지
      if (data.type === 'message') {
        const user = users.get(ws);
        if (!user || !user.room) return;
        
        const messageData = {
          type: 'message',
          username: user.username,
          text: data.text,
          time: new Date().toLocaleTimeString(),
          room: user.room
        };
        
        broadcastRoomMessage(user.room, messageData);
      }
      
    } catch (error) {
      console.error('Error parsing message:', error);
    }
  });

  // 연결 종료 이벤트 처리
  ws.on('close', () => {
    const user = users.get(ws);
    if (user) {
      if (user.room && chatRooms[user.room]) {
        chatRooms[user.room].delete(ws);
        broadcastRoomMessage(user.room, {
          type: 'notification',
          message: `${user.username}님이 방을 나갔습니다.`
        });
      }
      users.delete(ws);
    }
    console.log('Client disconnected');
  });
});

// 특정 방에 메시지 브로드캐스트
function broadcastRoomMessage(room, message) {
  if (!chatRooms[room]) return;
  
  chatRooms[room].forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify(message));
    }
  });
}

// 방 목록 전송
function sendRoomList(ws) {
  const roomList = Object.keys(chatRooms).map(room => ({
    name: room,
    userCount: chatRooms[room].size
  }));
  
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'roomList',
      rooms: roomList
    }));
  }
}
