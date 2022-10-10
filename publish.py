#!/usr/bin/env python
# -*- coding: utf-8 -*-
import random
import socket
import paho.mqtt.client as mqtt

# MQTT Broker
host = 'esp32-broker.local'
# MQTT Port
port = 1883
# MQTT Publish Topic
topic = '/topic/picture/sub'

if __name__=='__main__':
	_host = socket.gethostbyname(host)
	print("_host={}".format(_host))
	client_id = f'python-mqtt-{random.randint(0, 1000)}'
	client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311)
	client.connect(_host, port=port, keepalive=60)
	client.publish(topic, b'take picture')
	client.disconnect()
