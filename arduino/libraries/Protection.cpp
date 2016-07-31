#include "Protection.h"

Protection::Protection(){

}

Protection::Protection(int limit, float T, byte pin, char * name) {
  _pin = pin;
  _limit = limit;
  _T = T;
  // set our name
  _name = String(name);
  state = STATE_INIT;

  pinMode(_pin, OUTPUT);
  // IEC standard inverse curve
  k = 0.14;
  alpha = 0.02;
  beta = 2.97;
}

void Protection::trip(){
  digitalWrite(_pin, 0);
}

void Protection::run(int sample){
  if(state == STATE_INIT){
    if (millis() > t_settled){
      state = STATE_ARMED;
    }
  }

  if(state == STATE_ARMED){
    if(sample > _limit){
      t_pickup = millis();
      t_trip = iec_curve(sample);
      t_trip = t_trip + t_pickup;
      //Serial.println(String(millis(), DEC)+"ms "+_name + ": pickup@"+String(sample,DEC));
      Serial.println("EP"+_name+String(sample, DEC));
      state = STATE_PICKUP;
    }
  }

  if(state == STATE_PICKUP){
    if(sample < _limit){
      state = STATE_ARMED;
      return;
    }
    t_trip = iec_curve(sample);
    //Serial.println("t_trip = " + String(t_trip, DEC) + "ms");
    if(millis() > t_trip + t_pickup){ 
      //Serial.println(sample, DEC);
      //Serial.println(String(millis(), DEC)+ " > " +String(t_pickup + t_trip, DEC));
      state = STATE_TRIPPED;
      Serial.println("ET"+_name+String(sample, DEC));
      //Serial.println(String(millis(), DEC)+"ms "+_name + ": trip@"+String(sample,DEC));
      trip();
    }
  }

}

int Protection::limit(){
  return _limit;
}
void Protection::limit(int limit){
  _limit = limit;
}

void Protection::reset(){
  state = STATE_INIT;
  t_settled = millis() + MS_TO_SETTLE;
}

unsigned long Protection::iec_curve(long sample){
  float iis = (float)sample / (float)_limit;
  if(iis == 1) return 100000;
  unsigned long tmp = (unsigned long) (1000* (k / (pow(iis,alpha) - 1)) * _T / beta);
  return tmp < 100000 ? tmp : 100000; 
}