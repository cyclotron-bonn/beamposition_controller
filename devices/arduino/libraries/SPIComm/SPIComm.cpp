#include "SPIComm.h"



// struct{
//     uint8_t adc;
//     uint8_t dac;
// }n_val;

uint8_t n_volts;
uint16_t command;
uint16_t codes[16];
uint8_t crc_mode = 0;
uint16_t ccitt_crc_poly = 0b0001000000100001;
uint16_t ansi_crc_poly = 0b1000000000000101;

SPISettings HighResolutionMode(8192000, MSBFIRST, SPI_MODE1);
SPISettings LowPowerMode(4092000, MSBFIRST, SPI_MODE1);
SPISettings VeryLowPowerMode(2048000, MSBFIRST, SPI_MODE1);
SPISettings SPIsetting;

// void SPIsetup(bool doubleslit){
//     pinMode(ADC_CS, OUTPUT);
//     pinMode(ADC_EOC, INPUT);
//     pinMode(DAC_CS, OUTPUT);
//     digitalWrite(ADC_CS, HIGH);
//     digitalWrite(DAC_CS, HIGH);
//     delay(100);
//     setPowerMode(0);
//     SPI.begin();
// }

void setSPIsetting(uint8_t mode){
    switch (mode)
    {
    case 0:
        SPIsetting = HighResolutionMode;
        break;
    
    case 1:
        SPIsetting = LowPowerMode;
        break;
    
    case 2:
        SPIsetting = VeryLowPowerMode;

    default:
        break;
    }
}

void setPowerMode(uint8_t mode){
    setSPIsetting(mode);
    //command = getclocksetting();
}

void DACsetup(){
    digitalWrite(DAC_CS, LOW);
    uint8_t output = 0b01000000;
    uint16_t value = 0x0000;
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(output);
    SPI.transfer16(value);
    digitalWrite(DAC_CS, HIGH);
}

// bool analogReadMult(uint16_t *voltages){
//     digitalWrite(ADC_CS, LOW);
//     SPI.beginTransaction(SPISettings(4800000, MSBFIRST, SPI_MODE0)); //for max1168
//     SPI.transfer(commands.adcMult);
//     waitEOC();
//     for(uint8_t i=0; i<n_val.adc; i++){
//         voltages[i] = SPI.transfer16(0);
//     }
//     digitalWrite(ADC_CS, HIGH);
//     SPI.endTransaction();
//     return false;
// }

// uint16_t analogReadPrec(uint8_t channel){
//     if (channel > 7){
//         return 0;
//     }
//     uint8_t commandPrec = (channel<<5)+commands.adcPrec;
//     digitalWrite(ADC_CS, LOW);
//     SPI.beginTransaction(SPISettings(4800000, MSBFIRST, SPI_MODE0)); //for max1168
//     SPI.transfer(commandPrec);
//     waitEOC();
//     uint16_t voltage = SPI.transfer16(0x0);
//     digitalWrite(ADC_CS, HIGH);
//     SPI.endTransaction();
//     return voltage;
// }

// void analogWriteMult(uint16_t *voltages){
//     digitalWrite(DAC_CS, LOW);
//     SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
//     uint8_t i=0;
//     for(i=0; i<n_val.dac; i++){
//         uint8_t cmd = (commands.dac<<4)+(0x01 << i);
//         SPI.transfer(cmd);
//         SPI.transfer16(voltages[i]);
//     }
//     SPI.endTransaction();
//     digitalWrite(DAC_CS, HIGH);
// }

// void analogWritePrec(uint16_t voltage, uint8_t channel){
//     if(channel>n_val.dac){
//         return;
//     }
//     digitalWrite(DAC_CS, LOW);
//     SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
//     uint8_t cmd = (commands.dac)+(0x01<<channel);
//     SPI.transfer(cmd);
//     SPI.transfer16(voltage);
//     SPI.endTransaction();
//     digitalWrite(DAC_CS, HIGH);
// }

void query(){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(ADC_CS, LOW);
    SPI.transfer16(command);
    if(crc_mode){
        SPI.transfer16(get_crc(command));
    }
    digitalWrite(ADC_CS, HIGH);
    digitalWrite(ADC_CS, LOW);
    SPI.transfer16(0x00);
}

uint16_t get_crc(uint16_t command){
    if(crc_mode == 1){
        return command^ccitt_crc_poly;
    }
    return command^ansi_crc_poly;
}

void transADC(uint16_t *codes[], uint8_t n){
    *codes = new uint16_t[n];
    for(uint8_t i=0; i<4; i++){
        if((*codes)[i]>=0x7FFF){
            (*codes)[i] -= 0x10000;
        }
    }
}

void null(uint16_t &values){
    command = 0b00;
    query();
    uint16_t * adc_codes = new uint16_t[4];
    transADC(&adc_codes, 4);
}

void reset(){
    command = 0b10001;
    query();

}

void standby(){
    command = 0b100010;
    query();
}

void wakeup(){
    uint16_t command = 0b110011;
    query();
}

void lock(){
    command = 0b10101010101;
    query();
}

void unlock(){
    command = 0b11001010101;
    query();
}

bool error_a_n(uint16_t a, uint16_t n){
    if(a>ADDRESS_LIM||n>NUMBER_LIM){
        return true;
    }
    return false;
}

int rreg(uint16_t a, uint16_t n){
    if(error_a_n(a,n)){
        return -99;
    }
    command = 0b1010000000000000 + (a<<7) + n;
    int response = 0;
    query();

    return response;

}

void wreg(uint16_t n, uint16_t a){
    if(error_a_n(a,n)){
        return;
    }
    command = 0b0110000000000000 + (a<<7) + n;
}


struct{
    /*device settings and status indicators (read-only registers)
    detailed description in datasheet: https://www.ti.com/lit/ds/symlink/ads130b04-q1.pdf?HQS=dis-mous-null-mousermode-dsf-pf-null-wwe&ts=1695742377424&ref_url=https%253A%252F%252Fwww.mouser.de%252F*/
    uint16_t ID = 0x00;
    uint16_t STATUS = 0x01;
    //global settings across channels
    uint16_t MODE = 0x02;
    uint16_t CLOCK = 0x03;
    uint16_t GAIN = 0x04;
    uint16_t GLOBAL_CHOP_CFG = 0x06;
    //channel-specific settings
    uint16_t CHx_CFG[4] = {0x09, 0x0E, 0x13, 0x18};
    //register map crc register
    uint16_t REG_CRC = 0x3E;
}addr;

