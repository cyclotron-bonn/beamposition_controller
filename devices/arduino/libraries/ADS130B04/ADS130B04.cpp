#include "ADS130B04.h"


SPISettings HighResolutionMode(8192000, MSBFIRST, SPI_MODE1);
SPISettings LowPowerMode(4092000, MSBFIRST, SPI_MODE1);
SPISettings VeryLowPowerMode(2048000, MSBFIRST, SPI_MODE1);
SPISettings SPIsetting;


uint8_t CS;
uint8_t EOC;


uint16_t CMD_RESPONSE;
uint16_t CHx_VAL[4] = {0};
uint16_t REG_VAL[16] = {0};

struct _register{
    uint8_t addr;
    uint16_t content;
    _register(uint8_t address) : addr(address){};
};


_register ID(0x00);
_register STATUS(0x01);
_register MODE(0x02);
_register CLOCK(0x03);
_register GAIN(0x04);
_register GLOBAL_CHOP_CFG(0x06);
_register CH0_CFG(0x09);
_register CH1_CFG(0x0E);
_register CH2_CFG(0x13);
_register CH3_CFG(0x18);
_register REG_CRC(0x3E);



_register regs[11] = {ID, STATUS, MODE, CLOCK, GAIN, GLOBAL_CHOP_CFG, CH0_CFG, CH1_CFG, CH2_CFG, CH3_CFG, REG_CRC};

ADS130B04::ADS130B04(uint8_t CS_PIN, uint8_t EOC_PIN){
    CS = CS_PIN;
    EOC = EOC_PIN;
    rreg(ID, 10);
}

ADS130B04::~ADS130B04(){
}

// void ADS130B04::null(){
    
//     query(0x00, &regs, 10);
//     transADC();
// }

// void ADS130B04::reset(){
//     query(cReset);

// }

// void ADS130B04::standby(){
//     query(cStandby);
// }

// void ADS130B04::wakeup(){
//     query(cWakeup);
// }

// void ADS130B04::lock(){
//     query(cLock);
// }

// void ADS130B04::unlock(){
//     query(cUnlock);
// }

int ADS130B04::rreg(_register r, uint16_t n){
    if(error_a_n(r.addr,n)){
        return -99;
    }
    uint16_t command = 0b1010000000000000 + (r.addr<<7) + n;
    query(command, n+1);
    return 0;
}

void ADS130B04::wreg(uint16_t n, uint16_t a){
    if(error_a_n(a,n)){
        return;
    }
    uint16_t command = 0b0110000000000000 + (a<<7) + n;
}



// void ADS130B04::increaseGain(uint8_t channel){
//     int a = rreg(0,0);
// }

// void ADS130B04::decreaseGain(uint8_t channel){
    
// }

// void ADS130B04::setGain(uint8_t channel, uint8_t gain){
    
// }

// void ADS130B04::getGain(uint8_t channel){
    
// }

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

void ADS130B04::query(uint16_t command, uint8_t n){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    for(uint8_t i; i<4; i++){
        if(i==0){CHx_VAL[i] = SPI.transfer16(genCRC(command));}
        else CHx_VAL[i] = SPI.transfer16(0x00);  
    }
    uint16_t crc = SPI.transfer16(0x00);  
    digitalWrite(CS, HIGH);

    digitalWrite(CS, LOW);
    for (uint8_t i = 0; i < n; i++){
       regs->content = SPI.transfer16(0x00);
    }

    digitalWrite(CS, HIGH);
}

uint16_t ADS130B04::genCRC(uint16_t command){
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
        if(CHx_VAL[i]>=0x7FFF){
            CHx_VAL[i]-=0x10000;
        }
    }
}



bool ADS130B04::error_a_n(uint16_t a, uint16_t n){
    if(a>adress_lim||n>number_lim){
        return true;
    }
    return false;
}









