var net = require('net');
var port = 6807;

// create a socket
var client = new net.Socket();
// connect to server
client.connect(port, '127.0.0.1', function() {
    console.log("Wow, it connected");
    // write to server
    client.write("Can I get a ye yeee");
});

// read from server
client.on('data', function(data) {
    console.log(`Recieved: ${data}`);
    // what a violent method name
    client.destroy();
});

// print output
client.on('close', function() {
    console.log("Goodbye.");
});
