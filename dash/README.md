# MQTT Dash Board

I found [this](https://play.google.com/store/apps/details?id=net.routix.mqttdash&gl=US) app.   
You can take pictures using your Android.   
You can immediately see the photos you took.   

![slide4](https://user-images.githubusercontent.com/6020549/200440711-8f4aba78-8202-4fd4-b9eb-d5a7d1f79e38.JPG)

# Download Android App   
You need MQTT Dash (IoT, Smart Home).   
You can download from [Google Play](https://play.google.com/store/apps/details?id=net.routix.mqttdash&gl=US).   

# Setup Dash board   

## Setup Broker
I use local server (192.168.10.40) as broker.   

![broker](https://user-images.githubusercontent.com/6020549/188520878-705e04fb-c902-44e6-bf7b-a0715846b482.jpg)


## Add Shutter button   
- Name:shutter
- Topic(sub):(empty)
- Topic(pub):/take/picture
- Payload(on):1
- Payload(off):0
- Icon Image:Choose your favorite one
- Icon Color:Choose your favorite one

![shutter](https://user-images.githubusercontent.com/6020549/200437683-6933cbc9-745c-41cb-ad37-84b76c06cfe5.jpg)

## Add Image object
- Image file data receive as binary payload contents
- Name:image
- Topic(sub):/image/esp32cam

![image](https://user-images.githubusercontent.com/6020549/188521109-7316299d-82c3-407a-be62-80ce370afa79.jpg)


## How to use
Press the shutter button to display the pictire in the Image area.   
![dash-1](https://user-images.githubusercontent.com/6020549/200439117-be0ccca1-7369-403f-9003-aa2108c446d9.jpg)
![dash-2](https://user-images.githubusercontent.com/6020549/200439121-ee0ab137-f1c3-4635-b85a-a364e4cf007a.jpg)
