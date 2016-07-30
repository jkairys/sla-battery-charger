#include "Analogue.h"

#define PIN_RELAY 12
#define PIN_CURRENT A7
#define PIN_VOLTAGE A6
#define MV_PER_BIT 32

Analogue voltage(PIN_VOLTAGE, 1.0, MV_PER_BIT);
Analogue current(PIN_CURRENT, 1.0, 1);

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_CURRENT, INPUT);
  pinMode(PIN_VOLTAGE, INPUT);
  Serial.begin(115200);

  // calibrate current
  relay(0);
  current.calibrate();
  
}

void relay(byte state){
  digitalWrite(PIN_RELAY, state);
}

unsigned long next_disp = millis();

void loop() {
  voltage.measure();
  current.measure();  
  if(millis() > next_disp){
    
    next_disp = millis() + 100;
    Serial.print("Voltage: ");
    Serial.println(((float)voltage.value()/1000.0), 1);
    
    Serial.print("Current: ");
    Serial.println(((float) current.value() /1000.0), 1);
    
  }
}
