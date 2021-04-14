# coding=utf-8
# ebb_network.py
import gettext
import socket

try:
    from plot_utils_import import from_dependency_import
    inkex = from_dependency_import('ink_extensions.inkex')
    serial = from_dependency_import('serial')
except:
    import inkex
    import serial

from distutils.version import LooseVersion

use_network = 1;

def readline(s):
	if use_network == 1:
		msg=''
		chunk='a'
		while chunk != '\n':
			chunk=s.recv(1)
			if chunk == '':
				return msg
			if (chunk != '\n') and (chunk != '\r'):
				msg=msg+chunk
		return msg + "\r\n"
	else:
		s.readline()

def writeline(s, v):
	if use_network == 1:
		s.sendall(v);
	else:
		s.write(v);

def version():          # Version number for this document
	return "0.14"   # Dated 2019-06-18

def findPort():
	# Force network ??
	return "eggbot.local"
	# Find a D1 mini connected to a USB port.
	try:
		from serial.tools.list_ports import comports
	except ImportError:
		comports = None
		return "eggbot.local"
	if comports:
		comPortsList = list(comports())
		EBBport = "eggbot.local"
		for port in comPortsList:
			if port[1].startswith("USB-SERIAL CH340"):
				EBBport = port[0] 	#Success; EBB found by name match.
				break	#stop searching-- we are done.
		if EBBport == "eggbot.local":
			for port in comPortsList:
				if port[2].startswith("USB VID:PID=1a86:7523"):
					EBBport = port[0] #Success; EBB found by VID/PID match.
					break	#stop searching-- we are done.
		return EBBport
	return "eggbot.local"


def query_nickname(port_name, verbose=True):
    # Query the EBB nickname and report it.
    # This feature is only supported in firmware versions 2.5.5 and newer.
    # If verbose is True or omitted, the result will be human readable.
    # A short version is returned if verbose is False.
    # http://evil-mad.github.io/EggBot/ebb.html#QT
    if port_name is not None:
        version_status = min_version(port_name, "2.5.5")

        if version_status:
            raw_string = (query(port_name, 'QT\r'))
            if raw_string.isspace():
                if verbose:
                    return "This AxiDraw does not have a nickname assigned."
                else:
                    return None
            else:
                if verbose:
                    return "AxiDraw nickname: " + raw_string
                else:
#                     string_temp = str(raw_string).strip()
                    return str(raw_string).strip()
        elif version_status is False:
            if verbose:
                return "AxiDraw naming requires firmware version 2.5.5 or higher."

def write_nickname(port_name, nickname):
    # Write the EBB nickname.
    # This feature is only supported in firmware versions 2.5.5 and newer.
    # http://evil-mad.github.io/EggBot/ebb.html#ST
    if port_name is not None:
        version_status = min_version(port_name, "2.5.5")

        if version_status:
            try:
                cmd = 'ST,' + nickname + '\r'
                command(port_name,cmd)
                return True
            except:
                return False

def reboot(port_name):
    # Reboot the EBB, as though it were just powered on.
    # This feature is only supported in firmware versions 2.5.5 and newer.
    # It has no effect if called on an EBB with older firmware.
    # http://evil-mad.github.io/EggBot/ebb.html#RB
    if port_name is not None:
        version_status = min_version(port_name, "2.5.5")
        if version_status:
            try:
                command(port_name,'RB\r')
            except:
                pass


def list_port_info():
    # Find and return a list of all USB devices and their information.
    try:
        from serial.tools.list_ports import comports
    except ImportError:
        return None
    if comports:
        com_ports_list = list(comports())
        port_info_list = []
        for port in com_ports_list:
            port_info_list.append(port[0]) # port name
            port_info_list.append(port[1]) # Identifier
            port_info_list.append(port[2]) # VID/PID
        if port_info_list:
            return port_info_list


def listEBBports():
    # Find and return a list of all EiBotBoard units
    # connected via USB port.
    try:
        from serial.tools.list_ports import comports
    except ImportError:
        return None
    if comports:
        com_ports_list = list(comports())
        ebb_ports_list = []
        for port in com_ports_list:
            port_has_ebb = False
            if port[1].startswith("EiBotBoard"):
                port_has_ebb = True
            elif port[2].startswith("USB VID:PID=04D8:FD92"):
                port_has_ebb = True
            if port_has_ebb:
                ebb_ports_list.append(port)
        if ebb_ports_list:
            return ebb_ports_list


def list_named_ebbs():
    # Return discriptive list of all EiBotBoard units
    ebb_ports_list = listEBBports()
    if not ebb_ports_list:
        return
    ebb_names_list = []
    for port in ebb_ports_list:
        name_found = False
        p0 = port[0]
        p1 = port[1]
        p2 = port[2]
        if p1.startswith("EiBotBoard"):
            temp_string = p1[11:]
            if (temp_string):
                if temp_string is not None:
                    ebb_names_list.append(temp_string)
                    name_found = True
        if not name_found:
            # Look for "SER=XXXX LOCAT" pattern,
            #  typical of Pyserial 3 on Windows.
            if 'SER=' in p2 and ' LOCAT' in p2:
                index1 = p2.find('SER=') + len('SER=')
                index2 = p2.find(' LOCAT', index1)
                temp_string = p2[index1:index2]
                if len(temp_string) < 3:
                    temp_string = None
                if temp_string is not None:
                    ebb_names_list.append(temp_string)
                    name_found = True
        if not name_found:
            # Look for "...SNR=XXXX" pattern,
            #  typical of Pyserial 2.7 on Windows,
            #  as in Inkscape 0.91 on Windows
            if 'SNR=' in p2:
                index1 = p2.find('SNR=') + len('SNR=')
                index2 = len(p2)
                temp_string = p2[index1:index2]
                if len(temp_string) < 3:
                    temp_string = None
                if temp_string is not None:
                    ebb_names_list.append(temp_string)
                    name_found = True
        if not name_found:
            ebb_names_list.append(p0)
    return ebb_names_list


def testPort(port_name):
	if port_name == "eggbot.local":
		use_network = 1;
		try:
			serial_port = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			serial_port.connect(('eggbot.local', 2500))
			serial_port.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
			serial_port.setblocking(1)
			serial_port.sendall('v\r'.encode('ascii'))
			str_version = readline(serial_port)
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
	else:
		use_network = 0;
		try:
			serial_port = serial.Serial( port_name, timeout=3.0 ) # 1 second timeout!
			serial_port.write( 'v\r' )
			strVersion = serial_port.readline()
			if strVersion and strVersion.startswith( 'EBB' ):
				return serial_port
			serial_port.write( 'v\r' )
			strVersion = serial_port.readline()
			if strVersion and strVersion.startswith( 'EBB' ):
				return serial_port
			serial_port.close()
		except serial.SerialException:
			pass
		return None


def openPort():
	# Find and open a port to a single attached EiBotBoard.
	# The first port located will be used.
	found_port = findPort()
	serial_port = testPort(found_port)
	if serial_port:
		return serial_port


def open_named_port(port_name):
    # Find and open a port to a single attached EiBotBoard.
    # The first port located will be used.
    found_port = find_named_ebb(port_name)
    serial_port = testPort(found_port)
    if serial_port:
        return serial_port


def closePort(port_name):
    if port_name is not None:
        try:
            port_name.close()
        except serial.SerialException:
            pass


def query(port_name, cmd):
    if port_name is not None and cmd is not None:
        response = ''
        try:
            writeline(port_name,cmd.encode('ascii'))
            response = readline(port_name).decode('ascii')
            n_retry_count = 0
            while len(response) == 0 and n_retry_count < 100:
                # get new response to replace null response if necessary
                response = readline(port_name)
                n_retry_count += 1
            if cmd.strip().lower() not in ["v", "i", "a", "mr", "pi", "qm"]:
                # Most queries return an "OK" after the data requested.
                # We skip this for those few queries that do not return an extra line.
                unused_response = readline(port_name)  # read in extra blank/OK line
                n_retry_count = 0
                while len(unused_response) == 0 and n_retry_count < 100:
                    # get new response to replace null response if necessary
                    unused_response = readline(port_name)
                    n_retry_count += 1
        except:
            inkex.errormsg(gettext.gettext("Error reading serial data."))
        return response


def command(port_name, cmd):
    if port_name is not None and cmd is not None:
        try:
            writeline(port_name,cmd.encode('ascii'))
            response = readline(port_name).decode('ascii')
            n_retry_count = 0
            while len(response) == 0 and n_retry_count < 100:
                # get new response to replace null response if necessary
                response = readline(port_name).decode('ascii')
                n_retry_count += 1
            if response.strip().startswith("OK"):
                # Debug option: indicate which command:
                # inkex.errormsg( 'OK after command: ' + cmd )
                pass
            else:
                if response:
                    inkex.errormsg('Error: Unexpected response from EBB.')
                    inkex.errormsg('   Command: {0}'.format(cmd.strip()))
                    inkex.errormsg('   Response: {0}'.format(response.strip()))
                else:
                    inkex.errormsg('EBB Serial Timeout after command: {0}'.format(cmd))
        except:
            if cmd.strip().lower() not in ["rb"]: # Ignore error on reboot (RB) command
	            inkex.errormsg('Failed after command: {0}'.format(cmd))


def bootload(port_name):
    # Enter bootloader mode. Do not try to read back data.
    if port_name is not None:
        try:
            writeline(port_name,'BL\r'.encode('ascii'))
            return True
        except:
            return False


def min_version(port_name, version_string):
    # Query the EBB firmware version for the EBB located at port_name.
    # Return True if the EBB firmware version is at least version_string.
    # Return False if the EBB firmware version is below version_string.
    # Return None if we are unable to determine True or False.

    if port_name is not None:
        ebb_version_string = queryVersion(port_name)  # Full string, human readable
        ebb_version_string = ebb_version_string.split("Firmware Version ", 1)

        if len(ebb_version_string) > 1:
            ebb_version_string = ebb_version_string[1]
        else:
            return None  # We haven't received a reasonable version number response.

        ebb_version_string = ebb_version_string.strip()  # Stripped copy, for number comparisons
        if ebb_version_string is not "none":
            if LooseVersion(ebb_version_string) >= LooseVersion(version_string):
                return True
            else:
                return False


def queryVersion(port_name):
    return query(port_name, 'V\r')  # Query EBB Version String
