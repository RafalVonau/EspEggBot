/*
 * Two axis (2D) motion class for ESP8266.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef __MOTION_2D__
#define __MOTION_2D__


#include "Arduino.h"
#include <stdint.h>
#include <c_types.h>
#include <eagle_soc.h>
#include <ets_sys.h>
#include "osapi.h"
#include "Command.h"

/*! Use AccelStepper lib or simple but fast (no ramp implementation) timer1 based solution */
//#define USE_ACCEL_STEPPER

#define OPEN_CLOSE_TIME     (200)           /*!< Servo open/close time (comment out to disable ramp)       */
#define RAMP_TIMER_INTERVAL (20)            /*!< Servo ramp update interval                                */



#ifdef USE_ACCEL_STEPPER
#include "AccelStepper.h" // nice lib from http://www.airspayce.com/mikem/arduino/AccelStepper/
#endif



#define MOTION_QUEUE_SIZE (32)

#ifdef MOTION_QUEUE_SIZE
#define MOTION_QUEUE_MASK (MOTION_QUEUE_SIZE-1)
typedef struct motion_queue_s {
	uint16_t cmd;
	uint16_t duration;
	int x;
	int y;
} motion_queue_t;
#endif

class Motion2D
{
public:
	Motion2D(int step1, int dir1, int step2, int dir2, int en_pin, int servoPin);

	void motorsOff();
	void motorsOn();
	void toggleMotors();
	
	boolean loop();

	void setPenUpReal(int d = 0);
	void setPenDownReal(int d = 0);
	void togglePenReal(int d = 0);
	void goToReal(uint16_t duration, int xSteps, int ySteps);


#ifdef MOTION_QUEUE_SIZE
	void setPenUp(int d = 0)   {motionQ_push(2, d, 0, 0);}
	void setPenDown(int d = 0) {motionQ_push(3, d, 0, 0);}
	void togglePen(int d = 0)  {motionQ_push(4, d, 0, 0);}
	void goTo(uint16_t duration, int xSteps, int ySteps) {motionQ_push(1, duration, xSteps, ySteps);}

	void motionQ_push(uint16_t cmd, uint16_t duration, int x, int y) {
		int pos = m_motionQWr;
		motion_queue_t *v = &m_motionQ[pos];
		v->cmd = cmd;
		v->duration = duration;
		v->x = x;
		v->y = y;
		pos++;
		pos &= MOTION_QUEUE_MASK;
		m_motionQWr = pos;
	}
	
	uint32_t motionQ_is_full() {
		int pos = m_motionQWr;
		pos++;
		pos &= MOTION_QUEUE_MASK;
		if (pos == m_motionQRd) return 1;
		return 0;
	}
	
	void motionQ_pull() {
		if (m_motionQWr != m_motionQRd) {
			int pos = m_motionQRd;
			motion_queue_t *v = &m_motionQ[pos];
			switch (v->cmd) {
				case 1: goToReal(v->duration, v->x, v->y); break;
				case 2: setPenUpReal(v->duration); break;
				case 3: setPenDownReal(v->duration); break;
				case 4: togglePenReal(v->duration); break;
				default: break;
			}
			pos++;
			pos &= MOTION_QUEUE_MASK;
			m_motionQRd = pos;
		}
	}
#else
	void setPenUp(int d = 0) {setPenUpReal(d);}
	void setPenDown(int d = 0) {setPenDownReal(d);}
	void togglePen(int d = 0) {togglePenReal(d);}
	void goTo(uint16_t duration, int xSteps, int ySteps) {goToReal(duration, xSteps, ySteps);}
#endif
	void printStat(CommandQueueItem *c);
public:
#ifdef USE_ACCEL_STEPPER
	AccelStepper *m_xMotor;
	AccelStepper *m_yMotor;
#else
	int           m_x_dir;
	int           m_y_dir;
#endif
	boolean       m_motorsEnabled;
	int           m_en_pin;
	/* Servo */
	int           m_penState;
	int           m_penUpPos;
	int           m_penDownPos;
	int           m_servoRateDown;
	int           m_servoRateUp;
#ifdef MOTION_QUEUE_SIZE
	motion_queue_t m_motionQ[MOTION_QUEUE_SIZE];
	int            m_motionQWr;
	int            m_motionQRd;
#endif
};

#endif