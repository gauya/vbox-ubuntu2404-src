#!/usr/bin/env node 
/**
 * Module dependencies.
 */

var app = require('../app');
var debug = require('debug')('momed:server');
var http = require('http');
// Import the Server class from socket.io
const { Server } = require('socket.io'); 
var net = require('net'); // net 모듈 추가

// var node_server_ip = 'gau.iptime.org'; // 이 변수는 현재 사용되지 않습니다.

/**
 * Get port from environment and store in Express.
 */
var port = normalizePort(process.env.PORT || '3000');
var ctlport = normalizePort(process.env.CTLPORT || '3001');

app.set('port', port);

/**
 * Normalize a port into a number, string, or false.
 */
function normalizePort(val) {
  var port = parseInt(val, 10);

  if (isNaN(port)) {
    // named pipe
    return val;
  }

  if (port >= 0) {
    // port number
    return port;
  }

  return false;
}

// 1. HTTP Server Creation and Listening (unchanged from your original)
// This 'srv' is your core HTTP server
var srv = http.createServer(app); 
srv.listen(port); // Listen on the HTTP port

// 2. Socket.IO Server Initialization (***MAJOR CHANGE HERE***)
// Pass the HTTP server instance (srv) to the Socket.IO Server constructor
const io = new Server(srv); // This is the correct way for modern Socket.IO

// 3. Event listeners for the HTTP server (srv)
// These event listeners should be on 'srv' (the http.Server instance), not on 'io' (the Socket.IO server).
srv.on('error', function (error) { // Moved from 'server' to 'srv'
  if (error.syscall !== 'listen') {
    throw error;
  }

  var bind = typeof port === 'string' // Use 'port' here, not 'httpport'
    ? 'Pipe ' + port
    : 'Port ' + port;

  // handle specific listen errors with friendly messages
  switch (error.code) {
    case 'EACCES':
      console.error(bind + ' requires elevated privileges');
      process.exit(1);
      break;
    case 'EADDRINUSE':
      console.error(bind + ' is already in use');
      process.exit(1);
      break;
    default:
      throw error;
  }
});

srv.on('listening', function() { // Moved from 'server' to 'srv'
  console.log(__filename, 'onListening');
  var addr = srv.address(); // Use srv.address()
  var bind = typeof addr === 'string'
    ? 'pipe ' + addr
    : 'port ' + addr.port;
  debug('Listening on ' + bind);
  console.log(addr.address + ' Listening on ' + addr.port); // Changed addr.addr to addr.address for consistency
});


// 4. Socket.IO Connection Event Listener (***CHANGED FROM server.on('connection')***)
// This is for clients connecting to Socket.IO
io.on('connection', function(socket) {
	console.log(__filename, 'Socket.IO client connected'); // Changed log message
    // You can now handle events specific to this client 'socket'
    
    // Example: listen for 'chat message' from clients
    socket.on('chat message', (msg) => {
        console.log('message from client: ' + msg);
        io.emit('chat message', msg); // Broadcast to all connected Socket.IO clients
    });

    socket.on('disconnect', () => {
        console.log(__filename, 'Socket.IO client disconnected');
    });

    // Remove these, as Socket.IO server doesn't emit these raw TCP events
    // server.on('data', function(a) { ... });
    // server.on('close', function() { ... });
});


//////////////////////////////////////////////////////////////////////////
// Net Server (TCP/IP socket server)
// This part is for raw TCP connections, separate from HTTP and Socket.IO
var netServer = net.createServer(function(socket){
    // Event listeners for individual client sockets in the netServer
	socket.on('data', function(data) {
		var date = new Date();
		var now = date.toUTCString();
		console.log('net recved ['+data.toString()+']'+' lendtg='+data.length); // Convert Buffer to string

		var cliData;
		try{
			cliData = JSON.parse(data.toString()); // Parse the string data
		}catch(e){ 
            console.log('JSON parsing error:', e.toString());			
		    return;
		}
		
		for(let key in cliData) // Use let for loop variable
		{
			if(cliData[key] == "news") {
			  io.emit('news', {time : now, mac : cliData.mac}); // Use 'io' to emit to Socket.IO clients
			}
			if(key == "test") {
			  io.emit('test', cliData.mac); // Use 'io' to emit to Socket.IO clients
			}
		}
	});
	socket.on('lookup', function() {
	 	console.log('net client lookup');
	});
	socket.on('connect', function() {
	 	console.log('net client connect');
	});
	socket.on('end', function() {
	 	console.log('net client disconnect');
	});
	socket.on('timeout', function() {
	 	console.log('net client timeout');
	});
	socket.on('drain', function() {
	 	console.log('net client drain');
	});
	socket.on('close', function() {
	 	console.log('net client close');
	});
	socket.on('error', function(err) { // Add 'err' parameter to log the error
	 	console.log('net client error:', err.message);
	});
});

// Event listeners for the netServer itself
netServer.on('listening', function (){
  console.log('netServer is listening on port', ctlport);
});

netServer.on('connection', function(socket) {
  console.log('netServer has a new connection : ' + socket.remoteAddress);
  socket.write('What. i am very busy!! you are ['+socket.remoteAddress+']\n\r');
});

// Corrected: close event for netServer itself, not a specific client socket
netServer.on('close', function() {
  console.log('netServer connection closed');
  // Removed socket.end() as 'socket' is not defined in this scope
});

netServer.on('error', function(err) {
  console.log('netServer Error occured:', err.message);
});

netServer.listen(ctlport);
