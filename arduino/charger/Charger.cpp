#include "Charger.h"
#include <LiquidCrystal_I2C.h>
#include "PWM16.h"

LiquidCrystal_I2C lcd(0x27,20,2);

Charger::Charger() {
  a_volts_out = Analogue(PIN_VOLTS_OUT, FILTER_FREQ, MV10_PER_BIT);
  a_amps_out = Analogue(PIN_AMPS_OUT, FILTER_FREQ, -1*MA10_PER_BIT);
  a_amps_in = Analogue(PIN_AMPS_IN, FILTER_FREQ, MA10_PER_BIT);
}

void Charger::init(){
  Serial.begin(9600);
  // set the LCD address to 0x27 for a 16 chars and 2 line display
  // Display
  Serial.println("Init PWM");
  pwm = PWM16(PIN_DUTY);
  pwm.analogWrite(0);
  
  Serial.println("Init LCD");
  // initialize the lcd 
  
  lcd.init();                      
  lcd.backlight();

  // pin 12 is our 'calibrate' pin
  // if low on boot, perform calibration
  Serial.println("Init ADC");

  a_volts_out.offset(0);
  a_amps_out.calibrate();
  a_amps_in.calibrate();
  
  //analogues.offsets.load();
  pinMode(PIN_CALIBRATE, INPUT);
  lcd.clear();
  if(!digitalRead(PIN_CALIBRATE)){
    // perform calibration
    lcd.print("Calibrating");
    //analogues.calibrate();  
  }else{
    //analogues.offsets.load();
  }
  //delay(500);
  //lcd.clear();
  //lcd.print("In: ");
  //lcd.print(analogues.offsets.offset_amps_in, DEC);
  //lcd.setCursor(0,1);
  //lcd.print("Out: ");
  //lcd.println(analogues.offsets.offset_amps_out, DEC);
  //delay(500);

  //flt_mv_out_stats.setWindowSecs( 20.0/100.0 );

  // go to constant current mode
  //setMode(CURRENT);
  setMode(CURRENT);
  stage = BULK;
  
  ma_target = I_BULK * 1000;
  next_disp = millis() + 2000;
}


void Charger::disp(){
  
  lcd.setCursor(0,0);
  lcd.print("In: ");
  lcd.print((float)a_amps_in.value()/1000.0,1);
  lcd.print("A      ");
  lcd.setCursor(0,1);
  lcd.print("Out: ");
  lcd.print((float)a_amps_out.value()/1000.0,1);
  lcd.print("A ");
  lcd.print((float)a_volts_out.value()/1000.0,1);
  lcd.print("V ");
}

void Charger::setMode(CONTROL_MODE newMode){
  control_mode = newMode;
  if(control_mode == CURRENT){
    kP = 10;
    kI = 1;
  }else if(control_mode == VOLTAGE){
    kP = 5;
    kI = 1;
  }
  integral = 0;
}

void Charger::pidController(){
  

  if(control_mode == CURRENT){
    // basic P controller
    error = ma_target - a_amps_out.value();
    
    // scale it back into something sensibe
    error = error/(MA10_PER_BIT/10);
    
  }else if(control_mode == VOLTAGE){
    // basic P controller
    error = mv_target - a_volts_out.value();
    
    // scale it back into something sensibe
    error = error/(MV10_PER_BIT/10);

    //controlVolts();  
  }

  // integrate
  integral += error;
  integral = integral > 65000 ? 65000 : integral;
  integral = integral < 0 ? 0 : integral;
  
  // apply gains
  duty = kP * error + kI * integral;

  // scale everything back
  //duty = duty;
  
  if(duty > 65535) duty = 65535;
  if(duty < 0) duty = 0;

  //pwm.analogWrite(0);
  pwm.analogWrite(duty);
}


unsigned long next_assess = millis();
unsigned long last_mode_change = 0;
float amps_out_long_term_avg = NULL;
float amps_out_medium_term_avg = NULL;



void Charger::stageStr(char * buf){
  if(stage == BULK) strcpy(buf, "BULK");
  if(stage == ABSORB) strcpy(buf, "ABSORB");
  if(stage == FLOAT) strcpy(buf, "FLOAT");
}

byte mute_ttl = 0;

void Charger::go(){
  //for(uint16_t i = 0; i< 10; i++){
  a_volts_out.measure(200);
  //a_amps_in.measure(10);
  a_amps_out.measure(500);    
  //}
  
  //analogues.read();
  pidController();

  // determine the charge mode and setpoint
  
  if(millis() > next_disp){
    if(amps_out_long_term_avg == NULL){
      amps_out_long_term_avg = a_amps_out.value()/1000.0;
      amps_out_medium_term_avg = a_amps_out.value()/1000.0;
    }

    amps_out_long_term_avg = a_amps_out.value()/1000.0 * ALPHA_LONG_TERM + amps_out_long_term_avg * (1-ALPHA_LONG_TERM);
    amps_out_medium_term_avg = a_amps_out.value()/1000.0 * ALPHA_MEDIUM_TERM + amps_out_medium_term_avg * (1-ALPHA_MEDIUM_TERM);
    
    disp();
    next_disp = millis() + 1000;
    
    // assess mode
    if(stage == BULK){
      if(mute_ttl == 0 && a_volts_out.value() > V_ABSORB*1000.0){
        setMode(VOLTAGE);
        stage = ABSORB;
        mv_target = V_ABSORB * 1000.0;
        mute_ttl = 5;
        amps_out_long_term_avg = amps_out_medium_term_avg + .5;
      }  
    }else if(stage== ABSORB){
      if(mute_ttl== 0 && abs(amps_out_long_term_avg - amps_out_medium_term_avg) < 0.05){
        setMode(VOLTAGE);
        stage = FLOAT;
        mv_target = V_FLOAT * 1000;
        mute_ttl = 5;
      }
    }else if(stage == FLOAT){
      // cool. we'll just chill here
    }
    if(mute_ttl > 0) mute_ttl--;
    
  }
  
  if(millis() > next_serial){
    next_serial = next_serial + 500;
    //Serial.print((float)mv_target/1000,2);
    //Serial.print("V, ");
    char buf[10] = "";
    stageStr(buf);
    Serial.print(String(buf)+ ",");
    Serial.print((float)a_volts_out.value()/1000,2);
    Serial.print("V, ");
    //Serial.print((float)a_amps_in.value()/1000,2);
    //Serial.print("A ,");
    Serial.print((float)a_amps_out.value()/1000,2);
    Serial.print("A, ");
    Serial.print(error);
    Serial.print(", ");
    Serial.print(duty);
    Serial.print(", ");
    Serial.print(integral);
    Serial.print(", ");
    Serial.println(String(amps_out_medium_term_avg,1) + "A, "+String(amps_out_long_term_avg,1)+"A");
  }
}


void Offsets::save(){

  OffsetComponents o = {
    offset_amps_in,
    offset_amps_out
  };

  EEPROM.put(0, o);
}

void Offsets::load(){
  lcd.print("Restore settings");
  OffsetComponents o;
  EEPROM.get(0, o);
  offset_amps_out = o.offset_amps_out;
  offset_amps_in = o.offset_amps_in;
}
