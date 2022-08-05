// Software requirement
// npm install mqtt

// MQTT Broker
host = 'mqtt://your_broker'
//host = 'mqtt://192.168.10.40'
// MQTT Subscribe Topic
topic = '/image/#';
// Save File
saveFile = './output.jpg'

// Connect to MQTT server
const client = require('mqtt').connect(host);

// Message reception handler
client.on('message', (topic, message) => {
  console.log("receive topic is %s", topic);
  const fs = require("fs");
  fs.writeFile(saveFile, message, (err) => {
    if (err) throw err;
    console.log('file %s (%d bytes) write successfully', saveFile, message.length);
  });
});

// Server connection handler
client.on('connect', () => {
  console.log('Connected to MQTT server');
  client.subscribe(topic);

});

