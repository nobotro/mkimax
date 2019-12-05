import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.template
import socket
import threading
import os
from handlers import *
from collections import OrderedDict

import tornado.speedups


def get_frame():
    global frame, frame_dict
    framedata = b''

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 15000)
    s.bind(('0.0.0.0', 1327))

    while (1):
        data, _ = s.recvfrom(50000)

        if data.startswith(b'\x00\x00\x00\x01'):
            frame = framedata

            framedata = b''
            for uid in frame_dict.keys():
                if not frame_dict[uid]:
                    frame_dict[uid] = frame

        framedata += data


application = tornado.web.Application([

    (r'/stream/([a-zA-Z0-9]{16})/([a-zA-Z0-9]{1,30})', StreamHandler),
    (r'/chat/([a-zA-Z0-9]{16})', ChatHandler),
    (r'/drive/([a-zA-Z0-9]{16})', DriveHandler),
    (r'/', MainHandler),
    (r'/info/([a-zA-Z0-9]{16})', InfoHandler),
    (r'/(.*)', tornado.web.StaticFileHandler, {"path": os.path.join(os.path.dirname(__file__), '../static')}),

],websocket_ping_timeout=1)

x = threading.Thread(target=get_frame, args=())
x.start()

drivem = threading.Thread(target=drivermanager, args=(application,))
drivem.start()

if __name__ == "__main__":
    # application.listen(9090)
    # tornado.ioloop.IOLoop.instance().start()
    server = tornado.httpserver.HTTPServer(application)
    server.bind(9090)

    server.start(1)  # Specify number of subprocesses

    tornado.ioloop.IOLoop.current().start()

