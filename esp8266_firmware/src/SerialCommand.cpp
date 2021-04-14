/*
 * SerialCommand - Execite eggbot commands over a serial port.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#include "SerialCommand.h"

void SerialCommand::loop() 
{
	while (Serial.available() > 0) {
		char inChar = Serial.read();
		if ((inChar == '\r') || (inChar == '\n')) {
			if (bufPos) {
				buffer[bufPos] = '\0';
				m_db->executeCommand(this, buffer);
				clearBuffer();
			}
		} else if (isprint(inChar)) {
			if (bufPos < COMMAND_BUFFER) {
				buffer[bufPos++] = inChar;
			}
		}
	}
}
//====================================================================================
