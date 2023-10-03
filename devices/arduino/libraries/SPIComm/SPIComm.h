#ifndef SPIComm_h
#define SPIComm_h

#include <Arduino.h>
#include <SPI.h>

#define WAIT_EOC() while(digitalRead(ADC_EOC))
#define ADDRESS_LIM (0b1<<7)-1 //address is 6 bit
#define NUMBER_LIM (0b1<<6)-1 //number of registers read at once is limited at 7 bit

const uint8_t ADC_CS = 2;
const uint8_t ADC_EOC = 3;
const uint8_t DAC_CS = 4;

void SPIsetup(bool doubleslit);
void setPowerMode(uint8_t);
void DACsetup();
// bool analogReadMult(uint16_t *channels);
// uint16_t analogReadPrec(uint8_t channel);
// void analogWriteMult(uint16_t *voltages);
// void analogWritePrec(uint16_t voltage, uint8_t channel);
void convertCode(uint16_t&);
void null(uint16_t*);
void query();
uint16_t get_crc(uint16_t);
void transADC(uint16_t *codes[], uint8_t n);
void reset();
void standby();
void wakeup();
void lock();
void unlock();
bool check_a_n(uint16_t, uint16_t);
int rreg(uint16_t, uint16_t);
void wreg(uint16_t, uint16_t);

#endif 

