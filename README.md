# esp-idf-mqtt-camera
Take a picture and Publish it via MQTT.   
This project use [this](https://components.espressif.com/components/espressif/esp32-camera) Camera Driver.   

![slide0001](https://user-images.githubusercontent.com/6020549/123804185-f73f3a00-d927-11eb-9bf8-918595583c1a.jpg)
![slide0002](https://user-images.githubusercontent.com/6020549/123804188-f8706700-d927-11eb-83ed-65d3aaf2a335.jpg)

# Hardware requirements
ESP32 development board with OV2640 camera.   
If you use other camera, edit sdkconfig.default.   
From the left:   
- Aithinker ESP32-CAM   
- Freenove ESP32-WROVER CAM   
- UICPAL ESPS3 CAM   
- Freenove ESP32S3-WROVER CAM (Clone)   

![es32-camera](https://github.com/nopnop2002/esp-idf-websocket-camera/assets/6020549/38dbef9a-ed85-4df2-8d22-499b2b497278)

# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   

# Installation
For AiThinker ESP32-CAM, You have to use a USB-TTL converter.   

|ESP-32|USB-TTL|
|:-:|:-:|
|U0TXD|RXD|
|U0RXD|TXD|
|GPIO0|GND|
|5V|5V|
|GND|GND|


```
git clone https://github.com/nopnop2002/esp-idf-mqtt-camera
cd esp-idf-mqtt-camera
idf.py set-target {esp32/esp32s3}
idf.py menuconfig
idf.py flash monitor
```

# Start firmware
For AiThinker ESP32-CAM, Change GPIO0 to open and press the RESET button.

# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/99891822-7f35d500-2cb1-11eb-928c-be9a8191dec9.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/200430857-7ae56376-038e-40ea-a694-15f49948e27b.jpg)

## Wifi Setting

![config-wifi-1](https://user-images.githubusercontent.com/6020549/182987385-b885f6d2-f52a-4fc3-a338-c089547703a1.jpg)

You can connect using the mDNS hostname instead of the IP address.   
![config-wifi-2](https://user-images.githubusercontent.com/6020549/182987378-4074dc87-05b6-4102-baf9-c0428eb54321.jpg)

You can use static IP.   
![config-wifi-3](https://user-images.githubusercontent.com/6020549/182987383-74bc02d7-678f-4aa3-87b1-41d3e99f279e.jpg)


## MQTT Server Setting
MQTT broker is specified by one of the following.
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```

jpeg files are sent as multiple messages.   
When using a public broker like broker.emqx.io, some messages may be missing.   
You can download the MQTT broker from [here](https://github.com/nopnop2002/esp-idf-mqtt-broker).   

![config-mqtt](https://user-images.githubusercontent.com/6020549/194732213-8f7291e7-75b5-46c0-918d-c8002d19e1ff.jpg)


## Select Board
![config-board](https://github.com/nopnop2002/esp-idf-mqtt-camera/assets/6020549/632305c3-c992-4a49-b361-5fd18b79d629)


## Select Frame Size
Large frame sizes take longer to take a picture.   

![config-framesize-1](https://user-images.githubusercontent.com/6020549/122478692-d233f880-d004-11eb-8176-cd4d750d1ed9.jpg)
![config-framesize-2](https://user-images.githubusercontent.com/6020549/122478707-d9f39d00-d004-11eb-9e55-70f4db369b41.jpg)

## Select Shutter

You can choose one of the following shutter methods

- Shutter is the Enter key on the keyboard   
	For operation check.   
	When using the USB port provided by the USB Serial/JTAG Controller Console, you need to enable the following line in sdkconfig.
	```
	CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y
	```
	![config-shutter-1](https://user-images.githubusercontent.com/6020549/99891847-b73d1800-2cb1-11eb-84c0-2d6c6d85c010.jpg)

- Shutter is a GPIO toggle

	- Initial Sate is PULLDOWN   
		The shutter is prepared when it is turned from OFF to ON, and a picture is taken when it is turned from ON to OFF.   

	- Initial Sate is PULLUP   
	The shutter is prepared when it is turned from ON to OFF, and a picture is taken when it is turned from OFF to ON.   

	I confirmed that the following GPIO can be used.   

	|GPIO|PullDown|PullUp|
	|:-:|:-:|:-:|
	|GPIO12|OK|NG|
	|GPIO13|OK|OK|
	|GPIO14|OK|OK|
	|GPIO15|OK|OK|
	|GPIO16|NG|NG|

	![config-shutter-2](https://user-images.githubusercontent.com/6020549/99891859-c15f1680-2cb1-11eb-8204-2eced32c3d81.jpg)

- Shutter is TCP Socket   
	ESP32 acts as a TCP server and listens for requests from TCP clients.   
	You can use tcp_send.py as shutter.   
	`python3 ./tcp_send.py`
	![Image](https://github.com/user-attachments/assets/4c301018-2f8c-4644-be3f-417222fb1842)

- Shutter is UDP Socket   
	ESP32 acts as a UDP listener and listens for requests from UDP clients.   
	You can use this command as shutter.   
	`echo -n "take" | socat - UDP-DATAGRAM:255.255.255.255:49876,broadcast`   
	You can use udp_send.py as shutter.   
	Requires netifaces.   
	`python3 ./udp_send.py`   
	![Image](https://github.com/user-attachments/assets/3dcd72be-d0ef-4bd9-9273-f420ca88f11b)   
	You can use these devices as shutters.   
	![Image](https://github.com/user-attachments/assets/cc97da4e-6c06-4604-8362-f81c6fb6eb58)   
	Click [here](https://github.com/nopnop2002/esp-idf-selfie-trigger) for details.   

- Shutter is MQTT Publish   
	ESP32 acts as an MQTT subscriber and listens to requests from MQTT publishes.   
	You can use mosquitto_pub as shutter.   
	```
	mosquitto_pub -h your_broker -p 1883 -t "/take/picture" -m ""
	```
	![config-shutter-3](https://user-images.githubusercontent.com/6020549/200434412-adecbc77-7ba6-4520-9731-cc3eec301b84.jpg)


## Flash Light   
ESP32-CAM by AI-Thinker have flash light on GPIO4.   

![config-flash](https://user-images.githubusercontent.com/6020549/122479023-6b630f00-d005-11eb-98cc-b5fdbf2987c7.jpg)

## PSRAM   
When using ESP32S3, you need to set the PSRAM type according to the hardware.   
ESP32S3-WROVER CAM has Octal Mode PSRAM.   
UICPAL ESPS3 CAM  has Quad Mode PSRAM.   
![config-psram](https://github.com/nopnop2002/esp-idf-websocket-camera/assets/6020549/ba04f088-c628-46ac-bc5b-2968032753e0)

# View pictures using opencv-python   
You can use subscribe.py as image viewer.   
```
python3 -m pip install -U wheel
python3 -m pip install 'paho-mqtt>=1.0.0,<2.0.0'
python3 -m pip install opencv-python

python3 ./subscribe.py --help
usage: subscribe.py [-h] [--host HOST] [--port PORT] [--topic TOPIC] [--output OUTPUT]

options:
  -h, --help       show this help message and exit
  --host HOST      mqtt broker
  --port PORT      mqtt port
  --topic TOPIC    mqtt topic
  --output OUTPUT  output file name
```
__Close the image window with the ESC key.__   
![opencv](https://github.com/nopnop2002/esp-idf-mqtt-camera/assets/6020549/516b2f25-d285-47d6-ae56-ee1cceed5c58)   
This script works not only on Linux but also on Windows 10.   
![Image](https://github.com/user-attachments/assets/b1d4f037-3be3-4b02-bea7-6856aa2e1f8e)

# Save pictures using node.js   
You can use saveFile.js to save pictures.   
```
npm install mqtt
npm saveFile.js
```

# Take & view pictures using flask
Read [this](https://github.com/nopnop2002/esp-idf-mqtt-camera/tree/master/flask).   


# Take & view pictures using android
Read [this](https://github.com/nopnop2002/esp-idf-mqtt-camera/tree/master/dash).   


# Built-in WEB Server
ESP32 works as a web server.   
You can check the photos taken using the built-in WEB server.   
Enter the ESP32's IP address and port number in the address bar of your browser.   
You can connect using mDNS hostname instead of IP address.   

![browser](https://user-images.githubusercontent.com/6020549/124227364-837a7880-db45-11eb-9d8b-fa15c676adac.jpg)


# MQTT client Example
Example code in various languages.   
https://github.com/emqx/MQTT-Client-Examples


# References   
https://github.com/nopnop2002/esp-idf-mqtt-broker

https://github.com/nopnop2002/esp-idf-mqtt-image-viewer

https://github.com/nopnop2002/esp-idf-mqtt-image-client
