import socket, sys, time, struct

import sys
import serial
import threading
import os
import time
import struct
from gui import *

textport = "8080"
host = "piquopter.olin.edu"

#thread thats constantly reading from the tcp socket
class Reader(threading.Thread):
	def __init__(self, socket, g):
		super(Reader, self).__init__()
		self.socket = socket
		self.gui = g

	def run(self):
		count = 0
		data = ""
		#write each frame got into a file
		while 1:
			count +=1
			self.fil = open("current.jpg", 'wb+')
			l = []
			if host == "localhost":
				chars = self.socket.recv(8)
				l = struct.unpack('<Q', chars)
			else:
				chars = self.socket.recv(4)
				l = struct.unpack('<L', chars)
			print l
			#bad state for l if too big
			b5 = ''
			b4 = ''
			b3 = ''
			b2 = ''
			b1 = ''
			b0 = ''
			upper = 0
			#if this is the case, there was an error in the data sent. Go though until one finds the 0xffd8 which is the start of a jpg.
			while (l[0] > 50000):
				upper += 1
				if (upper%100 == 0):
					print upper
				b0 = self.socket.recv(1)
				if (b1 == chr(0xff) and b0 == chr(0xd8)):
					#print "bytes found", ord(b2), ord(b3), ord(b1), ord(b0)
					l = struct.unpack('<L',  str(b5) + str(b4) + str(b3) + str(b2))
					print "found new l", l
				#shift the buffer down
				b5 = b4
				b4 = b3
				b3 = b2
				b2 = b1
				b1 = b0
			#in a standard state, just write the bytes to the jpg.
			read_at_a_time = 8
			bytes_left = l[0]
			while bytes_left > 0:
				if bytes_left > read_at_a_time:
					self.fil.write(self.socket.recv(read_at_a_time));
				else:
					self.fil.write(self.socket.recv(bytes_left));
				bytes_left -= read_at_a_time
			self.gui.update()
			print "Done with frame!"