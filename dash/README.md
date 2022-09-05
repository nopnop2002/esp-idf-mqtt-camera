# MQTT Dash Board

I found [this](https://play.google.com/store/apps/details?id=net.routix.mqttdash&gl=US) app.   
You can take pictures using your Android.   
You can immediately see the photos you took.   


# Download Android App   
You need MQTT Dash (IoT, Smart Home).   
You can download from [Google Play](https://play.google.com/store/apps/details?id=net.routix.mqttdash&gl=US).   

# Setup Dash board   

## Setup Broker
I use local server as broker.   

![broker](https://user-images.githubusercontent.com/6020549/188520878-705e04fb-c902-44e6-bf7b-a0715846b482.jpg)


## Add Shutter button   
- Name:shutter
- Topic(sub):(empty)
- Topic(pub):/topic/picture/sub
- Payload(on):1
- Payload(off):0
- Icon Image:Choose your favorite one
- Icon Color:Choose your favorite one

![shutter](https://user-images.githubusercontent.com/6020549/188520937-75d5c258-2f81-4500-b282-22994ac39297.jpg)

## Add Image object
- Image file data receive as binary payload contents
- Name:image
- Topic(sub):/image/esp32cam

![image](https://user-images.githubusercontent.com/6020549/188521109-7316299d-82c3-407a-be62-80ce370afa79.jpg)


## How to use
Press the shutter button to display the pictire in the Image area.   
![dash-1](https://user-images.githubusercontent.com/6020549/188521165-b420f28f-d403-45b9-81ce-3117f3959f72.jpg)
![dash-2](https://user-images.githubusercontent.com/6020549/188521166-f4509e18-8f9b-4aef-976a-b91faedd487a.jpg)
