/*
 * NetworkCommand - Execite eggbot commands over a TCP/IP stream.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#include "NetworkCommand.h"

void NetworkCommand::handleData(char *data, int n) 
{
	int i;
	
	for (i=0; i < n; ++i) {
		char inChar = data[i];
		if ((inChar == '\r') || (inChar == '\n')) {
			if (bufPos) {
				buffer[bufPos] = '\0';
				m_db->executeCommand(this, buffer);
				clearBuffer();
			}
		} else if (isprint(inChar)) {     // Only printable characters into the buffer
			if (bufPos < COMMAND_BUFFER) {
				buffer[bufPos++] = inChar;  // Put character into buffer
			}
		}
	}
}
//====================================================================================
