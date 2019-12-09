# Mkimax

This project is about how to drive rc car online ,global web site , with video streaming.

Video demonstration
https://youtu.be/ASDACCriIlc

# Instruction

Used hardware : uvc fpv camera usb receiver,fpv camera, wltoys rc car , esp32 dev board ,nrf24l01 wireless module

1)
- Flash micropython firmware on esp32 dev board
- Put "main.py", "rc.py","regs.py" and "server.py" from "wltoys-v202-micropython" directory to esp32 board with micropython firmware
- Connect nrf pins correctly(we use software spi) 'miso': 32, 'mosi': 33, 'sck': 25, 'csn': 26, 'ce': 27


2)
- Upload  "httpserver" directory to server
- run "python2  httpserver/src.server.py",its start webserver which listening on 80 port

3)
- Set your server ip adress in "camstreamer/main.c" and compile project
- Connect uvc fpv camera usb receiver to yor pc
- Turn on yor fpv camera
- Run "camstreamer/server" on your pc to start video streaming to global website

4)
- Connect flashed and ready esp32 board to power
- Run "httpserver/src/udp_gateway.py" to your pc
- There are two mode inside "udp_gateway.py"
   - mode 0 ,esp32 works as gateway between car and your computer,you need free wifi adapter for that
   - mode 1 ,esp32 works as gateway between car and server
- Go to your global website and start driving

5)
- Webserver is multi user so many user can go to website same time
- Drivers will by queueing to drive car.each driver have 60 sec to drive,
after 60 sec driver will be next user.
- Users ordered by consecutive
- Users also can chat with each other

6)
Used libraries

- Broadway - for front side h264 decode and play
  - https://github.com/mbebenita/Broadway
  
- libuvc - for capture frames from uvc fpv camera usb receiver
  - https://github.com/libuvc/libuvc
  
- libswscale - for converting frame from yuyv to YUV420p 
  - https://ffmpeg.org/libswscale.html
  
- libx264 - for encode YUV420p frame to .h264
  - https://videolan.org/developers/x264.html

- tornado - python module for web server,it can handle websocket and webrequests same time
  - https://www.tornadoweb.org
 




