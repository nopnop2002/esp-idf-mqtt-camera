#!/usr/bin/env python
# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt

# MQTT Broker
#host = '192.168.10.40'
host = 'test.mosquitto.org'
# MQTT Port
port = 1883
# MQTT Publish Topic
topic = '/topic/picture/sub'

if __name__=='__main__':
	client = mqtt.Client(protocol=mqtt.MQTTv311)
	client.connect(host, port=port, keepalive=60)
	client.publish(topic, 'take picture')
	client.disconnect()
