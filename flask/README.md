# Take & View pictures using flask
You can take & view pictures using WEB interface.   

![slide3](https://user-images.githubusercontent.com/6020549/187050703-8df7abac-a3b7-4272-b412-ad2455f75e96.JPG)


# Install flask & flask-mqtt
```
sudo apt install python3-pip python3-setuptools python3-magic
python3 -m pip install -U pip
python3 -m pip install -U wheel
python3 -m pip install -U Werkzeug
python3 -m pip install -U flask
python3 -m pip install -U flask-mqtt
```

# Start WEB Server using Flask
```
git clone https://github.com/nopnop2002/esp-idf-mqtt-camera
cd esp-idf-mqtt-camera/flask
python3 mqtt-client.py --help
```

![flask-1](https://user-images.githubusercontent.com/6020549/187065193-1749fb4c-adec-4acc-b4cb-cf2223a37bdb.jpg)

Start application with broker.   
![flask-2](https://user-images.githubusercontent.com/6020549/187065194-de78ae81-fe9e-4b17-8b8f-a00e73cfc1fa.jpg)


# Take picture
ESP32 shutter is configured with mqtt subscribe.   
![flask-11](https://user-images.githubusercontent.com/6020549/187050741-bd27e1e5-cee1-46c8-98f2-bd9cc91dbf88.jpg)

Refresh web page.   
![flask-12](https://user-images.githubusercontent.com/6020549/187050744-c2da31c2-7e46-4996-aa6a-afa5e33c1377.jpg)
![flask-13](https://user-images.githubusercontent.com/6020549/187050745-a4ee65c8-2b0c-476b-8ea3-9c57af66c3d8.jpg)

