#include "Charger.h"
Charger charger;
void setup() {
  charger.init();
} 

void loop() {
  // put your main code here, to run repeatedly:
  charger.go();
} 
