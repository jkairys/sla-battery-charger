#include "Charger.h"
#include <LiquidCrystal_I2C.h>
#include "PWM16.h"

LiquidCrystal_I2C lcd(0x27,20,2);

Charger::Charger() {
  
  //analogues.init(PIN_AMPS_IN, PIN_AMPS_OUT, PIN_VOLTS_IN, PIN_VOLTS_OUT);
  //next_disp = millis();

  
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
  analogues.offsets.load();
  pinMode(PIN_CALIBRATE, INPUT);
  lcd.clear();
  if(!digitalRead(PIN_CALIBRATE)){
    // perform calibration
    lcd.print("Calibrating");
    analogues.calibrate();  
  }else{
    analogues.offsets.load();
  }
  //delay(500);
  lcd.clear();
  lcd.print("In: ");
  lcd.print(analogues.offsets.offset_amps_in, DEC);
  lcd.setCursor(0,1);
  lcd.print("Out: ");
  lcd.println(analogues.offsets.offset_amps_out, DEC);
  //delay(500);

  //flt_mv_out_stats.setWindowSecs( 20.0/100.0 );

  // go to constant current mode
  mode = MODE_CONSTANT_CURRENT;
}


void Charger::disp(){
  
  lcd.setCursor(0,0);
  lcd.print("In: ");
  lcd.print((float)analogues.ma_in/1000.0,1);
  lcd.print("A      ");
  lcd.setCursor(0,1);
  lcd.print("Out: ");
  lcd.print((float)analogues.ma_out/1000.0,1);
  lcd.print("A ");
  lcd.print((float)analogues.mv_out/1000.0,1);
  lcd.print("V ");
}




void Charger::go(){

  analogues.read();

  // determine the charge mode and setpoint
  //controlAmps();
  //controlVolts();
  
  if(millis() > next_disp){
    disp();
    next_disp = millis() + 1000;
    //Serial.print((float)mv_target/1000,2);
    //Serial.print("V, ");
    Serial.print((float)analogues.mv_out/1000,2);
    Serial.print("V, ");
    Serial.print((float)analogues.ma_in/1000,2);
    Serial.print("A ,");
    Serial.print((float)analogues.ma_out/1000,2);
    Serial.println("A");
    //Serial.print(error);
    //Serial.print(", ");
    //Serial.print(duty);
    //Serial.print(", ");
    //Serial.println(integral);
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
}
