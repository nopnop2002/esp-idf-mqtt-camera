// MQTT Broker
host = 'mqtt://broker.emqx.io'
// MQTT Subscribe Topic
topic = 'image/#';
// Save File
saveFile = './output.jpg'

// Connect to MQTT server
const client = require('mqtt').connect(host);

// Message reception handler
client.on('message', (topic, message) => {
  console.log(topic);
  const fs = require("fs");
  fs.writeFile(saveFile, message, (err) => {
    if (err) throw err;
    console.log('Writing completed successfully');
  });
});

// Server connection handler
client.on('connect', () => {
  console.log('Connected to MQTT server');
  client.subscribe(topic);

});

