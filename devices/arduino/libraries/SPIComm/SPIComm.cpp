#include "SPIComm.h"


int CS_PIN=3;

SPIComm::SPIComm(){
}

SPIComm::~SPIComm(){
}

void SPIComm::analogWrite(uint16_t value){
    digitalWrite(CS_PIN, LOW);
    SPI.transfer16(adc_channel);
    SPI.transfer16(value);
    digitalWrite(CS_PIN, HIGH);
}

uint16_t SPIComm::analogRead(){
    digitalWrite(CS_PIN, LOW);
    uint16_t val = SPI.transfer(0x0);
    digitalWrite(CS_PIN, HIGH);
    return val;
}