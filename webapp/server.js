var app = require('express')();
var http = require('http').Server(app);
var firebase = require('firebase');

var serverPort = 3000; // default to 3000

// landing page
app.get('/', function(req, res) {
  res.sendFile(__dirname + '/index.html');
});

// other files
app.get('/*', function(req, res) {
  res.sendFile(__dirname + req.url);
});

if(process.argv[2]) {
   serverPort = process.argv[2];
}

http.listen(serverPort, function() {
  console.log('Listening on Port: ' + serverPort);
});






