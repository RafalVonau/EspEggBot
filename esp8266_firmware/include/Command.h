/*
 * Base class for commands (SerialCommand, NetworkCommand, HTTPCommand).
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <Arduino.h>
#include <memory>
#include <list>
#include <map>
#include <utility>
#include <string.h>
#include <functional>

#if 1
#define cmddebug(x)
#define cmddebug2(fmt, args...)
#else
#define cmddebug(x) Serial.print(x)
#define cmddebug2(fmt, args...) Serial.printf(fmt, ## args)
#endif

// Size of the input buffer in bytes (maximum length of one command plus arguments)
#define COMMAND_BUFFER (63)

class Command;
class CommandQueueItem;

typedef std::function<void(CommandQueueItem *c)> CommandQueueCB;

/*!
 * \brief Queued command item.
 */
class CommandQueueItem {
public:
	CommandQueueItem(Command *c, char *cmdline, CommandQueueCB cb);
	~CommandQueueItem();
	void print(String s);
	void printInt(int i) {this->print(String(i) +"\r\nOK\r\n");       }
	void sendAck()       {this->print("OK\r\n");                      }
	void sendError()     {this->print("!8 Err: Unknown command\r\n"); }
	void execute() {m_cb(this);}                          // Execute (use calback function)
public:
	int            m_arg0;
	int            m_arg1;
	int            m_arg2;
	int            m_arg_mask;
	Command       *m_parent;        // Pointer to parent (SerialCommand or NetworkCommand)
	CommandQueueCB m_cb;            // Calback function
};
typedef std::shared_ptr<CommandQueueItem> CommandQueueItemPtr;


class CommandDBItem {
public:
	CommandDBItem(CommandQueueCB cb, bool waitMotors): m_cb(cb), m_waitMotors(waitMotors) {}
	CommandQueueCB m_cb;
	bool           m_waitMotors;
};
typedef std::shared_ptr<CommandDBItem> CommandDBItemPtr;



/*!
 * \brief Commands Database.
 */
class CommandDB {
public:
	CommandDB(): m_defaultHandler(NULL) {}
	/*!
	 * \brief Add command to the database.
	 * \param command    - command name,
	 * \param fn         - calback function,
	 * \param waitMotors - if true use motion queue instand of command queue.
	 */
	void addCommand(const char *command, CommandQueueCB fn, bool waitMotors = false);
	/*!
	 * \brief Set handler called when command was not found in the database.
	 */
	void setDefaultHandler(void (*function)(const char *, Command *c)) {m_defaultHandler = function;}
	/*!
	 * \brief Parse command line and add command to queue.
	 */
	void executeCommand(Command *c, char *line);
	/*!
	 * \brief Execute single command from command queue.
	 */
	void loop() {
		if (m_commandQueue.size()) {
			m_commandQueue.front()->execute();
			m_commandQueue.pop_front();
		}
	}
	/*!
	 * \brief Execute single command from motion queue.
	 */
	void loopMotion() {
		if (m_motionQueue.size()) {
			m_motionQueue.front()->execute();
			m_motionQueue.pop_front();
		}
	}
public:
	/* Command database */
	std::map<String, CommandDBItemPtr> m_commandMap;
	// Pointer to the default handler function
	void (*m_defaultHandler)(const char *, Command *c);
	/* Command Queue */
	std::list<CommandQueueItemPtr> m_commandQueue;
	/* Motion Queue */
	std::list<CommandQueueItemPtr> m_motionQueue;
};

/*!
 * \brief Base class for Commands.
 */
class Command {
public:
	Command(CommandDB *db):  m_db(db) {clearBuffer();}      // Constructor

	virtual void print(String s) {}
	virtual void loop() {};
	
	void clearBuffer() { buffer[0] = '\0';bufPos = 0; }  // Clears the input buffer.	
public:
	char       buffer[COMMAND_BUFFER + 1]; // Buffer of stored characters while waiting for terminator character
	byte       bufPos;                     // Current position in the buffer
	CommandDB *m_db;                       // Commands database
};

#endif //__COMMAND_H__

