#!/usr/bin/env python
# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt

# MQTT Broker
host = 'broker.emqx.io'
# MQTT Port
port = 1883
# MQTT Subscribe Topic
topic = 'image/#'
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
	print('file {} create'.format(saveFile))

if __name__=='__main__':
	client = mqtt.Client(protocol=mqtt.MQTTv311)
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(host, port=port, keepalive=60)
	client.loop_forever()

