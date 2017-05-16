#include "TimerSet.h"
#define DEBUG_TIME_SET

TimerSet::TimerSet(UDP& udp, const char* poolServerName){
	m_timeClient = new NTPClient (udp, poolServerName);
}

TimerSet::~TimerSet(){
	delete m_timeClient;
}

void TimerSet::begin(){
	m_timeClient->begin();
	m_timeClient->update();
	m_lastStateChange = m_timeClient->getEpochTime() % 86400;
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

// ============================================================
// Return 1: ON; 0: OFF; -1: NO EVENT
// If current state is OFF, check if it should be turned to ON
// If current state is ON,  check if it should be turned to OFF
// ============================================================

int TimerSet::getState(){
	
	int t;
	int newState;
	unsigned int *p;
	
	unsigned long secInDay = m_timeClient->getEpochTime() % 86400; 
	unsigned int sinceLastCheck = secInDay - m_lastCheck;
	
	
	if(sinceLastCheck > 5){ // Check every 5" since last check
	
		m_lastCheck = secInDay;
		
		if(m_currentState == 0) { // If it is OFF, find event to turn it ON
			newState = 1;
			p = m_startTimer;
			#ifdef DEBUG_TIME_SET
				Serial.println("OFF finds ON");
			#endif
		
		} else { // If it is ON, find event to turn it OFF
			newState = 0;
			p = m_stopTimer;
			#ifdef DEBUG_TIME_SET
				Serial.println("ON finds OFF");
			#endif
		}	
			
		for(int i = 0; i < NUM_TIMERS; i++){
			if(i == 0)
				t = secInDay - m_lastStateChange - p[i];
			else
				t = secInDay - p[i];
			
			#ifdef DEBUG_TIME_SET
				Serial.print(i);
				Serial.print(" / ");
				Serial.print(secInDay);
				Serial.print(" / ");
				Serial.print(m_lastStateChange);
				Serial.print(" / ");
				Serial.print(p[i]);
				Serial.print(" / ");
				Serial.println(t);
			#endif
			
			if (t>=0 && t<5) { // If interval reached
				m_currentState = newState;
				m_lastStateChange = secInDay;
				
				#ifdef DEBUG_TIME_SET
					Serial.print("Timer[");
					Serial.print(i);
					Serial.print("] CHANGED to: ");
					Serial.print(m_currentState);
					Serial.print(" at: ");
					Serial.println(m_timeClient->getFormattedTime());
				#endif
				break;
			}
		}
	} else {
		return -1;
	}
	
	return m_currentState;

}