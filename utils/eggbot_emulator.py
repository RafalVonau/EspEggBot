#!/usr/bin/env python2

import svgwrite
import socket
import sys
import argparse

HOST = ''	# Symbolic name, meaning all available interfaces
PORT = 2500	# Arbitrary non-privileged port
SVGFILE = "test.svg"
parser = argparse.ArgumentParser(description='Process some arguments.')
parser.add_argument('-o','--output' ,help='output file')

args = parser.parse_args()

if args.output:
	SVGFILE=args.output

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('Socket created')

#Bind socket to local host and port
try:
	s.bind((HOST, PORT))
except socket.error as msg:
	print('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
	sys.exit()

print('Socket bind complete')

#Start listening on socket
s.listen(10)
print('Socket now listening')

dwg = svgwrite.Drawing(SVGFILE, profile='tiny')

def readline(s):
	msg=''
	chunk='a'
	while chunk != '\r':
		chunk=s.recv(1)
		if chunk == '':
			return msg
		if (chunk != '\n') and (chunk != '\r'):
			msg=msg+chunk
	return msg

#Function for handling connections.
def clientthread(conn):
	#Sending message to connected client
	#infinite loop so that function do not terminate and thread do not end.
	x = 0
	y = 0
	p = 0
	while True:
		#Receiving from client
		d = readline(conn)
		if not d:
			break
		print(d);
		if (d == "v"):
			conn.sendall(b"EBBv13_and_above Protocol emulated by EggD1-Firmware V1.x\r\n")
		elif (d.startswith("SP")):
			conn.sendall(b"OK\r\n")
			sx = d.split(",")
			p = int(sx[1])
		elif (d.startswith("SM")):
			sx = d.split(",")
			nx = x + int(sx[2])
			ny = y + int(sx[3])
			if (p == 0):
				dwg.add(dwg.line((x, y), (nx, ny), stroke=svgwrite.rgb(10, 10, 16, '%')))
			x = nx
			y = ny
			conn.sendall(b"OK\r\n")
		elif (d.startswith("QB")):
			conn.sendall(b"0\r\nOK\r\n")
		elif (d.startswith("QP")):
			conn.sendall(b"0\r\nOK\r\n")
		elif (d.startswith("QN")):
			conn.sendall(b"0\r\nOK\r\n")
		elif (d.startswith("QL")):
			conn.sendall(b"0\r\nOK\r\n")
		elif (d.startswith("X")):
			break
		else:
			conn.sendall(b"OK\r\n")
		#conn.sendall(reply)
	#came out of loop
	conn.close()

#now keep talking with the client
while 1:
	#wait to accept a connection - blocking call
	conn, addr = s.accept()
	print('Connected with ' + addr[0] + ':' + str(addr[1]))
	clientthread(conn)
	dwg.save()
s.close()

#dwg = svgwrite.Drawing('test.svg', profile='tiny')
#dwg.add(dwg.line((0, 0), (10, 0), stroke=svgwrite.rgb(10, 10, 16, '%')))
#dwg.add(dwg.text('Test', insert=(0, 0.2), fill='red'))
#dwg.save()
