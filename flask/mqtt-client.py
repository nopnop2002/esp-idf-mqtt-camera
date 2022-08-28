# -*- coding: utf-8 -*-
import os
import sys
import datetime
import magic
import argparse
import logging
from flask import Flask, render_template, request, jsonify, Blueprint, redirect, url_for, send_file
# https://flask-mqtt.readthedocs.io/en/latest/
# https://www.emqx.com/en/blog/how-to-use-mqtt-in-flask
from flask_mqtt import Mqtt
from werkzeug.serving import WSGIRequestHandler

logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
parser = argparse.ArgumentParser()
parser.add_argument('--broker', default="localhost", help='mqtt broker. default=localhost')
parser.add_argument('--port', default=1883, help='mqtt port. default=1883')
parser.add_argument('--subtopic', default="/image/esp32cam", help='mqtt subscribe topic. default=/image/esp32cam')
parser.add_argument('--pubtopic', default="/topic/picture/sub", help='mqtt publish topic. default=/topic/picture/sub')
args = parser.parse_args()
logging.info("broker={} {}".format(type(args.broker), args.broker))
logging.info("port={} {}".format(type(args.port), args.port))
logging.info("subtopic={} {}".format(type(args.subtopic), args.subtopic))
logging.info("pubtopic={} {}".format(type(args.pubtopic), args.pubtopic))

app = Flask(__name__)

#app.config['MQTT_BROKER_URL'] = '192.168.10.40'
app.config['MQTT_BROKER_URL'] = args.broker
#app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_BROKER_PORT'] = args.port
app.config['MQTT_USERNAME'] = ''  # Set this item when you need to verify username and password
app.config['MQTT_PASSWORD'] = ''  # Set this item when you need to verify username and password
app.config['MQTT_KEEPALIVE'] = 5  # Set KeepAlive time in seconds
app.config['MQTT_TLS_ENABLED'] = False	# If your broker supports TLS, set it True
#topic = '/image/esp32cam'

mqtt_client = Mqtt(app)

# create UPLOAD_DIR
UPLOAD_DIR = os.path.join(os.getcwd(), "uploaded")
logging.info("UPLOAD_DIR={}".format(UPLOAD_DIR))
if (os.path.exists(UPLOAD_DIR) == False):
	logging.warning("UPLOAD_DIR [{}] not found. Create this".format(UPLOAD_DIR))
	os.mkdir(UPLOAD_DIR)

# Added /uploaded to static_url_path
add_app = Blueprint("uploaded", __name__, static_url_path='/uploaded', static_folder='./uploaded')
app.register_blueprint(add_app)

@app.route("/")
def root():
	function = sys._getframe().f_code.co_name
	files = []
	dirs = []
	meta = {
		"current_directory": UPLOAD_DIR
	}
	for (dirpath, dirnames, filenames) in os.walk(UPLOAD_DIR):
		logging.debug("{}:filenames={}".format(function, filenames))
		for name in filenames:
			nm = os.path.join(dirpath, name).replace(UPLOAD_DIR, "").strip("/").split("/")
			logging.debug("nm={}".format(nm))
			# Skip if the file is in a subdirect
			# nm=['templates', 'index.html']
			if len(nm) != 1: continue

			fullpath = os.path.join(dirpath, name)
			logging.debug("fullpath={}".format(fullpath))
			if os.path.isfile(fullpath) == False: continue
			size = os.stat(fullpath).st_size

			mime = magic.from_file(fullpath, mime=True)
			logging.debug("mime={}".format(mime))
			visible = "image/" in mime
			if (visible == False):
				visible = "text/" in mime
			logging.debug("visible={}".format(visible))

			files.append({
				"name": name,
				"size": str(size) + " B",
				"mime": mime,
				"fullname": fullpath,
				"visible": visible
			})

	return render_template("index.html", files=sorted(files, key=lambda k: k["name"].lower()), folders=dirs, meta=meta)

@app.route("/download")
def download():
	function = sys._getframe().f_code.co_name
	filename = request.args.get('filename', default=None, type=str)
	logging.info("{}:filename={}".format(function, filename))

	if os.path.isfile(filename):
		if os.path.dirname(filename) == UPLOAD_DIR.rstrip("/"):
			return send_file(filename, as_attachment=True)
		else:
			return render_template("no_permission.html")
	else:
		return render_template("not_found.html")
	return None

@app.route("/imageview")
def imageview():
	function = sys._getframe().f_code.co_name
	filename = request.args.get('filename', default=None, type=str)
	logging.info("{}:filename={}".format(function, filename))
	rotate = request.args.get('rotate', default=0, type=int)
	logging.info("{}:rotate={}{}".format(function, rotate, type(rotate)))

	mime = magic.from_file(filename, mime=True)
	mime = mime.split("/")
	logging.info("{} mime={}".format(function, mime))

	if (mime[0] == "image"):
		logging.info("{}:filename={}".format(function, filename))
		filename = filename.replace(os.getcwd(), "")
		logging.info("{}:filename={}".format(function, filename))
		return render_template("view.html", user_image = filename, rotate=rotate)

	if (mime[0] == "text"):
		contents = ""
		f = open(filename, 'r')
		datalist = f.readlines()
		for data in datalist:
			logging.info("[{}]".format(data.rstrip()))
			contents = contents + data.rstrip() + "<br>"
		return contents

@app.route("/delete")
def delete():
	function = sys._getframe().f_code.co_name
	filename = request.args.get('filename', default=None, type=str)
	logging.info("{}:filename={}".format(function, filename))
	os.remove(filename)
	return redirect(url_for('root'))

@app.route("/publish")
def publish():
	function = sys._getframe().f_code.co_name
	logging.info("{}".format(function))
	mqtt_client.publish(args.pubtopic, 'hello world')
	return redirect(url_for('root'))

@mqtt_client.on_connect()
def handle_connect(client, userdata, flags, rc):
	function = sys._getframe().f_code.co_name
	if rc == 0:
		logging.info('{}:Connected successfully'.format(function))
		mqtt_client.subscribe(args.subtopic) # subscribe topic
	else:
		logging.info('{}:Bad connection. Code:{}'.format(function, rc))


@mqtt_client.on_message()
def handle_mqtt_message(client, userdata, message):
	function = sys._getframe().f_code.co_name
	topic=message.topic 
	payload=message.payload

	logging.info("{}:Received message on topic={}".format(function, topic))
	logging.info("{}:UPLOAD_DIR={}".format(function, UPLOAD_DIR))
	dt_now = datetime.datetime.now()
	saveFile = "{}/{}.jpg".format(UPLOAD_DIR, dt_now.strftime('%Y%m%d-%H%M%S'))
	logging.info("{}:saveFile={}".format(function, saveFile))
	#saveFile = './output.jpg'
	outfile = open(saveFile, 'wb')
	outfile.write(payload)
	outfile.close
	logging.info("{}:file {} ({} bytes) write successfully".format(function, saveFile, len(payload)))

if __name__ == '__main__':
	WSGIRequestHandler.protocol_version = "HTTP/1.1"
	#app.run(host='127.0.0.1', port=5000)
	app.run(host='0.0.0.0', port=8080, debug=True)
