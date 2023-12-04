#include "ADS130B04.h"

ADS130B04 AD = ADS130B04(2,3);

void setup() {
  //SPIsetup(true);
  SPI.begin(); 
  Serial.begin(115200);
  delay(100);
  AD.wakeup();
  AD.setWordLength16();
}

void loop() {
  //bool error = analogReadMult(voltages, true);
  //Serial.println(error);
  AD.rreg(AD.CLOCK, 0);
  Serial.println(AD.CLOCK.content, BIN);
  delay(1000);
}
