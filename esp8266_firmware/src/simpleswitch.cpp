#include "simpleswitch.h"

#if 1
#undef DEBUG_ENABLED
#define pdebug(fmt, args...)
#define pwrite(fmt, len)
#else
#define DEBUG_ENABLED
#define pdebug(fmt, args...) Serial.printf(fmt, ## args)
#define pwrite(fmt, len) Serial.write(fmt, len)
#endif

#include "esp8266_gpio_direct.h"

void ICACHE_RAM_ATTR SimpleSwitch::onChange(void)
{
	uint32_t key, mark;
	int btn;

	mark = millis();
	key = gpio_r->in;
	btn = (key & m_gpioPinMask)?0:1;
	if (btn == m_state) return;
	// Check to see if the change is within a debounce delay threshold.
	if ((millis() - m_lastDebounceTime) <= m_debounceDelay) {
		m_lastDebounceTime = mark;
		return;
	}
	m_lastDebounceTime = mark;
	m_state = btn;
	if (m_holdDetect) {
		if (btn) m_ticker.attach_ms(m_holdDetect, [this]() ICACHE_RAM_ATTR { onHold();} ); else m_ticker.detach();
	}
	if (m_CB) m_CB(this, m_state);
}
//===========================================================================================

void SimpleSwitch::onHold(void)
{
	uint32_t key;

	m_ticker.detach();
	if (!m_CB) return;

	key = gpio_r->in;
	if ((key & m_gpioPinMask) == 0) {
		if (m_holdDetect2) m_ticker.attach_ms(m_holdDetect2, [this]() ICACHE_RAM_ATTR { onHold2();} );
		m_CB(this, 2);
	} else {
		if (m_state) {
			m_state = 0;
			m_CB(this, m_state);
		}
	}
}
//===========================================================================================

void SimpleSwitch::onHold2(void)
{
	uint32_t key;

	m_ticker.detach();
	if (!m_CB) return;

	key = gpio_r->in;
	if ((key & m_gpioPinMask) == 0) {
		m_CB(this, 3);
	} else {
		if (m_state) {
			m_state = 0;
			m_CB(this, m_state);
		}
	}
}
//===========================================================================================
