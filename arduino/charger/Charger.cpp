#include "Charger.h"
#include <LiquidCrystal_I2C.h>
#include "PWM16.h"

LiquidCrystal_I2C lcd(0x27,20,2);

Charger::Charger() {
  a_volts_out = Analogue(PIN_VOLTS_OUT, FILTER_FREQ, MV_PER_BIT);
  a_amps_out = Analogue(PIN_AMPS_OUT, FILTER_FREQ, MA_PER_BIT);
  a_amps_in = Analogue(PIN_AMPS_IN, FILTER_FREQ, MA_PER_BIT);
}

void Charger::init(){
  Serial.begin(115200);
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
  setMode(VOLTAGE);

  mv_target= 13600;
  ma_target = 2000;
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
    kI = 10;
  }else if(control_mode == VOLTAGE){
    kP = 15;
    kI = 1;
  }
  integral = 0;
}

void Charger::pidController(){
  

  if(control_mode == CURRENT){
    // basic P controller
    error = ma_target - a_amps_out.value();
    
    // scale it back into something sensibe
    error = error/MA_PER_BIT;
    
  }else if(control_mode == VOLTAGE){
    // basic P controller
    error = mv_target - a_volts_out.value();
    
    // scale it back into something sensibe
    error = error/MV_PER_BIT;

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



void Charger::go(){
  //for(uint16_t i = 0; i< 10; i++){
  a_volts_out.measure(100);
  a_amps_in.measure(100);
  a_amps_out.measure(100);    
  //}
  
  //analogues.read();
  pidController();

  // determine the charge mode and setpoint
  
  if(millis() > next_disp){
    disp();
    next_disp = millis() + 1000;
  }
  
  if(millis() > next_serial){
    next_serial = millis() + 100;
    //Serial.print((float)mv_target/1000,2);
    //Serial.print("V, ");
    Serial.print((float)a_volts_out.value()/1000,2);
    Serial.print("V, ");
    Serial.print((float)a_amps_in.value()/1000,2);
    Serial.print("A ,");
    Serial.print((float)a_amps_out.value()/1000,2);
    Serial.print("A, ");
    Serial.print(error);
    Serial.print(", ");
    Serial.print(duty);
    Serial.print(", ");
    Serial.println(integral);
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
/*

Analogues::Analogues(){
  flt_mv_out.setAsFilter( LOWPASS_BUTTERWORTH, 5.0 ); // 100Hz cutoff freq
  flt_ma_in.setAsFilter( LOWPASS_BUTTERWORTH, 5.0 ); // 100Hz cutoff freq
  flt_ma_out.setAsFilter( LOWPASS_BUTTERWORTH, 5.0 ); // 100Hz cutoff freq
}

void Analogues::calibrate(){
  offsets.offset_amps_in = getADCAvg(PIN_AMPS_IN, 10000);
  offsets.offset_amps_out = getADCAvg(PIN_AMPS_OUT, 10000);
  // save values
  offsets.save();
}

unsigned int Analogues::getADCAvg(byte pin, unsigned int n){
  unsigned long tmp=0;
  unsigned int i;
  for(i=0;i<n;i++){
    tmp = tmp + analogRead(pin); 
  }
  return tmp / n;
}


int Analogues::getMV(byte pin){
  long tmp = getADCAvg(pin, 50);
  flt_mv_out.input(tmp);
  //flt_mv_out_stats.input( flt_mv_out.output() );
  //Serial.println(tmp);
  tmp = flt_mv_out.output();
  tmp = tmp * MV_PER_BIT;
  if (tmp < 100) tmp = 0;
  return tmp;
}

int Analogues::getMA(byte pin){
  long tmp = getADCAvg(pin, 50);

  if(pin == PIN_AMPS_IN){
    tmp = tmp - offsets.offset_amps_in;
    flt_ma_in.input(tmp);
    tmp = flt_ma_in.output();
  }
  if(pin == PIN_AMPS_OUT){
    tmp = tmp - offsets.offset_amps_out;
    flt_ma_out.input(tmp);
    tmp = flt_ma_out.output();
  }
  
  tmp = tmp * MA_PER_BIT;
  if (tmp < 100) tmp = 0;
  return tmp;
}

void Analogues::read(){
  ma_in = getMA(PIN_AMPS_IN);
  ma_out = getMA(PIN_AMPS_OUT);
  mv_out = getMV(PIN_VOLTS_OUT);
}*/
