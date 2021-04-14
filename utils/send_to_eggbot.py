#!/usr/bin/env python2

import socket	#for sockets
import sys	#for exit
import time	#for exit
import argparse
import serial
from datetime import datetime

use_network = 1;

parser = argparse.ArgumentParser(description='Process some arguments.')
parser.add_argument('-s','--serial' ,help='serial port')
parser.add_argument('-l', '--local', action='store_true')
parser.add_argument('-i','--input' ,help='input file')
args = parser.parse_args()

def readline(s):
	if use_network == 1:
		msg=''
		chunk='a'
		while chunk != '\r':
			chunk=s.recv(1)
			if chunk == '':
				return msg
			if (chunk != '\n') and (chunk != '\r'):
				msg=msg+chunk
		return msg
	else:
		return s.readline().strip()

def writeline(s, v):
	if use_network == 1:
		s.sendall(v);
	else:
		s.write(v);

def sendCommand(s, x):
	if (x != ""):
		print('Send: '+x)
		try :
			writeline(s, x + "\r")
		except socket.error:
			print('Send failed')
			sys.exit()
		reply = readline(s)
		print reply


host = 'eggbot.local';
port = 2500;
inputfile = "test.gcode"

if args.local:
	host="127.0.0.1"

if args.serial:
	use_network = 0;
	host = args.serial

if args.input:
	inputfile = args.input
  
print("Host: "+host);


old_out = sys.stdout

class St_ampe_dOut:
	nl = True
	def write(self, x):
		if x == '\n':
			old_out.write(x)
			self.nl = True
		elif self.nl:
			old_out.write('(%s): %s' % (str(datetime.now().strftime("%H:%M:%S.%f"))[:-3], x))
			self.nl = False
		else:
			old_out.write(x)

sys.stdout = St_ampe_dOut()



if (use_network==1):
	#create an INET, STREAMing socket
	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	except socket.error:
		print('Failed to create socket')
		sys.exit()
	print('Socket Created')
	try:
		remote_ip = socket.gethostbyname( host )
	except socket.gaierror:
		#could not resolve
		print('Hostname could not be resolved. Exiting')
		sys.exit()
	#Connect to remote server
	s.connect((remote_ip , port))
	s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
	s.setblocking(1)
	print('Socket Connected to ' + host + ' on ip ' + remote_ip)
else:
	try:
		s = serial.Serial(host,baudrate=9600,parity=serial.PARITY_NONE,stopbits=serial.STOPBITS_ONE,bytesize=serial.EIGHTBITS,timeout=10.0)
	except serial.SerialException:
		print('Failed to open serial port')
		sys.exit()


file1 = open(inputfile, 'r')
Lines = file1.readlines()

count = 0
# Strips the newline character
for line in Lines:
	count += 1
	sendCommand(s, line.strip())
sendCommand(s,"X\r")
s.close()
time.sleep(1.0)
file1.close()

