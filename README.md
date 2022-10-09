# esp-idf-mqtt-camera
Take a picture and Publish it via MQTT.   
This project use [ESP32 Camera Driver](https://github.com/espressif/esp32-camera).

![slide0001](https://user-images.githubusercontent.com/6020549/123804185-f73f3a00-d927-11eb-9bf8-918595583c1a.jpg)
![slide0002](https://user-images.githubusercontent.com/6020549/123804188-f8706700-d927-11eb-83ed-65d3aaf2a335.jpg)

# Hardware requirements
ESP32-CAM Development board.   
Support for OV2640 camera.   
If you use other camera, edit sdkconfig.default.   
![ESP32-CAM-1](https://user-images.githubusercontent.com/6020549/118466947-4fd2c300-b73e-11eb-8e64-23260e73e693.JPG)
![ESP32-CAM-2](https://user-images.githubusercontent.com/6020549/118466960-53664a00-b73e-11eb-8950-a5058516e1a3.JPG)

# Software requirements
esp-idf v4.4 or later.   

# Installation
Use a USB-TTL converter.   

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
git clone https://github.com/espressif/esp32-camera components/esp32-camera
idf.py set-target esp32
idf.py menuconfig
idf.py flash monitor
```

# Start firmware
Change GPIO0 to open and press the RESET button.

# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/99891822-7f35d500-2cb1-11eb-928c-be9a8191dec9.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/119053256-ca326a00-ba00-11eb-9fcc-c520957c6592.jpg)

## Wifi Setting

![config-wifi-1](https://user-images.githubusercontent.com/6020549/182987385-b885f6d2-f52a-4fc3-a338-c089547703a1.jpg)

You can use the mDNS hostname instead of the IP address.   
- esp-idf V4.3 or earlier   
 You will need to manually change the mDNS strict mode according to [this](https://github.com/espressif/esp-idf/issues/6190) instruction.   
- esp-idf V4.4 or later  
 If you set CONFIG_MDNS_STRICT_MODE = y in sdkconfig.default, the firmware will be built with MDNS_STRICT_MODE = 1.

![config-wifi-2](https://user-images.githubusercontent.com/6020549/182987378-4074dc87-05b6-4102-baf9-c0428eb54321.jpg)

You can use static IP.   
![config-wifi-3](https://user-images.githubusercontent.com/6020549/182987383-74bc02d7-678f-4aa3-87b1-41d3e99f279e.jpg)

## MQTT Server Setting

![config-mqtt](https://user-images.githubusercontent.com/6020549/194732213-8f7291e7-75b5-46c0-918d-c8002d19e1ff.jpg)

You can use [this](https://github.com/nopnop2002/esp-idf-mqtt-broker) as MQTT Server.   

## Select Frame Size
Large frame sizes take longer to take a picture.   

![config-framesize-1](https://user-images.githubusercontent.com/6020549/122478692-d233f880-d004-11eb-8176-cd4d750d1ed9.jpg)
![config-framesize-2](https://user-images.githubusercontent.com/6020549/122478707-d9f39d00-d004-11eb-9e55-70f4db369b41.jpg)

## Select Shutter

You can choose one of the following shutter methods

- Shutter is the Enter key on the keyboard   
For operation check

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

- Shutter is MQTT Publish   

![config-shutter-3](https://user-images.githubusercontent.com/6020549/99891865-d340b980-2cb1-11eb-9da5-944bd6a07c17.jpg)

You can use pub.py as shutter.   
Change the following according to your environment.   

```
# MQTT Broker
host = 'your_broker'
# MQTT Port
port = 1883
# MQTT Publish Topic
topic = '/topic/picture/sub'
```

Requires mqtt library.   
```
$ python3 -m pip install -U wheel
$ python3 -m pip install paho-mqtt
$ python3 pub.py
```

You can use mosquitto_pub as shutter.   

```
$ mosquitto_pub -h your_broker -p 1883 -t "/topic/picture/sub" -m ""
```


## Flash Light

ESP32-CAM by AI-Thinker have flash light on GPIO4.   

![config-flash](https://user-images.githubusercontent.com/6020549/122479023-6b630f00-d005-11eb-98cc-b5fdbf2987c7.jpg)


# Save pictures using python   
You can use saveFile.py to save pictures.   
Change the following according to your environment.   

```
# MQTT Broker
host = 'your_broker'
# MQTT Subscribe Topic
topic = '/image/esp32cam'
# Save File
saveFile = './output.jpg'
```

Requires library.   
```
$ python3 -m pip install -U wheel
$ python3 -m pip install paho-mqtt
$ python3 saveFile.py
```

# View pictures using python   
You can use viewFile.py to view pictures.   
Change the following according to your environment.   
Requires X-Window environment.   

```
# MQTT Broker
host = 'your_broker'
# MQTT Subscribe Topic
topic = '/image/esp32cam'
# Save File
saveFile = './output.jpg'
```

Requires library.   
```
$ python3 -m pip install -U wheel
$ python3 -m pip install paho-mqtt
$ python3 -m pip install opencv-python
$ python3 -m pip install numpy
$ python3 viewFile.py
```


# Save pictures using node.js   
You can use saveFile.js to save pictures.   
Change the following according to your environment.   

```
// MQTT Broker
host = 'mqtt://your_broker'
// MQTT Subscribe Topic
topic = '/image/esp32cam';
// Save File
saveFile = './output.jpg'
```

Requires mqtt library.   
```
$ npm install mqtt
$ npm saveFile.js
```

# Take & view pictures using flask
Read [this](https://github.com/nopnop2002/esp-idf-mqtt-camera/tree/master/flask).   


# Take & view pictures using android
Read [this](https://github.com/nopnop2002/esp-idf-mqtt-camera/tree/master/dash).   


# Built-in WEB Server
You can check the photos taken using the built-in WEB server.   
Enter the ESP32's IP address and port number in the address bar of your browser.   

![browser](https://user-images.githubusercontent.com/6020549/124227364-837a7880-db45-11eb-9d8b-fa15c676adac.jpg)

# MQTT client Example
Example code in various languages.   
https://github.com/emqx/MQTT-Client-Examples


# References   
https://github.com/nopnop2002/esp-idf-mqtt-broker

https://github.com/nopnop2002/esp-idf-mqtt-image-viewer

https://github.com/nopnop2002/esp-idf-mqtt-image-client
