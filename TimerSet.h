#include "Arduino.h"
#include <NTPClient.h>

#ifndef _TIMERSET_
#define _TIMERSET_

#define NUM_TIMERS 4

class TimerSet{

NTPClient *m_timeClient;
int m_timeZone = 7;

unsigned int m_lastCheck = 0;

int m_currentState = 0; // 1: ON; 0: OFF;
unsigned int m_lastStateChange = 0;

// First item is used as Interval
// From 2nd item moving forward, value is stored as "Second in the day", 
// Ex, 15:20 = 15*60*60+20*60 = 55,200
unsigned int m_startTimer[NUM_TIMERS] = {86401, (11*60+17)*60, 86401, 86401}; 
unsigned int m_stopTimer[NUM_TIMERS] = {86401, (11*60+18)*60, 86401, 86401};

public:
	TimerSet(UDP& udp, const char* poolServerName);
	~TimerSet();
	
	void setTimeZone(int timeZone);
	
	void begin();
	void end();
	
	void setTurnOnInterval(int interval);
	void setTurnOffInterval(int interval);
	
	void setTimer(int idx, int onTimer, int duration);
	
	int getState();
	int setState(int state);
	
	
	void printTime();
};

#endif
