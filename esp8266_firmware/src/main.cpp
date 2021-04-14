/*
 * EggD1-Firmware by Rafal Vonau (ESP8266 port)
 *
 * Based on: Eggduino-Firmware by Joachim Cerny, 2014
 *
 * Thanks to the Eggbot-Team for such a funny and enjoable concept!
 * Thanks to my wife and my daughter for their patience. :-)
 */

// implemented Eggbot-Protocol-Version v13
// EBB-Command-Reference, I sourced from: http://www.schmalzhaus.com/EBB/EBBCommands.html
// no homing sequence, switch-on position of pen will be taken as reference point.
// No collision-detection!!
// Note: Maximum-Speed in Inkscape is 1000 Steps/s. You could enter more, but then Pythonscript sends nonsense.
// EBB-Coordinates are coming in for 16th-Microstepmode. The Coordinate-Transforms are done in weired integer-math. Be careful, when you diecide to modify settings.

// ==-- HW connection --==
// STEP1(EGG)- GPIO14 (D5)
// DIR1(EGG) - GPIO13 (D7)
// STEP2(ARM)- GPIO5  (D1)
// DIR2(ARM) - GPIO4  (D2)
// MOTTOR EN - GPIO2  (D4)
// SERVO     - GPIO12 (D6)
// BUTTON    - GPIO0  (D3)
//========================

#include <ESP8266WiFi.h>
extern "C" {
	#include <osapi.h>
	#include <os_type.h>
}
#include <functional>
#include <ESPAsyncTCP.h>       /* ESPAsyncTCP-esphome@1.2.2                   */
#include <ArduinoOTA.h>
#include "SerialCommand.h" //nice lib from Stefan Rado, https://github.com/kroimon/Arduino-SerialCommand
#include "NetworkCommand.h"
#include "HTTPCommand.h"
#include <EEPROM_Rotate.h>
#include "simpleswitch.h"
#include "Motion2D.h"
#include <DNSServer.h>

/* SWITCHES */
#define HOSTNAME                 "eggbot"
#define NPORT                    (2500)

#define WIFI_SSID                "EggBot"
#define WIFI_PASS                "EggBotPass"

#if 1
#undef DEBUG_ENABLED
#define pdebug(fmt, args...)
#define pwrite(fmt, len)
#else
#define DEBUG_ENABLED
#define pdebug(fmt, args...) Serial.printf(fmt, ## args)
#define pwrite(fmt, len) Serial.write(fmt, len)
#endif

/*=================*/
/* PIN definitions */
/*=================*/

//Rotational Stepper: ("X - EGG")
#define step1        14
#define dir1         13
#define enableMotor  2
#define rotMicrostep 16      // MicrostepMode, only 1,2,4,8,16 allowed, because of Integer-Math in this Sketch
//Pen Stepper:        ("Y - ARM")
#define step2        5
#define dir2         4
#define penMicrostep 16      // MicrostepMode, only 1,2,4,8,16 allowed, because of Integer-Math in this Sketch
//Servo
#define servoPin     12          // "SpnEn"
//#define engraverPin 13       // "SpnDir"
//Buttons
//#define prgButton 0         // PRG button ("Abort")
#define penToggleButton 0   // pen up/down button ("Hold")
//#define motorsButton 0      // motors enable button ("Resume")


//====================================================================================
//=================================-- VARIABLES --====================================
//====================================================================================
DNSServer          dnsServer;
Motion2D          *m2d;
CommandDB         CmdDB;
SerialCommand     *SCmd;
NetworkCommand    *NCmd;
HTTPCommand       *HCmd;
EEPROM_Rotate     EEPROMr;
uint32_t          ee_cache[8];
//create Buttons
#ifdef prgButton
	SimpleSwitch *prg;
#endif
#ifdef penToggleButton
	SimpleSwitch *penToggle;
#endif
#ifdef motorsButton
	SimpleSwitch *motorsToggle;
#endif
int servoRateUp           = 0;
int servoRateDown         = 0;
uint32_t nodeCount        = 0;
unsigned int layer        = 0;
boolean prgButtonState    = 0;
int otaInProgress         = 0;
//====================================================================================
//============================-- FUNCTION DECLARATION --==============================
//====================================================================================

/* Helpers */
void makeComInterface();
void loadPenPosFromEE();
void queryPen();
void queryButton();

/* Long Commands (short commands are implemented as Lambdas) */
void enableMotors(CommandQueueItem *c);
void stepperModeConfigure(CommandQueueItem *c);
void setPen(CommandQueueItem *c);
void togglePen(CommandQueueItem *c);
void stepperMove(CommandQueueItem *c);
void setEngraver(CommandQueueItem *c);
void pinOutput(CommandQueueItem *c);
void setNodeCount(CommandQueueItem *c);
void setLayer(CommandQueueItem *c);
void unrecognized(const char *command, Command *c) {c->print("!8 Err: Unknown command\r\n");}


//====================================================================================
//=================================-- SETUP/LOOP --===================================
//====================================================================================

/*!
 * \brief SETUP.
 */
void setup()
{
	int WifiCounter = 0, got_one = 1;

	Serial.begin(9600);
	pdebug("SETUP:START\n");
	/* Setup GPIO pins */
	pinMode(enableMotor,OUTPUT);
	digitalWrite(enableMotor, HIGH);

	/* Connect to WiFi */
	digitalWrite(enableMotor, LOW);
	WiFi.hostname(HOSTNAME);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	pdebug("Connecting to WiFi network\n");
	WiFi.begin ( WIFI_SSID, WIFI_PASS );
	while ( WiFi.status() != WL_CONNECTED ) {
		pdebug(".");
		delay (500);
		WifiCounter++;
		if (WifiCounter >=20) {
			got_one = 0;
			break;
		}
	}
	if (got_one == 0) {
		pdebug("Starting in AP mode\n");
		DNSServer dnsServer;
		WiFi.softAP("EspEggBootAP");
		dnsServer.start(53, "*", WiFi.softAPIP());
	}

	digitalWrite(enableMotor, HIGH);
	/* Setup OTA */
	ArduinoOTA.setHostname(HOSTNAME);
	ArduinoOTA.onStart([]() {
		otaInProgress = 1;
	});
	ArduinoOTA.begin();
	makeComInterface();
#ifdef engraverPin
	pinMode(engraverPin, OUTPUT);
#endif
	m2d = new Motion2D(step1, dir1, step2, dir2, enableMotor,servoPin);
	loadPenPosFromEE();
	m2d->setPenUp();

#ifdef prgButton
	prg = new SimpleSwitch(prgButton, [](SimpleSwitch *s, int v) ICACHE_RAM_ATTR {if (v == 2) prgButtonState = 1;});
#endif
#ifdef penToggleButton
	penToggle = new SimpleSwitch(penToggleButton, [](SimpleSwitch *s, int v) ICACHE_RAM_ATTR {if (v == 2) schedule_function([](){m2d->togglePen();});});
#endif
#ifdef motorsButton
	motorsToggle = new SimpleSwitch(motorsButton, [](SimpleSwitch *s, int v) ICACHE_RAM_ATTR {if (v == 2) schedule_function(toggleMotors);});
#endif
	pdebug("SETUP:DONE\n");
}
//====================================================================================

/*!
 * \brief LOOP.
 */
void loop()
{
	ArduinoOTA.handle();
	if (otaInProgress) return;
	/* Execute command from queue */
	if ( m2d->loop() ) {
		CmdDB.loop();
	} else {
		CmdDB.loopMotion();
		CmdDB.loop();
	}
	SCmd->loop();
}
//====================================================================================


uint32_t eeprom_read_v(int x)
{
	uint32_t v;
	int a = x + 10;
	v = (((uint32_t)EEPROMr.read(a) << 24) | ((uint32_t)EEPROMr.read(a+1) << 16) | ((uint32_t)EEPROMr.read(a+2) << 8) | ((uint32_t)EEPROMr.read(a+3)));
	ee_cache[(x>>2)] = v;
	return v;
}
//====================================================================================

void eeprom_write_v(int x, uint32 v)
{
	uint32 vv = ee_cache[(x>>2)];
	if (v != vv) {
		int a = x + 10;
		EEPROMr.write(a, (v >> 24) & 0xff);
		EEPROMr.write(a+1, (v >> 16) & 0xff);
		EEPROMr.write(a+2, (v >> 8) & 0xff);
		EEPROMr.write(a+3, v& 0xff);
		EEPROMr.commit();
	}
}
//===========================================================================================

/*!
 * \brief Read configuration from eeprom.
 */
void loadPenPosFromEE()
{
	EEPROMr.size(8);
	EEPROMr.begin(4096);
	m2d->m_penUpPos      = eeprom_read_v(0);
	m2d->m_penDownPos    = eeprom_read_v(4);
	m2d->m_servoRateUp   = eeprom_read_v(8);
	m2d->m_servoRateDown = eeprom_read_v(12);
}
//====================================================================================


void makeComInterface()
{
	pdebug("makeComInterface:START\n");
	CmdDB.addCommand("v",[](CommandQueueItem *c) {
		c->print("EBBv13_and_above Protocol emulated by EggD1-Firmware V1.x\r\n");
	});
	CmdDB.addCommand("EM",enableMotors);
	CmdDB.addCommand("SC",stepperModeConfigure);
	CmdDB.addCommand("SP",setPen, true);
	CmdDB.addCommand("SM",stepperMove, true);
	CmdDB.addCommand("SE",setEngraver);
	CmdDB.addCommand("TP",togglePen, true);
	CmdDB.addCommand("PD",[](CommandQueueItem *c){ c->sendAck();});
	CmdDB.addCommand("PO",pinOutput);
	CmdDB.addCommand("NI",[](CommandQueueItem *c){nodeCount++;c->sendAck();});
	CmdDB.addCommand("ND",[](CommandQueueItem *c){nodeCount--;c->sendAck();});
	CmdDB.addCommand("SN",setNodeCount);
	CmdDB.addCommand("QN",[](CommandQueueItem *c){c->printInt(nodeCount);});
	CmdDB.addCommand("SL",setLayer);
	CmdDB.addCommand("QL",[](CommandQueueItem *c){c->printInt(layer);});
	CmdDB.addCommand("QP",[](CommandQueueItem *c){c->printInt((m2d->m_penState == m2d->m_penUpPos)?0:1);});
	CmdDB.addCommand("QB",[](CommandQueueItem *c){c->printInt(prgButtonState);prgButtonState = 0;});  //"PRG" Button,
	//CmdDB.addCommand("XX",[](CommandQueueItem *c){c->printInt(tticks);});
	CmdDB.addCommand("XX",[](CommandQueueItem *c){m2d->printStat(c);});
	CmdDB.setDefaultHandler(unrecognized); // Handler for command that isn't matched (says "What?")

	SCmd = new SerialCommand(&CmdDB);
	NCmd = new NetworkCommand(&CmdDB, NPORT);
	HCmd = new HTTPCommand(&CmdDB);
	pdebug("makeComInterface:DONE\n");
}
//====================================================================================

void setLayer(CommandQueueItem *c)
{
	if (c->m_arg_mask & 1) {
		layer = c->m_arg0;
		c->sendAck();
	} else {
		c->sendError();
	}
}
//====================================================================================

void setNodeCount(CommandQueueItem *c) 
{
	if (c->m_arg_mask & 1) {
		nodeCount = c->m_arg0;
		c->sendAck();
	} else {
		c->sendError();
	}
}
//====================================================================================

void stepperMove(CommandQueueItem *c)
{
	uint16_t duration = c->m_arg0;
	int penStepsEBB   = c->m_arg1;
	int rotStepsEBB   = c->m_arg2;

	if ((c->m_arg_mask & 7) != 7) {
		c->sendError();
		return;
	}
	c->sendAck();
	if ( (penStepsEBB==0) && (rotStepsEBB==0) ) {
		delay(duration);
		return;
	}
	m2d->goTo(duration, rotStepsEBB, penStepsEBB);
}
//====================================================================================

void setPen(CommandQueueItem *c)
{
	int v = 0, cmd = -1;

	if (c->m_arg_mask & 1) cmd = c->m_arg0;
	if (c->m_arg_mask & 2) v = c->m_arg1;
	switch (cmd) {
		case 0: {c->sendAck();m2d->setPenDown(v);}break;
		case 1: {c->sendAck();m2d->setPenUp(v);}break;
		default: c->sendError();break;
	}
}
//====================================================================================

void togglePen(CommandQueueItem *c)
{
	int v = 500;

	if (c->m_arg_mask & 1) v = c->m_arg0;
	c->sendAck();
	m2d->togglePen(v);
}
//====================================================================================

void enableMotors(CommandQueueItem *c)
{
	int value = -1, cmd = -1;

	if (c->m_arg_mask & 1) cmd = c->m_arg0;
	if (c->m_arg_mask & 2) value = c->m_arg1;

	if ((cmd != -1) && (value == -1)){
		switch (cmd) {
			case 0: {m2d->motorsOff();c->sendAck();}break;
			case 1: {m2d->motorsOn();c->sendAck();}break;
			default: c->sendError(); break;
		}
	}
	if ((cmd != -1) && (value != -1)) {
		switch (value) {
			case 0: {m2d->motorsOff();c->sendAck();}break;
			case 1: {m2d->motorsOn();c->sendAck();}break;
			default: c->sendError(); break;
		}
	}
}
//====================================================================================

void stepperModeConfigure(CommandQueueItem *c)
{
	int cmd = 0, value = 0;

	if ((c->m_arg_mask & 3) == 3) {
		cmd   = c->m_arg0;
		value = c->m_arg1;
		switch (cmd) {
			case 4: {
				m2d->m_penUpPos= (int) ((float) (value-6000)/(float) 133.3); // transformation from EBB to PWM-Servo
				eeprom_write_v(0, m2d->m_penUpPos);
				c->sendAck();
			} break;
			case 5: {
				m2d->m_penDownPos= (int)((float) (value-6000)/(float) 133.3); // transformation from EBB to PWM-Servo
				eeprom_write_v(4, m2d->m_penDownPos);
				c->sendAck();
			} break;
			case 6: {
				//rotMin=value;    ignored
				c->sendAck();
			} break;
			case 7: {
				//rotMax=value;    ignored
				c->sendAck();
			} break;
			case 11: {
				m2d->m_servoRateUp = value/5;
				eeprom_write_v(8, m2d->m_servoRateUp);
				c->sendAck();
			} break;
			case 12: {
				m2d->m_servoRateDown = value/5;
				eeprom_write_v(12, m2d->m_servoRateDown);
				c->sendAck();
			} break;
			default:c->sendError();break;
		}
	}
}
//====================================================================================

void pinOutput(CommandQueueItem *c)
{
	if ((c->m_arg_mask & 7) != 7) {
		c->sendError();
		return;
	}
	//PO,B,3,0 = disable engraver
	//PO,B,3,1 = enable engraver
#ifdef engraverPin
	if ((c->m_arg0 == 0xff ) && (c->m_arg1 == 3)) {
		digitalWrite(engraverPin, c->m_arg2);
	}
#endif
	c->sendAck();
}
//====================================================================================

//currently inkscape extension is using PO command for engraver instead of SE
void setEngraver(CommandQueueItem *c)
{
#ifdef engraverPin
	if (c->m_arg_mask & 1) digitalWrite(engraverPin, c->m_arg0);
#endif
	c->sendAck();
}
//====================================================================================


