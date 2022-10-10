#!/usr/bin/env python
# -*- coding: utf-8 -*-
#python3 -m pip install -U wheel
#python3 -m pip install paho-mqtt
import random
import socket
import paho.mqtt.client as mqtt


# MQTT Broker
host = 'esp32-broker.local'
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
	_host = socket.gethostbyname(host)
	print("_host={}".format(_host))
	client_id = f'python-mqtt-{random.randint(0, 1000)}'
	client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311)
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(_host, port=port, keepalive=60)
	client.loop_forever()

