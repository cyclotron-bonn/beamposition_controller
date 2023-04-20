#ifndef SPIComm_h
#define SPIComm_h

#include <Arduino.h>
#include <SPI.h>



extern int CS_PIN;

class SPIComm{
public:
    SPIComm();
    ~SPIComm();
    void analogWrite(uint16_t);
    uint16_t analogRead();



private:
    uint16_t dac_channel;
    uint16_t adc_channel;
};

#endif 

