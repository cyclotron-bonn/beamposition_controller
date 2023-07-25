#ifndef SPIComm_h
#define SPIComm_h

#include <Arduino.h>
#include <SPI.h>

#define waitEOC() while(digitalRead(ADC_EOC))

const uint8_t ADC_CS = 2;
const uint8_t ADC_EOC = 3;
const uint8_t DAC_CS = 4;

void SPIsetup(bool doubleslit);
bool analogReadMult(uint16_t *controllers);
uint16_t analogReadPrec(uint8_t channel);
void analogWrite(uint16_t);



#endif 

