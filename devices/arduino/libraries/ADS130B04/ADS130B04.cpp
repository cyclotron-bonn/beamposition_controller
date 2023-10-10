#include "ADS130B04.h"


SPISettings HighResolutionMode(8192000, MSBFIRST, SPI_MODE1);
SPISettings LowPowerMode(4092000, MSBFIRST, SPI_MODE1);
SPISettings VeryLowPowerMode(2048000, MSBFIRST, SPI_MODE1);
SPISettings SPIsetting;

struct PINS{
        uint8_t CS;
        uint8_t EOC;
    };

struct DATA{
    uint16_t CMD_RESPONSE;
    uint16_t CHx_VAL[4] = {0};
    uint16_t REG_VAL[16] = {0};
    bool REG_R;
};  

PINS pins;
DATA data;


ADS130B04::ADS130B04(uint8_t CS_PIN, uint8_t EOC_PIN){
    pins.CS = CS_PIN;
    pins.EOC = EOC_PIN;
    data.REG_R = false;
}

ADS130B04::~ADS130B04(){
}

void ADS130B04::null(){
    command = 0b00;
    query();
    transADC();
}

void ADS130B04::reset(){
    command = 0b10001;
    query();

}

void ADS130B04::standby(){
    command = 0b100010;
    query();
}

void ADS130B04::wakeup(){
    command = 0b110011;
    query();
}

void ADS130B04::lock(){
    command = 0b10101010101;
    query();
}

void ADS130B04::unlock(){
    command = 0b11001010101;
    query();
}

int ADS130B04::rreg(uint16_t a, uint16_t n){
    if(error_a_n(a,n)){
        return -99;
    }
    command = 0b1010000000000000 + (a<<7) + n;
    query();
    return 0;
}

void ADS130B04::wreg(uint16_t n, uint16_t a){
    if(error_a_n(a,n)){
        return;
    }
    command = 0b0110000000000000 + (a<<7) + n;
}



void ADS130B04::GAIN::increase(uint8_t channel){
    rreg()
}

void ADS130B04::GAIN::decrease(uint8_t channel){
    
}

void ADS130B04::GAIN::set(uint8_t channel, uint8_t gain){
    
}

void ADS130B04::GAIN::get(uint8_t channel){
    
}

void ADS130B04::setSPIsetting(uint8_t mode){
    switch (mode){
        case 0:
            SPIsetting = HighResolutionMode;
            break;

        case 1:
            SPIsetting = LowPowerMode;
            break;

        case 2:
            SPIsetting = VeryLowPowerMode;
            break;
            
        default:
            break;
    }
}

void ADS130B04::setPowerMode(uint8_t mode){
    setSPIsetting(mode);
    //command = getclocksetting();
}

void ADS130B04::query(){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(pins.CS, LOW);
    data.CMD_RESPONSE = SPI.transfer16(command);
    uint8_t n_vals = 0;
    if(data.REG_R){n_vals = n_reg;}
    else n_vals = 4;

    uint16_t transmit = 0x00;
    for(uint8_t i; i<n_vals; i++){
        transmit = uint16_t((crc_mode&&i==0)*gen_crc(command));
        if(data.REG_R){
            data.REG_VAL[i] = SPI.transfer16(transmit);
        }
        else data.CHx_VAL[i] = SPI.transfer16(transmit);  
    }

    digitalWrite(pins.CS, HIGH);
    if(command!=0x00){
        data.REG_R = true;
    }
}

uint16_t ADS130B04::gen_crc(uint16_t command){
    if(crc_mode == 1){
        return command^ccitt_crc_poly;
    }
    if(crc_mode == 2){
        return command^ansi_crc_poly;
    }
    return 0;
    
}

void ADS130B04::transADC(){
    for(uint8_t i=0; i<4; i++){
        if(data.CHx_VAL[i]>=0x7FFF){
            data.CHx_VAL[i]-=0x10000;
        }
    }
}



bool ADS130B04::error_a_n(uint16_t a, uint16_t n){
    if(a>ADDRESS_LIM||n>NUMBER_LIM){
        return true;
    }
    return false;
}









