var client = require('net').connect({host: "localhost", port: 3001}, function() { 
	client.write('{"state" : "end", "mac":"00:19:94:47:fa:8f"}');
	client.end();
});
