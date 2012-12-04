#!/usr/bin/python

import socket, sys, time, struct

import sys
import serial
import threading
import os
import time

#Class just to print out whats on the socket.
class Reader(threading.Thread):
	def __init__(self, socket):
		super(Reader, self).__init__()
		self.socket = socket
	def run(self):
		while 1:
			print self.socket.recv(128);

def main():
	textport = "8080"
	host = "localhost"
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