#include "Analogue.h"
#include "Protection.h"
#include <EEPROM.h>



#define PIN_RELAY 12
#define PIN_CURRENT A7
#define PIN_VOLTAGE A6
#define MV_PER_BIT 32
#define MA_PER_BIT -67


Analogue voltage(PIN_VOLTAGE, 1.0, MV_PER_BIT);
Analogue current(PIN_CURRENT, 1.0, MA_PER_BIT);

Protection prot_voltage; //(8000, 0.3, PIN_RELAY, "V");
Protection prot_current; //(1800, 0.2, PIN_RELAY, "I");


struct SETTINGS {
  int v_limit;
  int i_limit;
};

SETTINGS settings;

void save_settings(){
  EEPROM.put(0, settings);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_CURRENT, INPUT);
  pinMode(PIN_VOLTAGE, INPUT);
  Serial.begin(9600);

  // calibrate current
  relay(0);
  current.calibrate();

  // load settings for protetion
  EEPROM.get(0, settings);

  if(settings.v_limit < 1000 || settings.v_limit > 30000){
    settings.v_limit = 12000;
    save_settings();
  }

  if(settings.i_limit < 100 || settings.i_limit > 30000){
    settings.i_limit = 1000;
    save_settings();
  }


  // apply settings
  prot_voltage = Protection(settings.v_limit, 0.3, PIN_RELAY, "V");
  prot_current = Protection(settings.i_limit, 0.2, PIN_RELAY, "I");

  
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

#define CMD_BUF_LEN 32
char cmdBuf[CMD_BUF_LEN];
byte cmdPtr = 0;


void print_vlimit(){ print_data("V", prot_voltage.limit()); }
void print_ilimit(){ print_data("I", prot_current.limit()); }
void print_relaystate(){ print_data("S", digitalRead(PIN_RELAY) ? 1 : 0);}

void print_data(String attribute, String value){
  Serial.println("D"+attribute+value);
}

void print_data(String attribute, int value){
  Serial.println("D"+attribute+String(value, DEC));
}

void process_command(String cmd){
  // first character must be 'I' to denote instruction
  if(cmd[0] != 'I') return;
  // second character contains command
  int payload = 0;
  if(cmd.length() > 2) payload = cmd.substring(2).toInt();
  
  switch(cmd[1]){
    case 'A':
      //Serial.println("Analogues requested");
      print_data(
        "A",
        String(((float)voltage.value()/1000.0), 1) + "," +String(((float)current.value()/1000.0), 1)
      );
      break;
    case 'i':
      // new setpoint
      //Serial.println("New I_limit: " + String(payload, DEC));
      if(!payload) return;
      settings.i_limit = payload;
      save_settings();
      prot_current.limit(settings.i_limit);
      print_ilimit();
      break;
    case 'v':
      // new setpoint
      //Serial.println("New V_limit: " + String(payload, DEC));
      if(!payload) return;
      settings.v_limit = payload;
      save_settings();
      prot_voltage.limit(settings.v_limit);

      print_vlimit();
      break;
    case 'V': 
      //Serial.println("DV"+String(prot_voltage.limit(), DEC));
      print_vlimit();
      break;
    case 'I': 
      //Serial.println("DI"+String(prot_current.limit(), DEC));
      print_ilimit();
      break;
    case 'S':
      print_relaystate();
      //Serial.println("DS"+String(_relay,DEC));
      break;
    case 'T':
      //Serial.println("Trip requested");
      relay(0);
      print_relaystate();
      //Serial.println("DS0");
      break;
    case 'R': 
      relay(1);
      print_relaystate();
      //Serial.println("DS1");
      prot_voltage.reset();
      prot_current.reset();
      break;
    default:
      break;
  }
}

void process_serial(){
  
  while(Serial.available()){
    cmdBuf[cmdPtr] = (char)Serial.read();
    cmdPtr++;
  
    if(cmdBuf[cmdPtr-1] == '\n'){
      cmdBuf[cmdPtr-1] = '\0';
      process_command(cmdBuf);
      cmdPtr = 0;
    }

    if(cmdPtr >= CMD_BUF_LEN){
      // overflow
      Serial.println("OVF");
      cmdPtr = 0;
    }
    
  }
  
}

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
  process_serial();
}
