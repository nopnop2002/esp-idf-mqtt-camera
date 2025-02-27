#!/usr/bin/env python
# -*- coding: utf-8 -*-
# python3 -m pip install -U wheel
# python3 -m pip install 'paho-mqtt>=1.0.0,<2.0.0'
# python3 -m pip install opencv-python
import random
import socket
import paho.mqtt.client as mqtt
import cv2
import time
import argparse
import signal

running = True

def signal_handler(signum, frame):
	# print("SIGINT")
	global running
	running = False

def on_connect(client, userdata, flags, respons_code):
	print('connect {0} status {1}'.format(args.host, respons_code))
	client.subscribe(args.topic)

def on_message(client, userdata, msg):
	#outfile = open('./output.jpg', 'wb')
	outfile = open(args.output, 'wb')
	outfile.write(msg.payload)
	outfile.close
	print('receive topic is {}'.format(msg.topic))
	print('file {} ({} bytes) write successfully'.format(args.output, len(msg.payload)))
	print('esc is close image windows')

	global image
	image = cv2.imread(args.output)

if __name__=='__main__':
	signal.signal(signal.SIGINT, signal_handler)
	parser = argparse.ArgumentParser()
	parser.add_argument('--host', help='mqtt broker', default='esp32-broker.local')
	parser.add_argument('--port', type=int, help='mqtt port', default=1883)
	parser.add_argument('--topic', help='mqtt topic', default='/image/esp32cam')
	parser.add_argument('--output', help='output file name', default='./output.jpg')
	args = parser.parse_args() 
	print("args.host={}".format(args.host))
	print("args.port={}".format(args.port))
	print("args.topic={}".format(args.topic))
	print("args.output={}".format(args.output))

	_host = socket.gethostbyname(args.host)
	# print("_host={}".format(_host))
	client_id = f'python-mqtt-{random.randint(0, 1000)}'
	print("client_id={}".format(client_id))
	client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311)
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(_host, port=args.port, keepalive=60)
	#client.loop_forever()
	client.loop_start()
	
	image = None
	while running:
		# print("image={}".format(image))
		if (image is not None):
			cv2.imshow('image', image)
			cv2.waitKey(0)
			cv2.destroyWindow('image')
			image = None
			
		time.sleep(1)

