import threading
import socket


MODE=0


def configure():
	global MODE
	if MODE==0:
		gateway=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
		gateway.sendto('hello'.encode(),('35.198.140.227',7777))

		while True:

		    data,_=gateway.recvfrom(1024)
		    print data
		    gateway.sendto(data, ('192.168.4.1', 1327))
	elif MODE==1:


  #get config
  #config message looks like 
  #mode;essid;passwords;serverip;serverport

  #when mode=0(old method) use udp_gateway
  #when mode=1 rc controler conect received essid and start receiving data from server


		#mode;essid;passwords;serverip;serverport
		data='1;GL-AR300M-f2f;mari+nino+1+2+3+4+5+6;35.198.140.227;7777'.encode()
		gateway=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
		gateway.sendto(data, ('192.168.4.1', 1327))

configure()




