/*
 * Base class for commands (SerialCommand, NetworkCommand, HTTPCommand).
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#include "Command.h"

//====================================================================================
//============================-- Command Queue --=====================================
//====================================================================================

CommandQueueItem::CommandQueueItem(Command *c, char *cmdline, CommandQueueCB cb)
{
	char *arg;
	cmddebug("CommandQueueItem::constructor\n");
	m_parent     = c;
	m_cb         = cb;
	m_arg_mask   = 0;
	/* Parse Arguments */
	arg = strtok_r(NULL, ",", &cmdline);
	if (arg) {
		if (arg[0] == 'B') m_arg0=0xff; else m_arg0 = atoi(arg);
		m_arg_mask |= 1;
		arg = strtok_r(NULL, ",", &cmdline);
		if (arg) {
			m_arg1 = atoi(arg);
			m_arg_mask |= 2;
			arg = strtok_r(NULL, ",", &cmdline);
			if (arg) {
				m_arg2 = atoi(arg);
				m_arg_mask |= 4;
			}
		}
	}
}
//====================================================================================

CommandQueueItem::~CommandQueueItem()
{
	cmddebug("CommandQueueItem::destructor\n");
}
//====================================================================================

void CommandQueueItem::print(String s)
{
	m_parent->print(s);
}
//====================================================================================

//====================================================================================
//==============================-- Command DB --======================================
//====================================================================================

void CommandDB::addCommand(const char *command, CommandQueueCB cb, bool waitMotors)
{
	cmddebug2("Add command <%s>\n",command);
	m_commandMap[String(command)] = std::make_shared<CommandDBItem>(cb, waitMotors);
}
//====================================================================================

void CommandDB::executeCommand(Command *c, char *line)
{
	char *last = NULL, *command;
	
	cmddebug2("Execute command <%s>\n",line);
	command = strtok_r(line, ",", &last);   // Search for command at start of buffer
	if (command != NULL) {
		auto it = m_commandMap.find(String(command));
		if (it != m_commandMap.end()) {
			/* Push command to command queue */
			CommandQueueItemPtr cqi = std::make_shared<CommandQueueItem>(c, last, it->second->m_cb);
			if (it->second->m_waitMotors) {
				m_motionQueue.emplace_back(cqi);
			} else {
				m_commandQueue.emplace_back(cqi);
			}
		} else if (m_defaultHandler != NULL) {
			cmddebug2("Command not found <%s>!\n",command);
			(*m_defaultHandler)(command, c);
		}
	}
}
//====================================================================================

