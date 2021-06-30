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

![config-wifi](https://user-images.githubusercontent.com/6020549/99891826-8ceb5a80-2cb1-11eb-8470-f5ceb9e4576a.jpg)

## MQTT Server Setting

![config-mqtt](https://user-images.githubusercontent.com/6020549/119053264-cc94c400-ba00-11eb-8ab3-844c1ba61e38.jpg)

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
You can use pub.py.   
Please change the following according to your environment.   

```
# MQTT Broker
host = 'broker.emqx.io'
# MQTT Port
port = 1883
# MQTT Publish Topic
topic = '/topic/picture/sub'
```

![config-shutter-3](https://user-images.githubusercontent.com/6020549/99891865-d340b980-2cb1-11eb-9da5-944bd6a07c17.jpg)

## Flash Light

ESP32-CAM by AI-Thinker have flash light on GPIO4.   

![config-flash](https://user-images.githubusercontent.com/6020549/122479023-6b630f00-d005-11eb-98cc-b5fdbf2987c7.jpg)

# How to save picture file using python   
You can use saveFile.py.   
Please change the following according to your environment.   

```
# MQTT Broker
host = 'broker.emqx.io'
# MQTT Port
port = 1883
# MQTT Subscribe Topic
topic = 'image/#'
# Save File
saveFile = './output.jpg'
```

# MQTT client Example
Example code in various languages.   
https://github.com/emqx/MQTT-Client-Examples

# References   
https://github.com/nopnop2002/esp-idf-mqtt-broker

https://github.com/nopnop2002/esp-idf-mqtt-image-viewer

