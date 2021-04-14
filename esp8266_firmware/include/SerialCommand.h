/*
 * SerialCommand - Execite eggbot commands over a serial port.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef __SERIALCOMMAND_H__
#define __SERIALCOMMAND_H__

#include "Command.h"


class SerialCommand: public Command{
public:
	SerialCommand(CommandDB *db):Command(db) {};
	virtual void print(String s) {Serial.print(s);}
	virtual void loop();
private:
};

#endif // __SERIALCOMMAND_H__
