/*
 * NetworkCommand - Execite eggbot commands over a TCP/IP stream.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef NetworkCommand_h
#define NetworkCommand_h

#include "Command.h"
#include <ESPAsyncTCP.h>

class NetworkCommand: public Command{
public:
	NetworkCommand(CommandDB *db, int port):Command(db) {
		m_client = 0;
		m_server = new AsyncServer(port);
		m_server->onClient([](void* arg, AsyncClient* client) {
			cmddebug("TCP:New client\n");
			NetworkCommand *p = ((NetworkCommand*)(arg));
			client->setNoDelay(true);
			p->m_client = client;
			client->onDisconnect([](void* arg, AsyncClient* client) {
				cmddebug("TCP:DisConnect\n");
				NetworkCommand *p = ((NetworkCommand*)(arg));
				if (p->m_client == client) p->m_client = 0;
			}, p);
			client->onData([](void *narg, AsyncClient* client, void *data, size_t len){((NetworkCommand*)(narg))->handleData((char *)data, (int)len); }, p);
		}, this);
		m_server->begin();
	};      // Constructor

	~NetworkCommand() {
		delete m_server;
	}

	virtual void print(String s) {
		if (m_client) {
			m_client->add(s.c_str(),s.length());
			m_client->send();
		}
		cmddebug(s);
	}
	virtual void readSerial() {};
	void handleData(char *data, int len);
public:
	AsyncServer    *m_server;
	AsyncClient    *m_client;
};

#endif //NetworkCommand_h
