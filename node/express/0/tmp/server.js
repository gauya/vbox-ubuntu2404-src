const express = require('express');
const WebSocket = require('ws');
const path = require('path');

const app = express();
const PORT = 3000;

app.use(express.static(path.join(__dirname, 'public')));

const server = app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});

const wss = new WebSocket.Server({ server });

// 채팅방 저장 객체
const chatRooms = {
  '일반': new Set(),
  '게임': new Set(),
  '음악': new Set()
};

// 사용자 정보 저장 (Map<WebSocket, {username, room}>)
const users = new Map();

// WebSocket 연결 이벤트 처리
wss.on('connection', (ws) => {
  console.log('New client connected');

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
            type: 'userList',
            users: getUsersInRoom(user.room)
          });
          broadcastRoomMessage(user.room, {
            type: 'notification',
            message: `${user.username}님이 방을 나갔습니다.`
          });
        }

        // 새 방에 추가 (없으면 생성)
        if (!chatRooms[data.room]) {
          chatRooms[data.room] = new Set();
        }
        
        user.room = data.room;
        chatRooms[data.room].add(ws);
        
        // 방 사용자 목록 전송
        broadcastRoomMessage(data.room, {
          type: 'userList',
          users: getUsersInRoom(data.room)
        });
        
        broadcastRoomMessage(data.room, {
          type: 'notification',
          message: `${user.username}님이 방에 입장했습니다.`
        });
        
        sendRoomList(ws);
        return;
      }
      
      // 방 생성
      if (data.type === 'createRoom') {
        if (!chatRooms[data.roomName]) {
          chatRooms[data.roomName] = new Set();
          broadcastRoomList();
        }
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

  ws.on('close', () => {
    const user = users.get(ws);
    if (user) {
      if (user.room && chatRooms[user.room]) {
        chatRooms[user.room].delete(ws);
        broadcastRoomMessage(user.room, {
          type: 'userList',
          users: getUsersInRoom(user.room)
        });
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

// 방 안의 사용자 목록 가져오기
function getUsersInRoom(room) {
  if (!chatRooms[room]) return [];
  return Array.from(chatRooms[room])
    .map(ws => users.get(ws)?.username)
    .filter(Boolean);
}

// 특정 방에 메시지 브로드�스트
function broadcastRoomMessage(room, message) {
  if (!chatRooms[room]) return;
  
  chatRooms[room].forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify(message));
    }
  });
}

// 모든 클라이언트에 방 목록 전송
function broadcastRoomList() {
  const roomList = Object.keys(chatRooms).map(room => ({
    name: room,
    userCount: chatRooms[room].size
  }));
  
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify({
        type: 'roomList',
        rooms: roomList
      }));
    }
  });
}

// 특정 클라이언트에 방 목록 전송
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
