from collections import OrderedDict

import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.template
import threading
import random
import string
import json
import time
import datetime

import socket
import tornado.speedups

thread_lock = threading.Lock()
current_driver_uid=None
current_driver_queue=0
drive_start_date=None
current_driver_is_online=False

carsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
carsocket.bind(('0.0.0.0',7777))
carsock_addr=()

frame=b''
#structure is user['userid']=[username,chathandler,drivehandler,frame]
users=OrderedDict()
frame_dict={}

def sender(client,uid):
    global frame_dict

    frame_dict[uid] = None

    while client.ws_connection:

        data=frame_dict[uid]
        frame_dict[uid] = None
        #check if bufer is empty and we have data
        if data and len(client.stream._write_buffer)==0:
           client.write_message(data, True)
        else:time.sleep(0.01)





def get_car_address():
    global carsock_addr
    data,addr=carsocket.recvfrom(1025)
    carsock_addr=addr







def drivermanager(app):
    global current_driver_queue,current_driver_is_online,current_driver_uid,users
    get_car_address()
    current_driver_queue = 0
    while True:
            usrlen=len(users)
            if usrlen==0 or not carsock_addr:
                time.sleep(1)
                continue
            current_driver_uid=users.keys()[current_driver_queue % usrlen]
            current_driver_is_online=True
            drive_start_date=datetime.datetime.now()
            while(current_driver_is_online and (datetime.datetime.now()-drive_start_date).seconds<60):
                time.sleep(1)

            carsocket.sendto(b"!'s'", carsock_addr)
            carsocket.sendto(b"!'w'", carsock_addr)
            carsocket.sendto(b"!'a'", carsock_addr)
            carsocket.sendto(b"!'d'", carsock_addr)

            current_driver_queue+=1

def randomStringDigits(prefix,stringLength=16):

    lettersAndDigits = string.ascii_letters + string.digits
    return prefix+(''.join(random.choice(lettersAndDigits) for i in range(stringLength-len(prefix))))



def execute_synced( func, *args):
    thread_lock.acquire()
    res = func(*args)
    thread_lock.release()
    return res


def synced_add_user(uid,name):

    global users

    users[uid] = [name]
    return uid


def send_broadcast(usersdict,action,data,exclude=None):
    chatcons=[us[1] for us in usersdict.values() if us[0]!=exclude]

    threads=[]
    for chatcon in list(chatcons):
        sendth = threading.Thread(target=chatcon.write_message, args=(json.dumps({"action": action, "data": data[:180]},ensure_ascii=False),))
        sendth.start()
        threads.append(sendth)
    for th in threads:
        th.join()



class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("../templetes/register.html")

    def post(self):
        global users
        name=self.get_argument('name', '', True)
        if not name or len(name)>30:
            self.write('name is null or too big')
            return
        else:

            usercount = len(users)
            uid = randomStringDigits(str(usercount) + 'p')
            while uid in users:
                uid = randomStringDigits(str(usercount) + 'p')
            self.render("../templetes/index.html", uid=uid,user=name)

class InfoHandler(tornado.web.RequestHandler):
    def get(self,uid):
        global users
        if uid not in users.keys():
            self.write('error')
            return

        currdr='-'
        iam_driver=False
        if current_driver_uid:
            currdr=users[current_driver_uid][0]
        usron=len(users)
        curq=current_driver_queue % usron


        queuen=users.keys().index(uid)

        if curq<=queuen:
            queuen=queuen-curq
        else:
            queuen=(len(users)-curq)+queuen
        if current_driver_uid==uid:
            iam_driver=True

        self.write(json.dumps({'curdriver': currdr,'userscount':usron,'queue':queuen,'you_are_driver':iam_driver}))


class StreamHandler(tornado.websocket.WebSocketHandler):
  def open(self,uid,name):


    execute_synced(synced_add_user, uid,name)
    self.set_nodelay(True)

    sendth = threading.Thread(target=sender, args=(self,uid))
    sendth.start()

  def on_close(self):
    global users

    uid = self.path_args[0]
    if uid in users:
        del (users[uid])
    print('connection closed...')


class ChatHandler(tornado.websocket.WebSocketHandler):
    def open(self,uid):
        global users
        self.set_nodelay(True)
        if uid and uid  in users:
            users[uid].insert(1,self)
        else:
            self.close()


    def on_message(self, message):
        global users
        if not message or not message.replace(" ","") or not message.replace("\n","") or not message.replace("\t",""):
            return
        message=message
        uid = self.path_args[0]
        sender=users[uid][0]
        data="%s:>%s" % (sender, message)
        execute_synced(send_broadcast, users, 'send',data, sender)


    def on_close(self):
        global users
        uid=self.path_args[0]
        if uid in users:
            del(users[uid])
        print('connection closed...')


class DriveHandler(tornado.websocket.WebSocketHandler):
    def open(self,uid):


        global users
        if uid and uid in users:
            users[uid].insert(2, self)
        else:
            self.close()


    def on_message(self, message):
        self.set_nodelay(True)
        print message

        uid = self.path_args[0]
        if current_driver_uid==uid and carsock_addr:
            carsocket.sendto(message,carsock_addr)

    def on_close(self):

        global current_driver_is_online, current_driver_uid, users,carsocket
        if carsock_addr:
            carsocket.sendto(b"!'s'", carsock_addr)
            carsocket.sendto(b"!'w'", carsock_addr)
            carsocket.sendto(b"!'a'", carsock_addr)
            carsocket.sendto(b"!'d'", carsock_addr)


        uid = self.path_args[0]

        if uid in users:
            del(users[uid])
        if current_driver_uid == uid:
            current_driver_is_online=False
            current_driver_uid=None
        print('connection closed...')
