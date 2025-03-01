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
import queue
import threading

queue01 = queue.Queue()
queue02 = queue.Queue()

def threadView(q1, q2):
	time.sleep(1)
	localVal = threading.local()
	while True:
		if q1.empty():
			time.sleep(1.0)
			#print("thread waiting..")
		else:
			localVal = q1.get()
			print("thread q1.get() localVal={} timeout={}".format(localVal, args.timeout))
			image = cv2.imread(args.output)
			cv2.imshow('image', image)
			cv2.waitKey(args.timeout*1000)
			#cv2.waitKey(0)
			cv2.destroyWindow('image')
			print("thread q2.put() localVal={}".format(localVal))
			q2.put(localVal)


def on_connect(client, userdata, flags, respons_code):
	print('connect {0} status {1}'.format(args.host, respons_code))
	client.subscribe(args.topic)

def on_message(client, userdata, msg):
	print('receive topic is {}'.format(msg.topic))
	#outfile = open('./output.jpg', 'wb')
	outfile = open(args.output, 'wb')
	outfile.write(msg.payload)
	outfile.close
	print('file {} ({} bytes) write successfully'.format(args.output, len(msg.payload)))

	queue01.put(0)
	while True:
		time.sleep(1)
		if queue02.empty():
			print("thread end waiting. ESC to end.")
			pass
		else:
			queue02.get()
			break
	print("thread end")

if __name__=='__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('--host', help='mqtt broker', default='esp32-broker.local')
	parser.add_argument('--port', type=int, help='mqtt port', default=1883)
	parser.add_argument('--topic', help='mqtt topic', default='/image/esp32cam')
	parser.add_argument('--output', help='output file name', default='./output.jpg')
	parser.add_argument('--timeout', type=int, help='wait time for keyboard input[sec]', default=0)
	args = parser.parse_args() 
	print("args.host={}".format(args.host))
	print("args.port={}".format(args.port))
	print("args.topic={}".format(args.topic))
	print("args.output={}".format(args.output))
	print("args.timeout={}".format(args.timeout))

	thread = threading.Thread(target=threadView, args=(queue01,queue02,) ,daemon = True)
	thread.start()

	_host = socket.gethostbyname(args.host)
	#print("_host={}".format(_host))
	client_id = f'python-mqtt-{random.randint(0, 1000)}'
	print("client_id={}".format(client_id))
	client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311)
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(_host, port=args.port, keepalive=60)
	client.loop_forever()
