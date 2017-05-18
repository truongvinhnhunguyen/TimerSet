#include "Arduino.h"
#include <NTPClient.h>

#ifndef _TIMERSET_
#define _TIMERSET_

#define NUM_TIMERS 4
#define MAX_LEN_TO_STORE 64 //Convert timer to string => 6 characters each timer

class TimerSet{

NTPClient *m_timeClient;

unsigned long m_lastCheck = 0;

int m_currentState = -1; // 1: ON; 0: OFF; -1: INACTIVATED
unsigned long m_lastStateChange = 0;

// First item is used as Interval
// From 2nd item moving forward, value is stored as "Second in the day", 
// Ex, 15:20 = 15*60*60+20*60 = 55,200
unsigned long m_startTimer[NUM_TIMERS] = {86401, (10*60+28)*60, 86402, (10*60+30)*60}; 
unsigned long m_stopTimer[NUM_TIMERS] = {86404, (10*60+29)*60, 86405, (10*60+31)*60};

unsigned long m_timeZone = 7*60*60; // Number of seconds

public:
	TimerSet(UDP& udp, const char* poolServerName);
	~TimerSet();
	
	void setTimeZone(int timeZone);
	
	void begin();
	void end();
	
	
	void setTimer(int idx, unsigned long startTime, unsigned long stopTime);
	
	int checkState();
	void forceState(int state);
	
	int writeEEPROM(int pos);
	void loadEEPROM(int pos);
	
	
	void printTime();
	
	
	static void _storeEEPROM(char* str, int pos, int len);
	static void _loadEEPROM(char* str, int pos, int len);
};

#endif
