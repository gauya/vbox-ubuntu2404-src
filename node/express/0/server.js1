const express = require('express');
const WebSocket = require('ws');
const path = require('path');
const fs = require('fs');
const { v4: uuidv4 } = require('uuid');

const app = express();
const PORT = 3000;

// 파일 업로드 디렉토리 설정
const uploadDir = path.join(__dirname, 'uploads');
if (!fs.existsSync(uploadDir)) {
  fs.mkdirSync(uploadDir, { recursive: true });
}

app.use(express.static(path.join(__dirname, 'public')));
app.use('/uploads', express.static(uploadDir));

const server = app.listen(PORT, () => {
  console.log(`서버 실행 중: http://localhost:${PORT}`);
});

const wss = new WebSocket.Server({ server });

// 초기 채팅방 설정
const chatRooms = {
  '일반': new Set(),
  '게임': new Set(),
  '음악': new Set()
};

// 사용자 정보 저장
const users = new Map();

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
        rooms: roomList,
        onlineCount: users.size
      }));
    }
  });
}

// 특정 방에 메시지 브로드캐스트
function broadcastRoomMessage(room, message) {
  if (!chatRooms[room]) return;
  
  chatRooms[room].forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify(message));
    }
  });
}

// WebSocket 연결 처리
wss.on('connection', (ws) => {
  console.log('새 클라이언트 연결됨');

  // 연결 시 바로 방 목록 전송
  broadcastRoomList();

  // 클라이언트로부터 메시지 수신
  ws.on('message', (message) => {
    try {
      // 바이너리 데이터 처리 (파일 전송)
      if (typeof message !== 'string') {
        const user = users.get(ws);
        if (!user || !user.room) return;

        const metaDataStr = message.slice(0, 50).toString().replace(/\0+$/, '');
        const metaData = JSON.parse(metaDataStr);
        
        const fileName = uuidv4() + '.' + metaData.ext;
        const filePath = path.join(uploadDir, fileName);
        
        fs.writeFile(filePath, message.slice(50), (err) => {
          if (err) return console.error('파일 저장 오류:', err);
          
          broadcastRoomMessage(user.room, {
            type: 'file',
            username: user.username,
            fileName: metaData.originalName,
            fileUrl: `/uploads/${fileName}`,
            time: new Date().toLocaleTimeString(),
            fileType: metaData.type
          });
        });
        return;
      }

      // JSON 메시지 처리
      const data = JSON.parse(message);
      console.log('받은 메시지:', data);

      // 사용자 등록
      if (data.type === 'register') {
        if (!data.username || data.username.trim() === '') {
          ws.send(JSON.stringify({
            type: 'error',
            message: '사용자 이름을 입력해주세요.'
          }));
          return;
        }

        users.set(ws, {
          username: data.username,
          room: null
        });
        console.log(`사용자 등록: ${data.username}`);
        
        ws.send(JSON.stringify({
          type: 'registerSuccess',
          username: data.username
        }));
        
        broadcastRoomList();
        return;
      }

      // 방 생성
      if (data.type === 'createRoom') {
        const user = users.get(ws);
        if (!user) {
          ws.send(JSON.stringify({
            type: 'error',
            message: '먼저 사용자 이름을 등록해주세요.'
          }));
          return;
        }

        if (!data.roomName || data.roomName.trim() === '') {
          ws.send(JSON.stringify({
            type: 'error',
            message: '방 이름을 입력해주세요.'
          }));
          return;
        }

        if (!chatRooms[data.roomName]) {
          chatRooms[data.roomName] = new Set();
          console.log(`새 방 생성: ${data.roomName}`);
          
          ws.send(JSON.stringify({
            type: 'roomCreated',
            roomName: data.roomName
          }));
          
          broadcastRoomList();
        }
        return;
      }

      // 방 입장
      if (data.type === 'join') {
        const user = users.get(ws);
        if (!user) {
          ws.send(JSON.stringify({
            type: 'error',
            message: '먼저 사용자 이름을 등록해주세요.'
          }));
          return;
        }

        // 기존 방에서 나가기
        if (user.room) {
          chatRooms[user.room].delete(ws);
          broadcastRoomMessage(user.room, {
            type: 'userLeft',
            username: user.username
          });
        }

        // 새 방에 입장 (없으면 생성)
        if (!chatRooms[data.room]) {
          chatRooms[data.room] = new Set();
        }

        user.room = data.room;
        chatRooms[data.room].add(ws);

        console.log(`${user.username}님이 ${data.room} 방에 입장`);
        
        // 입장 성공 응답
        ws.send(JSON.stringify({
          type: 'joinSuccess',
          room: data.room,
          username: user.username
        }));

        // 방 사용자 목록 전송
        const usersInRoom = Array.from(chatRooms[data.room]).map(ws => users.get(ws).username);
        broadcastRoomMessage(data.room, {
          type: 'userList',
          users: usersInRoom
        });

        broadcastRoomList();
        return;
      }

      // 일반 메시지
      if (data.type === 'message') {
        const user = users.get(ws);
        if (!user || !user.room) return;

        broadcastRoomMessage(user.room, {
          type: 'message',
          username: user.username,
          text: data.text,
          time: new Date().toLocaleTimeString()
        });
      }

    } catch (error) {
      console.error('메시지 처리 오류:', error);
      ws.send(JSON.stringify({
        type: 'error',
        message: '메시지 처리 중 오류가 발생했습니다.'
      }));
    }
  });

  // 연결 종료 처리
  ws.on('close', () => {
    const user = users.get(ws);
    if (user) {
      if (user.room) {
        chatRooms[user.room].delete(ws);
        broadcastRoomMessage(user.room, {
          type: 'userLeft',
          username: user.username
        });
      }
      users.delete(ws);
      broadcastRoomList();
    }
    console.log('클라이언트 연결 종료');
  });

  ws.on('error', (error) => {
    console.error('웹소켓 오류:', error);
  });
});
