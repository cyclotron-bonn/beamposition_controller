#ifndef controlIO_h
#define controlIO_h

#include "Arduino.h"
#include <SPI.h>

void initDac();
void initADC();


class controlIO{
    
public:
    controlIO();
    ~controlIO();
    uint16_t readInput(uint8_t);
    int16_t calcNorm(uint8_t, uint8_t);
    void writeOutput(uint16_t); //write 12-bit value to dac
    uint16_t analogRead(uint8_t);
    
    int32_t IOnorm;
private:
    uint8_t bit_control_limit = 9;
    uint16_t readBuffer();
    void chooseChannel(uint8_t channel);
};

#endif /* controlIO_h */
