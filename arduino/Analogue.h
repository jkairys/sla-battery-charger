#ifndef Analogue_h
#define Analogue_h
#include <Arduino.h>

class Analogue{
  public:
    PWM16();
    Analogue(uint8_t pin, float freq, short scaleFactor);
    
    void measure(uint16_t val);
    int value();
    void calibrate();

  private:
    uint8_t _pin;
    uint16_t _scaleFactor;
    float _freq;
    unsigned int _raw;

    FilterTwoPole _flt;
    uint16_t _offset;
};

#endif