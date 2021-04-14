#!/usr/bin/env python2

import os
import urllib2
class EnhancedFile(file):
    def __init__(self, *args, **keyws):
        file.__init__(self, *args, **keyws)

    def __len__(self):
        return int(os.fstat(self.fileno())[6])

theFile = EnhancedFile('test.svg', 'r')
theUrl = "https://tymek.duckdns.org/eggbot/svg2gcode.py"
theHeaders= {'Content-Type': 'text/xml'}

theRequest = urllib2.Request(theUrl, theFile, theHeaders)
response = urllib2.urlopen(theRequest)
theFile.close()

for line in response:
    print line
