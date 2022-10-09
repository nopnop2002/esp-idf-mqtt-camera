#!/usr/bin/env python
# -*- coding: utf-8 -*-
#python3 -m pip install -U wheel
#python3 -m pip install paho-mqtt
import paho.mqtt.client as mqtt


# MQTT Broker
host = 'your_broker'
#host = 'esp32-broker.local'
# MQTT Port
port = 1883
# MQTT Subscribe Topic
topic = '/image/esp32cam'
# Save File
saveFile = './output.jpg'

def on_connect(client, userdata, flags, respons_code):
	print('connect {0} status {1}'.format(host, respons_code))
	client.subscribe(topic)

def on_message(client, userdata, msg):
	#outfile = open('./output.jpg', 'wb')
	outfile = open(saveFile, 'wb')
	outfile.write(msg.payload)
	outfile.close
	print('receive topic is {}'.format(msg.topic))
	print('file {} ({} bytes) write successfully'.format(saveFile, len(msg.payload)))

if __name__=='__main__':
	client = mqtt.Client(protocol=mqtt.MQTTv311)
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(host, port=port, keepalive=60)
	client.loop_forever()

