#include "TimerSet.h"
#include <EEPROM.h>

#define _DEBUG_TIME_SET_

#define SECS_IN_DAY_UTC m_timeClient->getEpochTime()% 86400UL
#define SECS_FROM_START millis()/1000

TimerSet::TimerSet(NTPClient* timeClient){
	m_timeClient = timeClient;
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
	
	unsigned long secsInDay = SECS_IN_DAY_UTC;
        unsigned long secsFromStart = SECS_FROM_START;
        
        
	//unsigned long sinceLastCheck = secsInDay - m_lastCheck;
        //unsigned long sinceLastCheck = secsFromStart - m_lastCheck;
	
	String s = String(m_stopTimer[0]);
	
	if(m_currentState == -1)
		return -1;
        
	if((secsFromStart - m_lastCheck) > 5){ // Check every 5" since last check
            
            m_lastCheck = secsFromStart;

            if(m_currentState == 0) { // If it is OFF, find a timer to turn it ON
                    newState = 1;
                    p = m_startTimer;

                    #ifdef _DEBUG_TIME_SET_
                        Serial.println("OFF finds ON");
                    #endif

            } else { // If it is ON, find timer to turn it OFF
                    newState = 0;
                    p = m_stopTimer;

                    #ifdef _DEBUG_TIME_SET_
                        Serial.println("ON finds OFF");
                    #endif
            }	

            for(int i = 0; i < NUM_TIMERS; i++){
                    if(i == 0)                            
                            t = secsFromStart - m_lastStateChange - p[i];
                    else
                            t = secsInDay - p[i];

                    if ((t>=0 && t<5)||(t<0 && t>-5)) {
                            m_currentState = newState;                
                            m_lastStateChange = secsFromStart;

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
        restartInterval();
}

// =================================================================
// void TimerSet::update()
// =================================================================
void TimerSet::restartInterval() {
    m_lastStateChange = SECS_FROM_START;
}

// =================================================================
// int getState()
// =================================================================
int TimerSet::getState() {
    return m_currentState;
}

// =================================================================
// void setState()
// =================================================================
void TimerSet::setState(int state) {
    m_currentState = state;
}

// =================================================================
// TimerSet::setTimer(unsigned long[] startTime, unsigned long[] stopTime)
// =================================================================
void TimerSet::setTimer(unsigned long* startTime, unsigned long* stopTime) {
	
    for(int i=0; i<NUM_TIMERS; i++)
    {
        m_startTimer[i] = startTime[i];
	m_stopTimer[i] = stopTime[i];
    }
	
	
    m_lastStateChange = SECS_FROM_START;
}

/*
// =================================================================
// void setTimeZone(int timeZone)
// timeZone param is in number of seconds
// =================================================================
void TimerSet::setTimeZone(int timeZone) {
	m_timeZone = timeZone;
}
*/

// =================================================================
// String TimerSet::toString()
// Put data into a String to store in EEPROM or publishing.
// =================================================================
String TimerSet::toString(){
    String str="";
    
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
    
    return str;
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
	
	str = toString();
	
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
	
        str = "";
	// Read time zone
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
            EEPROM.write(pos+i, str[i]); // The null-terminal '\0' in string is also stored
    
            /*#ifdef _DEBUG_TIME_SET_
		Serial.print(pos+i);
		Serial.print("-W-");
		Serial.println((byte)str[i]);
            #endif*/
        }
}

void TimerSet::_loadEEPROM(char* str, int pos, int len){
  
	for (int i=0; i<len; i++){
		str[i] = (char)EEPROM.read(i+pos);
	}

	/*#ifdef _DEBUG_TIME_SET_
		Serial.print("LOAD: ");
		Serial.print(str);
		Serial.print(" AT: ");
		Serial.print(pos);
		Serial.print(" LEN: ");
		Serial.println(len);
	#endif*/
}






