/*
 * Two axis (2D) motion class for ESP8266.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
extern "C" {
	#include <osapi.h>
	#include <os_type.h>
}
#include "Motion2D.h"
#include "AccelStepper.h" // nice lib from http://www.airspayce.com/mikem/arduino/AccelStepper/
#include "Ramp.h"
#include <Servo.h>

#if defined(OPEN_CLOSE_TIME)
ramp              myRamp;
static os_timer_t rampTimer;
#endif
Servo             penservo;

#ifndef USE_ACCEL_STEPPER

#include "esp8266_gpio_direct.h"
#include "core_esp8266_waveform.h"

#define PULSE_TIME (microsecondsToClockCycles(100))
#define MIN_PERIOD (microsecondsToClockCycles(200))

static volatile int        int_active   = 0;
static volatile int        in_motion    = 0;
static volatile uint32_t   last_time    = 0;

/* X */
static uint16_t            x_gpio_mask  = 0;
static volatile uint32_t   x_period     = 0;
static volatile int        x_target     = 0;
static volatile uint32_t   x_time       = 0;
static volatile int        x_pos        = 0;
static volatile int        x_pulse      = 0;

/* Y */
static uint16_t            y_gpio_mask  = 0;
static volatile uint32_t   y_period     = 0;
static volatile int        y_target     = 0;
static volatile uint32_t   y_time       = 0;
static volatile int        y_pos        = 0;
static volatile int        y_pulse      = 0;


static uint32_t motion_intr_handler(void);

//#pragma GCC optimize ("Os")

static inline ICACHE_RAM_ATTR uint32_t GetCycleCount()
{
	uint32_t ccount;
	__asm__ __volatile__("esync; rsr %0,ccount":"=a"(ccount));
	return ccount;
}
//===========================================================================================

void Motion2D::printStat(CommandQueueItem *c)
{
	c->print("now="+String(GetCycleCount()) + "\r\nint_active="+String(int_active)+"\r\nin_motion="+String(in_motion)+"\r\n" + \
		"x_time="+String(x_time)+ "\r\ny_time="+String(y_time)+"\r\n" \
		"x_pulse="+String(x_pulse)+ "\r\ny_pulse="+String(y_pulse)+"\r\n" \
		"x_pos="+String(x_pos)+",target = "+String(x_target)+"\r\n" + \
		"y_pos="+String(y_pos)+",target = "+String(y_target)+"\r\nOK\r\n");
}
//===========================================================================================

#else
void Motion2D::printStat(CommandQueueItem *c)
{
}
//===========================================================================================
#endif


#if defined(OPEN_CLOSE_TIME)

/*!
 * \brief RAMP tick - update servo position from ramp generator.
 */
static void rampTick(void* arg)
{
	unsigned int pos = myRamp.update();
	if (myRamp.isRunning()) {
		penservo.write(pos);
	} else {
		os_timer_disarm(&rampTimer);
	}
}
//====================================================================================
#endif


/*!
 * \breif Constructor.
 */
Motion2D::Motion2D(int step1, int dir1, int step2, int dir2, int en_pin, int servoPin)
{
#ifdef USE_ACCEL_STEPPER
	m_xMotor = new AccelStepper(1, step1, dir1);
	m_yMotor = new AccelStepper(1, step2, dir2);
	m_xMotor->setMaxSpeed(2000.0);
	m_xMotor->setAcceleration(10000.0);
	m_yMotor->setMaxSpeed(2000.0);
	m_yMotor->setAcceleration(10000.0);
#else
	pinMode(step1, OUTPUT);
	pinMode(step2, OUTPUT);
	pinMode(dir1, OUTPUT);
	pinMode(dir2, OUTPUT);
	digitalWrite(step1, LOW);
	digitalWrite(step2, LOW);
	m_x_dir         = dir1;
	m_y_dir         = dir2;
	x_gpio_mask     = (1 << step1);
	y_gpio_mask     = (1 << step2);
	/* Disable timer */
	setTimer1Callback(NULL);
	int_active      = 0;
	in_motion       = 0;
#endif
	m_en_pin        = en_pin;
	m_motorsEnabled = 0;
	pinMode(en_pin, OUTPUT);
	motorsOff();
#if defined(OPEN_CLOSE_TIME)
	/* Start ramp timer */
	os_timer_disarm(&rampTimer);
#endif
	pinMode(servoPin,OUTPUT);
	penservo.attach(servoPin);
	setPenUp();
#ifdef MOTION_QUEUE_SIZE
	m_motionQWr   = 0;
	m_motionQRd   = 0;
#endif
}
//====================================================================================

/*!
 * \brief Disable Motors.
 */
void Motion2D::motorsOff()
{
	digitalWrite(m_en_pin, HIGH);
	m_motorsEnabled = 0;
}
//====================================================================================

/*!
 * \brief Enable Motors.
 */
void Motion2D::motorsOn()
{
	digitalWrite(m_en_pin, LOW) ;
	m_motorsEnabled = 1;
}
//====================================================================================

/*!
 * \brief Toggle Motors.
 */
void Motion2D::toggleMotors()
{
	if (m_motorsEnabled) {
		motorsOff();
	} else {
		motorsOn();
	}
}
//====================================================================================

void Motion2D::setPenUpReal(int d)
{
#if defined(OPEN_CLOSE_TIME)
	os_timer_disarm(&rampTimer);
	myRamp.go(m_penUpPos, OPEN_CLOSE_TIME, SINUSOIDAL_INOUT, ONCEFORWARD);
	os_timer_setfn(&rampTimer, &rampTick, nullptr);
	os_timer_arm(&rampTimer, RAMP_TIMER_INTERVAL, true);
#else
	penservo.write(m_penUpPos);
#endif
	m_penState = m_penUpPos;
	if (d) delay(d);
}
//====================================================================================

void Motion2D::setPenDownReal(int d)
{
#if defined(OPEN_CLOSE_TIME)
	os_timer_disarm(&rampTimer);
	myRamp.go(m_penDownPos, OPEN_CLOSE_TIME, SINUSOIDAL_INOUT, ONCEFORWARD);
	os_timer_setfn(&rampTimer, &rampTick, nullptr);
	os_timer_arm(&rampTimer, RAMP_TIMER_INTERVAL, true);
#else
	penservo.write(m_penDownPos);
#endif
	m_penState = m_penDownPos;
	if (d) delay(d);
}
//====================================================================================

void Motion2D::togglePenReal(int d)
{
	if (m_penState == m_penUpPos) {
		setPenDown(d);
	} else {
		setPenUp(d);
	}
}
//====================================================================================


/*!
 * \brief Prepare and start move.
 */
void Motion2D::goToReal(uint16_t duration, int xSteps, int ySteps)
{
#ifdef USE_ACCEL_STEPPER
	if (!m_motorsEnabled) {motorsOn();}
	//set Coordinates and Speed
	m_xMotor->move(xSteps);
	if (xSteps < 0) xSteps = -xSteps;
	m_xMotor->setSpeed( (xSteps * 1000) / duration );
	m_yMotor->move(ySteps);
	if (ySteps < 0) ySteps = -ySteps;
	m_yMotor->setSpeed( (ySteps * 1000) / duration );
#else
	int tmp;
	//delay(duration);
	//return;
	if (in_motion) {Serial.print("ERROR\n"); return; }
	if (!m_motorsEnabled) {motorsOn();}
	setTimer1Callback(NULL);
	int_active      = 0;
	in_motion       = 0;
	x_pulse         = 0;
	y_pulse         = 0;
	if (duration == 0) duration = 100;
	/* Set target */
	x_target += xSteps;
	y_target += ySteps;
	/* Set direction pin */
	if (x_target > x_pos) digitalWrite(m_x_dir, HIGH); else digitalWrite(m_x_dir, LOW);
	if (y_target > y_pos) digitalWrite(m_y_dir, HIGH); else digitalWrite(m_y_dir, LOW);
	/* ABS */
	if (xSteps < 0) xSteps = -xSteps;
	if (ySteps < 0) ySteps = -ySteps;
	/* Set period */
	if (xSteps) {
		tmp = (duration * 1000)/xSteps;
		x_period = microsecondsToClockCycles(tmp);
	} else {
		x_period = 10000;
	}
	if (ySteps) {
		tmp = (duration * 1000)/ySteps;
		y_period = microsecondsToClockCycles(tmp);
	} else {
		y_period = 10000;
	}
	if (x_period < MIN_PERIOD) x_period = MIN_PERIOD;
	if (y_period < MIN_PERIOD) y_period = MIN_PERIOD;
	/* Start timer1 */
	in_motion  = 1;
	int_active = 1;
	x_time = last_time + microsecondsToClockCycles(3300);
	y_time = (GetCycleCount() + microsecondsToClockCycles(500));
	if (x_time > y_time) y_time = x_time; else x_time = y_time;
	setTimer1Callback(motion_intr_handler);
#endif
}
//====================================================================================

/*!
 * \brief Executed in main loop.
 */
boolean Motion2D::loop()
{
#ifdef USE_ACCEL_STEPPER
#ifdef MOTION_QUEUE_SIZE
	if ( m_yMotor->distanceToGo() || m_xMotor->distanceToGo() ) {
		m_yMotor->runSpeedToPosition();
		m_xMotor->runSpeedToPosition();
	} else {
		motionQ_pull();
	}
	return motionQ_is_full();
#else
	while ( m_yMotor->distanceToGo() || m_xMotor->distanceToGo() ) {
		m_yMotor->runSpeedToPosition();
		m_xMotor->runSpeedToPosition();
		yield();
	}
	return 0;
#endif
#else
#ifdef MOTION_QUEUE_SIZE
	if (in_motion) {
		if (int_active == 0) {
			setTimer1Callback(NULL);
			in_motion = 0;
			motionQ_pull();
		}
	} else {
		motionQ_pull();
	}
	return motionQ_is_full();
#else
	if (in_motion) {
		if (int_active == 0) {
			setTimer1Callback(NULL);
			in_motion = 0;
		}
	}
	return in_motion;
#endif
#endif
}
//====================================================================================


#ifndef USE_ACCEL_STEPPER

// Speed critical bits
#pragma GCC optimize ("O2")
// Normally would not want two copies like this, but due to different
// optimization levels the inline attribute gets lost if we try the
// other version.

static inline ICACHE_RAM_ATTR uint32_t GetCycleCountIRQ()
{
	uint32_t ccount;
	__asm__ __volatile__("rsr %0,ccount":"=a"(ccount));
	return ccount;
}
//===========================================================================================

static uint32_t ICACHE_RAM_ATTR motion_intr_handler(void)
{
	if (int_active == 0) return 10000;
	uint32_t d0 = 0, d1, now = GetCycleCountIRQ();
	int32_t expiryToGo;

	/* Process ROT */
	expiryToGo = (x_time - now);
	if (expiryToGo < 0) {
		if (x_pulse) {
			x_pulse = 0;
			asm volatile ("" : : : "memory");
			gpio_r->out_w1tc = (uint32_t)(x_gpio_mask);
			x_pulse = 0;
			if (x_pos == x_target) x_time = 0; else x_time += x_period;
		} else if (x_pos != x_target) {
			asm volatile ("" : : : "memory");
			gpio_r->out_w1ts = (uint32_t)(x_gpio_mask);
			x_time = now + PULSE_TIME;
			x_pulse = 1;
			if (x_pos > x_target) x_pos--; else x_pos++;
		} else {
			x_time = 0;
		}
	}

	/* Process PEN */
	expiryToGo = (y_time - now);
	if (expiryToGo < 0) {
		if (y_pulse) {
			y_pulse = 0;
			asm volatile ("" : : : "memory");
			gpio_r->out_w1tc = (uint32_t)(y_gpio_mask);
			y_pulse = 0;
			if (y_pos == y_target) y_time = 0; else y_time += y_period;
		} else if (y_pos != y_target) {
			asm volatile ("" : : : "memory");
			gpio_r->out_w1ts = (uint32_t)(y_gpio_mask);
			y_time = now + PULSE_TIME;
			y_pulse = 1;
			if (y_pos > y_target) y_pos--; else y_pos++;
		} else {
			y_time = 0;
		}
	}

	/* calculate next event time */
	if ((!y_time) && (!x_time)) {
		/* We are done :-) */
		int_active = 0;
		last_time  = now;
		return 10000;
	} else if ((y_time) && (!x_time)) {
		d0 = y_time - now;
	} else if ((!y_time) && (x_time)) {
		d0 = x_time - now;
	} else {
		d0 = x_time - now;
		d1 = y_time - now;
		if (d1 < d0) d0 = d1;
	}
	return d0;
}
//===========================================================================================

#endif
