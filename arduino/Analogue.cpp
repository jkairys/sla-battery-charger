#include "Analogue.h"

Analogue::Analogue(){

}

Analogue::Analogue(uint8_t pin, float freq, short scaleFactor) {
  pinMode(pin, INPUT);
  _freq = freq;
  _scaleFactor = scaleFactor;
  _flt.setAsFilter( LOWPASS_BUTTERWORTH, freq);
  _raw = 0;
}

// measure(n)
// Performs n ADC readings and return the average
uint16_t Analogue::measure(unsigned int n){
  unsigned long tmp=0;
  unsigned int i;
  for(i=0;i<n;i++){
    tmp = tmp + analogRead(_pin); 
  }
  _raw = tmp / n;
  _flt.input(_raw);
  return _raw;
}

// calibrate()
// Determine the zero-offset by recording the ADC measurement (must not have any signal present)
void Analogue::calibate(){
  // get a reading
  measure(1000);
  _offset = _raw;
}

// value()
// Return the output value from the filter
int Analogue::value(){
  long tmp = _flt.output();
  tmp = tmp - _offset;
  tmp = tmp * _scaleFactor;
  return tmp;
}