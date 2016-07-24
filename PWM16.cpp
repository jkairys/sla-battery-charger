#include "PWM16.h"

PWM16::PWM16(){

}

PWM16::PWM16(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
   DDRB |= _BV(PB1) | _BV(PB2);        /* set pins as outputs */
  TCCR1A = _BV(COM1A1) | _BV(COM1B1)  /* non-inverting PWM */
      | _BV(WGM11);                   /* mode 14: fast PWM, TOP=ICR1 */
  TCCR1B = _BV(WGM13) | _BV(WGM12)
      | _BV(CS10);                    /* no prescaling */
  ICR1 = 0xffff;                      /* TOP counter value */
}

void PWM16::analogWrite(uint16_t val){
  switch (_pin) {
    case  9: OCR1A = val; break;
    case 10: OCR1B = val; break;
  }
}