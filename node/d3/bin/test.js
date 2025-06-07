// 1. Import necessary modules
const app = require('express')(); // Or whatever HTTP server framework you're using
const http = require('http'); // Node.js built-in HTTP module
const { Server } = require('socket.io'); // Import the Server class from socket.io

// 2. Create an HTTP server instance
const srv = http.createServer(app); // 'srv' is your HTTP server instance

// 3. Initialize Socket.IO by passing the HTTP server instance
const io = new Server(srv); // This is the correct way

// Now 'io' is your Socket.IO server instance.
// You can listen for connections:
io.on('connection', (socket) => {
    console.log('A user connected');

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });

    // Handle custom events
    socket.on('chat message', (msg) => {
        console.log('message: ' + msg);
        io.emit('chat message', msg); // Broadcast the message to all connected clients
    });
});

// 4. Start the HTTP server
const PORT = process.env.PORT || 3000;
srv.listen(PORT, () => {
    console.log(`Server listening on *:${PORT}`);
});
