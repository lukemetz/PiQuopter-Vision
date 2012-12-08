from Tkinter import *
from PIL import Image, ImageTk
from Reader import *
import imghdr
import socket

#network stuff
textport = "8080"
host = "piquopter.olin.edu"
#host = "localhost"
port = int(textport)
addr = (host, port)
buf = 1024
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(addr)


class GUI(object):
    def __init__(self):
        self.active = False
        self.command = 'stop'
        self.setup()

    def move(self, code, value):
        self.command = 'C'+str(code)+value
        self.sendData(self.command); 
        print self.command

    def start(self):
        if  self.throttle.get()==0:
            self.active = True
            for c in self.controls:
                c.configure(state = 'normal')
                c.set(0)
            print 'started'

    def stop(self):
        self.active = False
        print 'stopped'
        self.command= 'stop'

    def key(self,event):
        key = event.keysym
        if key == 'Escape':
            code = 'stop'
            value = 0
            self.move(0,'0')
            self.move(1,'0')
            self.move(2,'0')
            self.move(3,'0')
            self.throttle.set(0)
            self.yaw.set(0)
            self.pitch.set(0)
            self.roll.set(0)
            self.command='stop'
            return

        elif key == 'space':
            code = 0
            value = 0
            self.move(0,'0')
            self.move(1,'0')
            self.move(2,'0')
            self.yaw.set(0)
            self.pitch.set(0)
            self.roll.set(0)
            
            return
        elif key == 'w':
            code = 3
            value = self.throttle.get()+1
            self.throttle.set(value)
        elif key == 's':
            code = 3
            value = self.throttle.get()-1
            self.throttle.set(value)
        elif key == 'a':
            code = 0
            value = self.yaw.get()-1
            self.yaw.set(value)
        elif key == 'd':
            code = 0
            value = self.yaw.get()+1
            self.yaw.set(value)
        elif key == 'Up':
            code = 1
            value = self.pitch.get()+1
            self.pitch.set(value)
        elif key == 'Down':
            code = 1
            value = self.pitch.get()-1
            self.pitch.set(value)
        elif key == 'Left':
            code = 2
            value = self.roll.get()-1
            self.roll.set(value)
        elif key == 'Right':
            code = 2
            value = self.roll.get()+1
            self.roll.set(value)
        self.move(code,str(value))

    def update(self):
        print "good"
        if imghdr.what("current.jpg") == 'jpeg':
            self.im = Image.open("current.jpg")
            if (not self.im.verify()):
                print "BAD"
                return
            self.tkim = ImageTk.PhotoImage(self.im)
            #self.tkim.place(x=0, y=0, width=320, height=240);
            self.lab = Label(self.root, image=self.tkim)
            self.lab.image = self.im
            self.lab.pack()
            print "uppen the things"
            self.lab.grid(row = 1, column = 1)
        else:
            print "got bad"


    def setup(self):
        self.root = Tk()

        #bindings
        self.root.bind("w", self.key)
        self.root.bind("a", self.key)
        self.root.bind("s", self.key)
        self.root.bind("d", self.key)

        self.root.bind("<Up>", self.key)
        self.root.bind("<Left>", self.key)
        self.root.bind("<Right>", self.key)
        self.root.bind("<Down>", self.key)
        self.root.bind("<Escape>", self.key)
        self.root.bind("<space>", self.key)

        #wigets:
        sidebar_bg = '#228800'

        title = Label(text="PiQuopter", background="#4400FF",  font=("Helvetica", "16"))

        leftbar = Frame(bg = sidebar_bg, bd=0)
        start = Button(leftbar, text = 'start', command=lambda: self.start())
        self.throttle = Scale(leftbar, variable=IntVar(), orient='vertical', length=200,from_=100, to=0, bg = sidebar_bg, bd=0, tickinterval=5, state='disabled', command=lambda(x): self.move(3,x))
        self.yaw = Scale(leftbar, variable=IntVar(), orient='horizontal', length=200,from_=-50, to=50, bg = sidebar_bg, tickinterval=5, state='disabled',command=lambda(x): self.move(0,x))

        self.screen = Canvas(width=320, height=240)
            # add canvasy stuff
        '''self.im = Image.open("current.jpg")
        self.tkim = ImageTk.PhotoImage(self.im)
        #self.tkim.place(x=0, y=0, width=320, height=240);
        self.lab = Label(self.root, image=self.tkim)
        self.lab.image = self.im
        self.lab.pack()
        #self.lab.place(x=0, y=0, width=320, height=240)
        '''
        rightbar = Frame(background=sidebar_bg)
        stop = Button(rightbar, text='stop', command=lambda: self.stop())
        self.pitch = Scale(rightbar, variable=IntVar(), orient='vertical', length=200,from_=50, to=-50, bg = sidebar_bg, tickinterval=5, state='disabled', command=lambda(x): self.move(1,x))
        self.roll = Scale(rightbar, variable=IntVar(), orient='horizontal', length=200,from_=-50, to=50, bg = sidebar_bg, tickinterval=5, state='disabled', command=lambda(x): self.move(2,x))

        self.controls = [self.throttle, self.yaw, self.pitch, self.roll]
        #placing:
        title.grid(row=0, column=0, columnspan=3, sticky=E+W)

        leftbar.grid(row=1, column=0, sticky=N+S)
        start.pack(fill='both')
        self.throttle.pack(fill='x')
        self.yaw.pack()



        rightbar.grid(row=1, column=2, sticky=N+S)
        stop.pack(fill='x')
        self.pitch.pack()
        self.roll.pack()


    def sendData(self,command):
        sock.send(command);
        r = Reader(sock)
        r.start()

def main():
    #host = "localhost"
    gui = GUI()
    port = int(textport)
    addr = (host, port)
    buf = 1024
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    sock.connect(addr)
    #sock.send("C1200");
    r = Reader(sock, gui)
    print "are u sure?"
    r.start()
    print "did u block?"
    gui.root.mainloop()

    #write as well as read from socket.

    #while(1):
    #    k = raw_input()
    #    sock.send(k)

    sys.exit()

if (__name__ == "__main__"):
  main()









