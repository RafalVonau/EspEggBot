#ifndef SIMPLESWITCH_H
#define SIMPLESWITCH_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}
#include "Ticker.h"
#include <FunctionalInterrupt.h>

class SimpleSwitch;

/* 1 - button press (immediatelly), 2 - button still pressed after holdDetect time, 3 - button still pressed after holdDetect2 time */
typedef std::function<void(SimpleSwitch *, int butonEvent)> SimpleSwitchCb;

class SimpleSwitch
{
public:
	SimpleSwitch(int gpioPin, SimpleSwitchCb cb, int debounceDelay = 10, int holdDetect = 20, int holdDetect2 = 2000) {
		m_CB               = cb;
		m_gpioPin          = gpioPin;
		m_gpioPinMask      = (1<<gpioPin);
		m_lastDebounceTime = 0;
		m_debounceDelay    = debounceDelay;
		m_holdDetect       = holdDetect;
		m_holdDetect2      = holdDetect2;
		pinMode(gpioPin,INPUT_PULLUP);
		attachInterrupt(gpioPin, std::bind(&SimpleSwitch::onChange, this), CHANGE);
	}
	~SimpleSwitch() {}
protected:
	void ICACHE_RAM_ATTR onChange(void);
	void onHold(void);
	void onHold2(void);
public:
	SimpleSwitchCb  m_CB;
	int      m_gpioPin;
	int      m_gpioPinMask;
	int      m_debounceDelay;
	int      m_holdDetect;
	int      m_holdDetect2;
	Ticker   m_ticker;
	volatile int m_state;
	volatile long m_lastDebounceTime;
};
//==========================================================================================


#endif
