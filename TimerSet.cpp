#include "TimerSet.h"
#include <EEPROM.h>

#define _DEBUG_TIME_SET_

#define CUR_TIME_W_TZ (m_timeZone + (m_timeClient->getEpochTime() % 86400L))

TimerSet::TimerSet(NTPClient* timeClient){
	m_timeClient = timeClient;
}

TimerSet::~TimerSet(){
	delete m_timeClient;
}

void TimerSet::begin(){
	
	m_timeClient->begin();
	m_timeClient->update();
	
	m_lastStateChange = CUR_TIME_W_TZ;
}

void TimerSet::end(){
	m_timeClient->end();
	m_currentState = -1;
}

void TimerSet::printTime(){
#ifdef _DEBUG_TIME_SET_
	Serial.println("TimerSet");
	Serial.println(m_timeClient->getEpochTime());
	Serial.println(m_timeClient->getFormattedTime()); 
	Serial.println(m_timeClient->getDay());
	Serial.println(m_timeClient->getHours());
	Serial.println(m_timeClient->getMinutes());
	Serial.println(m_timeClient->getSeconds());
#endif
}

// =================================================================
// int TimerSet::checkState()
// + Return new state 1: START; 0: STOP; -1: INACTIVED; -2: NO EVENT
// + If current state is STOP, check if it should be turned to START
// + If current state is START,  check if it should be turned to STOP
// =================================================================
int TimerSet::loop(){
	int t;
	int newState;
	unsigned long *p;
	
	unsigned long secInDay = CUR_TIME_W_TZ; 
	unsigned long sinceLastCheck = secInDay - m_lastCheck;
	
	String s = String(m_stopTimer[0]);
	
	if(m_currentState == -1)
		return -1;
	
	
	if(sinceLastCheck > 5){ // Check every 5" since last check
	
		m_lastCheck = secInDay;
		
		if(m_currentState == 0) { // If it is OFF, find event to turn it ON
			newState = 1;
			p = m_startTimer;
			
			#ifdef _DEBUG_TIME_SET_
				Serial.println("OFF finds ON");
			#endif
		
		} else { // If it is ON, find event to turn it OFF
			newState = 0;
			p = m_stopTimer;
			
			#ifdef _DEBUG_TIME_SET_
				Serial.println("ON finds OFF");
			#endif
		}	
			
		for(int i = 0; i < NUM_TIMERS; i++){
			if(i == 0)
				t = secInDay - m_lastStateChange - p[i];
			else
				t = secInDay - p[i];
			
			#ifdef _DEBUG_TIME_SET_LV2_
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
			
			if (t>=0 && t<5) {
				m_currentState = newState;
				m_lastStateChange = secInDay;
				
				#ifdef _DEBUG_TIME_SET_
					Serial.print("Timer[");
					Serial.print(i);
					Serial.print("] CHANGED to: ");
					Serial.print(m_currentState);
					Serial.print(" at: ");
					Serial.println(m_timeClient->getFormattedTime());
				#endif
				
				return m_currentState; 
				
			}
		}
	} else {
		return -2;
	}
	
	return -2;

}
// =================================================================
// void forceState(int state)
// 1: START; 0: STOP; -1: INACTIVATED
// =================================================================
void TimerSet::forceState(int state) {
	m_currentState = state;
	m_lastStateChange = CUR_TIME_W_TZ;
}

// =================================================================
// int getState()
// =================================================================
int TimerSet::getState() {
    return m_currentState;
}

// =================================================================
// setTimer(int idx, int startTime, int stopTime)
// Idx = 0 -> set interval
// startTime/stopTime is seconds in day
// =================================================================
void TimerSet::setTimer(int idx, unsigned long startTime, unsigned long stopTime) {
	
	m_startTimer[idx] = startTime;
	m_startTimer[idx] = stopTime;
	
	if(idx == 0)
		m_lastStateChange = CUR_TIME_W_TZ;
}

// =================================================================
// void setTimeZone(int timeZone)
// timeZone param is in number of seconds
// =================================================================
void TimerSet::setTimeZone(int timeZone) {
	m_timeZone = timeZone;
}

// =================================================================
// int TimerSet::writeEEPROM(int pos)
// Return number of bytes include '\0' written to EEPROM
// =================================================================
int TimerSet::writeEEPROM(int pos) {
	String str="";
	char buf[MAX_LEN_TO_STORE];
	
	EEPROM.begin(1024);
	
	for (int i=0; i<MAX_LEN_TO_STORE; i++)
		buf[i] = '\0';
	
	// Adding Start Timer
	for (int i=0; i<NUM_TIMERS; i++) {
		str += m_startTimer[i];
		str += "#";
	}
	
	// Adding Stop Timer
	for (int i=0; i<NUM_TIMERS; i++) {
		str += m_stopTimer[i];
		str += "#";
	}
	
	// Adding time zone
	str += m_timeZone;
	str += "#";
	
	int len = str.length();
	
	str.toCharArray (buf, MAX_LEN_TO_STORE);
	_storeEEPROM(buf, pos, len);
	
	EEPROM.end();
	
	return len;
}

// =================================================================
// int TimerSet::loadEEPROM(int pos)
// =================================================================
void TimerSet::loadEEPROM(int pos) {
	char buf[MAX_LEN_TO_STORE];
	
	unsigned long *p = m_startTimer;
	
	int i = 0;
	int j = 0;
	int c = 0;
	String str = "";
	
	EEPROM.begin(1024);
	_loadEEPROM(buf, pos, MAX_LEN_TO_STORE);
	
	// Read Timers
	while (c < NUM_TIMERS*2){
		if(buf[i] != '#'){
			str += buf[i];
		}else {
			p[j] = (unsigned long) str.toInt();
			
			str = "";
			
			#ifdef _DEBUG_TIME_SET_ 
				Serial.print("Loaded[");
				Serial.print(j);
				Serial.print("] = ");
				Serial.println(p[j]);
			#endif
			
			c++;
			j++;
			
			if(c == NUM_TIMERS) {
				p = m_stopTimer;
				j = 0;
			}
		}
		
		i++;
	}
	
	// Read rime zone
	while (buf[i] != '#'){
		str += buf[i];
		i++;
	}
	
	m_timeZone = (unsigned long) str.toInt();
	
	#ifdef _DEBUG_TIME_SET_ 
		Serial.print("TimeZone Loaded: ");
		Serial.println(m_timeZone);
	#endif
}

// =================================================================
// void TimerSet::_storeEEPROM(char* str, int pos, int len)
// A static function utility to write a str including '\0' to EEPROM
// =================================================================
void TimerSet::_storeEEPROM(char* str, int pos, int len){

	#ifdef _DEBUG_TIME_SET_LV2_
		Serial.print("STORE: ");
		Serial.print(str);
		Serial.print(" AT: ");
		Serial.print(pos);
		Serial.print(" LEN: ");
		Serial.println(len);
	#endif

	for (int i=0; i<=len; i++){
		EEPROM.write(pos+i, str[i]); // The null-terminal '\0' in string is also strored
    
    #ifdef _DEBUG_TIME_SET_
		Serial.print(pos+i);
		Serial.print("-W-");
		Serial.println((byte)str[i]);
    #endif
  }
}

void TimerSet::_loadEEPROM(char* str, int pos, int len){
  
	for (int i=0; i<len; i++){
		str[i] = (char)EEPROM.read(i+pos);
	}

	#ifdef _DEBUG_TIME_SET_
		Serial.print("LOAD: ");
		Serial.print(str);
		Serial.print(" AT: ");
		Serial.print(pos);
		Serial.print(" LEN: ");
		Serial.println(len);
	#endif
}






