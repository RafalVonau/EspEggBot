#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

import cgi, os
import cgitb; cgitb.enable()
import eggbot
import tempfile

#temp_name = next(tempfile._get_candidate_names())
#print('Content-type: application/octet-stream') # the mime-type header.
#print() # header must be separated from body by 1 empty line.
form = cgi.FieldStorage()
# Get filename here.
fileitem = form['filename']
# Test if the file was uploaded
if fileitem.filename:
	fn = os.path.basename(fileitem.filename.replace("\\", "/" ))
	gcn = os.path.splitext(fn)[0] + ".gcode";
	print("Content-Disposition: attachment; filename=\"" + gcn + "\"")
	print("Access-Control-Allow-Origin: *")
	print("Content-type: text/plain\n")
	file_name='/tmp/' + next(tempfile._get_candidate_names())
	open(file_name, 'wb').write(fileitem.file.read())
	e = eggbot.EggBot()
	e.affect([file_name,])
	os.remove(file_name)
