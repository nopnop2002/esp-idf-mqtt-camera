# esp-idf-mqtt-camera
Take a picture and Publish it via MQTT.   

![スライド1](https://user-images.githubusercontent.com/6020549/99764072-90f76b00-2b3f-11eb-8cf1-a01b93e5309e.JPG)

![スライド2](https://user-images.githubusercontent.com/6020549/99764076-93f25b80-2b3f-11eb-915a-d1e39bb57295.JPG)

# Software requirements
esp-idf ver4.1 or later.   

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

# Start firmware
Change GPIO0 to open and press the RESET button.

# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/66692052-c17e9b80-ecd5-11e9-8316-075350ceb2e9.jpg)

![config-app](https://user-images.githubusercontent.com/6020549/66692054-c5aab900-ecd5-11e9-9b1d-b1200e555df5.jpg)

## Wifi Setting

![config-wifi](https://user-images.githubusercontent.com/6020549/66692062-e4a94b00-ecd5-11e9-9ea7-afb74cc347af.jpg)

## MQTT Server Setting

![config-mqtt](https://user-images.githubusercontent.com/6020549/66692167-b5dfa480-ecd6-11e9-9b56-02e662f67d38.jpg)

## Camera Pin

![config-camerapin](https://user-images.githubusercontent.com/6020549/66692087-1d492480-ecd6-11e9-8b69-68191005a453.jpg)

## Picture Size

![config-picturesize](https://user-images.githubusercontent.com/6020549/66692095-26d28c80-ecd6-11e9-933e-ab0be911ecd2.jpg)

## Select Shutter

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
