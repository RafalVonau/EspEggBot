# coding=utf-8
# test.py
import gettext
import socket
import serial


def readline(s):
    msg=''
    chunk='a'
    while chunk != '\n':
        chunk=s.recv(1)
        if chunk == '':
            return msg
        if (chunk != '\n') and (chunk != '\r'):
            msg=msg+chunk
    return msg + "\r\n"

def findPort():
	#Find a single EiBotBoard connected to a USB port.
	try:
		from serial.tools.list_ports import comports
	except ImportError:
		comports = None
		return "eggbot.local"
	if comports:
		comPortsList = list(comports())
		print(comPortsList)
		EBBport = "eggbot.local"
		for port in comPortsList:
			for port in comPortsList:
				if port[2].startswith("USB VID:PID=1a86:7523"):
					EBBport = port[0] #Success; EBB found by VID/PID match.
					break	#stop searching-- we are done.
		return EBBport
	return "eggbot.local"


def testPort(port_name):
    """
    Open a given serial port, verify that it is an EiBotBoard,
    and return a SerialPort object that we can reference later.

    This routine only opens the port;
    it will need to be closed as well, for example with closePort( port_name ).
    You, who open the port, are responsible for closing it as well.

    """
    print(port_name);
    if port_name is not None:
        try:
            serial_port = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            serial_port.connect(('eggbot.local', 2500))
            serial_port.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            serial_port.setblocking(1)
            serial_port.sendall('v\r\n'.encode('ascii'))
            str_version = readline(serial_port)
            print(str_version)
            if str_version and str_version.startswith("EBB".encode('ascii')):
                return serial_port

            serial_port.sendall('v\r'.encode('ascii'))
            str_version = readline(serial_port)
            if str_version and str_version.startswith("EBB".encode('ascii')):
                return serial_port
            serial_port.close()
        except:
            pass
        return None

print(testPort(findPort()));

