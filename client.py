#!/usr/bin/python

import socket, sys, time, struct

import sys
import serial
import threading
import os
import time
import struct

textport = "8080"
host = "piquopter.olin.edu"

#Class just to print out whats on the socket.
class Reader(threading.Thread):
	def __init__(self, socket):
		super(Reader, self).__init__()
		self.socket = socket
	def run(self):
		count = 0
		data = ""

		while 1:
			count +=1
			self.fil = open(str(count)+"moo.jpg", 'wb+')
			l = []
			if host == "localhost":
				chars = self.socket.recv(8)
				l = struct.unpack('<Q', chars)
			else:
				#f (count == 1):
				chars = self.socket.recv(4)
				otherfi = open(str(count)+"size.hex", 'wb+')
				otherfi.write(chars)
				otherfi.close()
				l = struct.unpack('<L', chars)
				'''else:
					chars = self.socket.recv(32)
					#chars += self.socket.recv(32)
					print "DOIN THE THING"
					otherfi = open(str(count)+"size.hex", 'wb+')
					otherfi.write(chars)
					otherfi.close()'''
			print l
			#bad state for l if too big
			b5 = ''
			b4 = ''
			b3 = ''
			b2 = ''
			b1 = ''
			b0 = ''
			upper = 0
			while (l[0] > 50000):
				upper += 1
				if (upper%100 == 0):
					print upper
				b0 = self.socket.recv(1)
				if (b1 == chr(0xff) and b0 == chr(0xd8)):
					print "asdfasdf", ord(b2), ord(b3), ord(b1), ord(b0)
					l = struct.unpack('<L',  str(b5) + str(b4) + str(b3) + str(b2))
					print "found new l", l

				b5 = b4
				b4 = b3
				b3 = b2
				b2 = b1
				b1 = b0

			read_at_a_time = 8
			bytes_left = l[0]

			while bytes_left > 0:
				if bytes_left > read_at_a_time:
					self.fil.write(self.socket.recv(read_at_a_time));
				else:
					self.fil.write(self.socket.recv(bytes_left));
				bytes_left -= read_at_a_time
				#print bytes_left
			print "Done with frame!"
			

def main():
	

	#host = "localhost"
	port = int(textport)
	addr = (host, port)
	buf = 1024
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	sock.connect(addr)
	sock.send("C1200");
	r = Reader(sock)
	r.start()

	#write as well as read from socket.
	while(1):
		k = raw_input()
		sock.send(k)

	sys.exit()

if (__name__ == "__main__"):
  main()