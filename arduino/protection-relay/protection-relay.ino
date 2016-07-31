#include "Analogue.h"
#include "Protection.h"



#define PIN_RELAY 12
#define PIN_CURRENT A7
#define PIN_VOLTAGE A6
#define MV_PER_BIT 32
#define MA_PER_BIT -67


Analogue voltage(PIN_VOLTAGE, 1.0, MV_PER_BIT);
Analogue current(PIN_CURRENT, 1.0, MA_PER_BIT);

Protection prot_voltage(8000, 0.3, PIN_RELAY, "V");
Protection prot_current(1800, 0.2, PIN_RELAY, "I");




void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_CURRENT, INPUT);
  pinMode(PIN_VOLTAGE, INPUT);
  Serial.begin(115200);

  // calibrate current
  relay(0);
  current.calibrate();
  // reset protection, will place in settle mode
  prot_current.reset();
  prot_voltage.reset();
  // close contactor
  relay(1);

}

byte _relay = 0;
void relay(byte state){
  _relay = state;
  digitalWrite(PIN_RELAY, _relay);
}

unsigned long next_disp = millis();


void loop() {
  voltage.measure();
  current.measure();  
  
  prot_current.run(current.value());
  
  prot_voltage.run(voltage.value());

  /*
  if(millis() > next_disp){    
    next_disp = millis() + 100;
    Serial.println(
      String(((float)voltage.value()/1000.0), 1) + "," +
      String(((float)current.value()/1000.0), 1)
    );
  }*/
}
