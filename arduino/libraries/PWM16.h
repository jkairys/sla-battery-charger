#ifndef PWM16_h
#define PWM16_h
#include <Arduino.h>

class PWM16{
  public:
    PWM16();
    PWM16(uint8_t pin);
    void analogWrite(uint16_t val);
  private:
    uint8_t _pin;
};

#endif