//#include <PIController.h>
#include "ADS130B04.h"


uint16_t voltages[] = {0,0,0,0,0,0,0,0};

void setup() {
  //SPIsetup(true);
  SPI.begin(); 
  Serial.begin(115200);
  delay(100);

}

void loop() {
  //bool error = analogReadMult(voltages, true);
  //Serial.println(error);
  for(uint8_t i=0; i<8;i++){
  }
  delay(1000);
}
