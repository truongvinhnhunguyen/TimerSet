#include "TimerSet.h"

TimerSet::TimerSet(UDP& udp, const char* poolServerName){
	m_timeClient = new NTPClient (udp, poolServerName);
}

TimerSet::~TimerSet(){
	delete m_timeClient;
}

void TimerSet::begin(){
	m_timeClient->begin();
	m_timeClient->update();
}

void TimerSet::end(){
	m_timeClient->end();
}

void TimerSet::printTime(){
	Serial.println("TimerSet");
	Serial.println(m_timeClient->getEpochTime());
	Serial.println(m_timeClient->getFormattedTime()); 
	Serial.println(m_timeClient->getDay());
	Serial.println(m_timeClient->getHours());
	Serial.println(m_timeClient->getMinutes());
	Serial.println(m_timeClient->getSeconds());
}