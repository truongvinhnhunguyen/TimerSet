#include "Arduino.h"
#include <NTPClient.h>

#ifndef _TIMERSET_
#define _TIMERSET_


class TimerSet{

NTPClient *m_timeClient;
int m_timeZone = 7;

	
public:
	TimerSet(UDP& udp, const char* poolServerName);
	~TimerSet();
	
	void setTimeZone(int timeZone);
	
	void begin();
	void end();
	void printTime();
};

#endif
