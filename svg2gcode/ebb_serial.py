# coding=utf-8
# ebb_serial.py
import gettext
import socket

try:
	from plot_utils_import import from_dependency_import
	inkex = from_dependency_import('ink_extensions.inkex')
except:
	import inkex

from distutils.version import LooseVersion

def writeline(s, v):
	print(v)

def version():          # Version number for this document
	return "0.14"   # Dated 2019-06-18

def findPort():
	return "eggbot.local"

def reboot(port_name):
	print("UPS - no reboot support !");

def testPort(port_name):
	serial_port = [1,]
	return serial_port;

def openPort():
	found_port = findPort()
	serial_port = testPort(found_port)
	if serial_port:
		return serial_port

def closePort(port_name):
	pass;

def query(port_name, cmd):
	return '0';


def command(port_name, cmd):
	writeline(port_name,cmd.encode('ascii'))


def bootload(port_name):
	return False


def min_version(port_name, version_string):
	return None  # We haven't received a reasonable version number response.

def queryVersion(port_name):
	return "13"  # Query EBB Version String
