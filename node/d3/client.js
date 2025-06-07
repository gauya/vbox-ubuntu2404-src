var net = require('net');
var client = net.connect({host: "localhost", port: 3001}, function() {
   console.log('connected to server!');  
	client.write('{"state" : "end", "mac":"00:19:94:47:fa:8f"}');
});

client.on('data', function(data) {
   console.log('from server :' + data.toString());
	console.log();
});

client.on('end', function() { 
   console.log('disconnected from server');
   client.end();
});
