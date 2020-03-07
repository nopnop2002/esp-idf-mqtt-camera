# esp-idf-mqtt-camera
MQTT camera for esp-idf.   
Take a picture and Publish it via MQTT.   

![0001](https://user-images.githubusercontent.com/6020549/66692500-ea089480-ecd9-11e9-9f3a-b843d60eca47.jpg)

![0002](https://user-images.githubusercontent.com/6020549/66692498-ea089480-ecd9-11e9-93fa-b011a5103140.jpg)

# Install
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

# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/66692052-c17e9b80-ecd5-11e9-8316-075350ceb2e9.jpg)

![config-app](https://user-images.githubusercontent.com/6020549/66692054-c5aab900-ecd5-11e9-9b1d-b1200e555df5.jpg)

## Wifi

![config-wifi](https://user-images.githubusercontent.com/6020549/66692062-e4a94b00-ecd5-11e9-9ea7-afb74cc347af.jpg)

## MQTT Server

![config-mqtt](https://user-images.githubusercontent.com/6020549/66692167-b5dfa480-ecd6-11e9-9b56-02e662f67d38.jpg)

## Camera Pin

![config-camerapin](https://user-images.githubusercontent.com/6020549/66692087-1d492480-ecd6-11e9-8b69-68191005a453.jpg)

## Picture Size

![config-picturesize](https://user-images.githubusercontent.com/6020549/66692095-26d28c80-ecd6-11e9-933e-ab0be911ecd2.jpg)

## Shutter method

You can choose one of the following shutter methods

![config-shutter-1](https://user-images.githubusercontent.com/6020549/66692107-381b9900-ecd6-11e9-8d73-1ee7423c5188.jpg)

- Shutter is the Enter key on the keyboard   
For operation check

![config-shutter-2](https://user-images.githubusercontent.com/6020549/66692119-4964a580-ecd6-11e9-9695-bfd3d61dc20a.jpg)

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

![config-shutter-3](https://user-images.githubusercontent.com/6020549/66692126-5b464880-ecd6-11e9-8823-e8a9a5fa1eed.jpg)

- Shutter is MQTT Publish   
You can use pub.py.   

![config-shutter-4](https://user-images.githubusercontent.com/6020549/66692137-6c8f5500-ecd6-11e9-90ef-b83981ea4809.jpg)

## Flash Light

ESP32-CAM by AI-Thinker have flash light on GPIO4.

![config-flash](https://user-images.githubusercontent.com/6020549/66263918-5ac13400-e836-11e9-9511-7db58aa147b1.jpg)

# How to save picture file
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
