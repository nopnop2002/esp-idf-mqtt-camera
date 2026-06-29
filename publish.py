#!/usr/bin/env python
# -*- coding: utf-8 -*-
import argparse
import random
import socket
import paho.mqtt.client as mqtt

if __name__=='__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('--host', help='mqtt broker', default='esp32-broker.local')
	parser.add_argument('--port', type=int, help='mqtt port', default=1883)
	parser.add_argument('--topic', help='mqtt topic', default='/take/picture')
	args = parser.parse_args() 
	print("args.host={}".format(args.host))
	print("args.port={}".format(args.port))
	print("args.topic={}".format(args.topic))

	_host = socket.gethostbyname(args.host)
	print("_host={}".format(_host))
	client_id = f'python-mqtt-{random.randint(0, 1000)}'
	client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id)
	client.connect(_host, port=args.port, keepalive=60)
	client.publish(args.topic, b'take picture')
	client.disconnect()
