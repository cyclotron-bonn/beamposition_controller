#include "SPIComm.h"

uint8_t command;
uint8_t n_volts;

void SPIsetup(bool doubleslit){
    pinMode(ADC_CS, OUTPUT);
    pinMode(ADC_EOC, INPUT);
    pinMode(DAC_CS, OUTPUT);
    digitalWrite(ADC_CS, HIGH);
    digitalWrite(DAC_CS, HIGH);
    delay(100);
    if(doubleslit){
        command = 0b11101111;
        n_volts = 8;
    }
    else{
        command = 0b11110111;
        n_volts = 4;
    }
    SPI.begin();
}

bool analogReadMult(uint16_t *voltages){
    digitalWrite(ADC_CS, LOW);
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); //for max1168
    SPI.transfer(command);
    waitEOC();
    for(uint8_t i=0; i<n_volts; i++){
        voltages[i] = SPI.transfer16(0);
    }
    digitalWrite(ADC_CS, HIGH);
    SPI.endTransaction();
    return false;
}

uint16_t analogReadPrec(uint8_t channel){
    if (channel > 7){
        return 0;
    }
    uint8_t commandPrec = (channel<<5)+0b00011111;
    digitalWrite(ADC_CS, LOW);
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); //for max1168
    SPI.transfer(commandPrec);
    waitEOC();
    uint16_t voltage = SPI.transfer16(0x0);
    Serial.println(voltage);
    digitalWrite(ADC_CS, HIGH);
    SPI.endTransaction();
    return voltage;
}

void analogWrite(uint16_t value){
    digitalWrite(DAC_CS, LOW);
    //SPI.transfer16(adc_channel);
    //SPI.transfer16(value);
    digitalWrite(DAC_CS, HIGH);
}

