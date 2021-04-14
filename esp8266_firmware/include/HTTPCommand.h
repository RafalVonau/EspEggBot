/*
 * HTTPCommand - implements a web page and allows to execute eggbot commands from a web browser (direct printing of SVG files).
 *
 * NOTE: The class uses a script written in python and hosted at https://tymek.duckdns.org/eggbot/togcode.py 
 *       to convert the SVG file into GCODE commands.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef __HTTPCOMMAND_H__
#define __HTTPCOMMAND_H__

#include "Command.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>


class HTTPCommand: public Command {
public:
	HTTPCommand(CommandDB *db);
	~HTTPCommand();

	virtual void print(String s) {
		if (m_events) {
			m_events->send(s.c_str(), "cmd");
		}
		cmddebug(s);
	}
	virtual void readSerial() {};
	void handleData(const char *data, int len);
public:
	AsyncWebServer   *m_server;
	AsyncEventSource *m_events;
};

#endif __HTTPCOMMAND_H__
