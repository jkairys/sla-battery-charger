#ifndef Analogue_h
#define Analogue_h
#include <Filters.h>
#include <Arduino.h>

class Analogue{
  public:
    Analogue();
    Analogue(uint8_t pin, float freq, short scaleFactor);

    uint16_t measure();
    uint16_t measure(uint16_t val);
    long value();
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
