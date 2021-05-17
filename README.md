# esp-idf-mqtt-camera
Take a picture and Publish it via MQTT.   

![スライド1](https://user-images.githubusercontent.com/6020549/99764072-90f76b00-2b3f-11eb-8cf1-a01b93e5309e.JPG)

![スライド2](https://user-images.githubusercontent.com/6020549/99764076-93f25b80-2b3f-11eb-915a-d1e39bb57295.JPG)

# Software requirements
esp-idf v4.0.2-120.   
git clone -b release/v4.0 --recursive https://github.com/espressif/esp-idf.git

esp-idf v4.1-520.   
git clone -b release/v4.1 --recursive https://github.com/espressif/esp-idf.git

esp-idf v4.2-beta1-227.   
git clone -b release/v4.2 --recursive https://github.com/espressif/esp-idf.git

__It does not work with esp-idf v4.3.__
__Even if I fix [this](https://github.com/espressif/esp-idf/pull/6029), I still get a panic.__

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
git clone https://github.com/espressif/esp32-camera components
make menuconfig
make flash monitor
```

# Start firmware
Change GPIO0 to open and press the RESET button.

# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/99891822-7f35d500-2cb1-11eb-928c-be9a8191dec9.jpg)

![config-app](https://user-images.githubusercontent.com/6020549/99891825-852bb600-2cb1-11eb-8946-ab9ed2253390.jpg)

## Wifi Setting

![config-wifi](https://user-images.githubusercontent.com/6020549/99891826-8ceb5a80-2cb1-11eb-8470-f5ceb9e4576a.jpg)

## MQTT Server Setting

![config-mqtt](https://user-images.githubusercontent.com/6020549/99891829-95439580-2cb1-11eb-8aad-3f697fa0513b.jpg)

## Camera Pin

![config-camerapin](https://user-images.githubusercontent.com/6020549/99891832-9ffe2a80-2cb1-11eb-8941-4b31ce900ea9.jpg)

## Picture Size

![config-picturesize](https://user-images.githubusercontent.com/6020549/99891839-ad1b1980-2cb1-11eb-94b7-832152fdb167.jpg)

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
#host = '192.168.10.40'
host = 'test.mosquitto.org'
# MQTT Port
port = 1883
# MQTT Publish Topic
topic = '/topic/picture/sub'
```

![config-shutter-3](https://user-images.githubusercontent.com/6020549/99891865-d340b980-2cb1-11eb-9da5-944bd6a07c17.jpg)

## Flash Light

ESP32-CAM by AI-Thinker have flash light on GPIO4.

![config-flash](https://user-images.githubusercontent.com/6020549/99891870-e18ed580-2cb1-11eb-8e9a-6ed44633d4f2.jpg)

# How to save picture file using python   
You can use saveFile.py.   
Please change the following according to your environment.   

```
# MQTT Broker
#host = '192.168.10.40'
host = 'test.mosquitto.org'
# MQTT Port
port = 1883
# MQTT Subscribe Topic
topic = '/topic/picture/pub'
# Save File
saveFile = './output.jpg'
```

# References   
https://github.com/nopnop2002/esp-idf-mqtt-broker
