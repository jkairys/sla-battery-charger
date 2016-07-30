#ifndef Charger_h
#define Charger_h
#include <Arduino.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Filters.h>
#include "PWM16.h"


// Configurable Options
#define PIN_AMPS_IN A0
#define PIN_AMPS_OUT A1
#define PIN_VOLTS_OUT A2
#define PIN_VOLTS_IN -1

#define PIN_DUTY 9
#define PIN_CALIBRATE 12

#define MA_PER_BIT 73
#define MV_PER_BIT 30

enum CHARGER_MODE { 
  MODE_CONSTANT_CURRENT, 
  MODE_CONSTANT_VOLTAGE, 
  MODE_FLOAT
};

struct OffsetComponents {
  unsigned int offset_amps_in;
  unsigned int offset_amps_out;
};

class Offsets {
public:
  unsigned int offset_amps_in;
  unsigned int offset_amps_out;
  void save();
  void load();
};

class Analogues{
  private:
    unsigned int getADCAvg(byte pin, unsigned int n);
    int getMA(byte pin);
    int getMV(byte pin);
    FilterTwoPole flt_mv_out;                                       // create a two pole Lowpass filter
    FilterTwoPole flt_ma_in;
    FilterTwoPole flt_ma_out;

  public:
    short ma_in;
    short ma_out;
    short mv_in;
    short mv_out;
    Analogues();
    void init(byte pin_ma_in, byte pin_ma_out, byte pin_mv_in, byte pin_mv_out);
    Offsets offsets;
    void calibrate();
    void read();
};





class Charger{

  private:
    void disp();
    unsigned long next_disp;
  public:

    CHARGER_MODE mode;
    Analogues analogues;
    


    PWM16 pwm;

    Charger();

    void init();
    //void loadSettings();
    //void saveSettings();
    void go();


};



// Don't change anything below this line








#endif

