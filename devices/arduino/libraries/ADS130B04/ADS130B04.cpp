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
};

register ID;
register STATUS;
register MODE;
register CLOCK;
register GAIN;
register GLOBAL_CHOP_CFG;
register CH0_CFG;
register CH1_CFG;
register CH2_CFG;
register CH3_CFG;
register REG_CRC;

ID.addr = 0x00;
STATUS.addr = 0x01;
MODE.addr = 0x02;
CLOCK.addr = 0x03;
GAIN.addr = 0x04;
GLOBAL_CHOP_CFG.addr = 0x06;
CH0_CFG.addr = 0x09;
CH1_CFG.addr = 0x0E;
CH2_CFG.addr = 0x13;
CH3_CFG.addr = 0x18;
REG_CRC.addr = 0x3E;


ADS130B04::ADS130B04(uint8_t CS_PIN, uint8_t EOC_PIN){
    CS = CS_PIN;
    EOC = EOC_PIN;
    CHx_VAL = {0};
    //fill register contents
}

ADS130B04::~ADS130B04(){
}

void ADS130B04::null(){
    query(commands.null);
    transADC();
}

void ADS130B04::reset(){
    query(commands.reset);

}

void ADS130B04::standby(){
    command = 0b100010;
    query(commands.standby);
}

void ADS130B04::wakeup(){
    query(commands.wakeup);
}

void ADS130B04::lock(){
    query(commands.lock);
}

void ADS130B04::unlock(){
    query(commands.unlock);
}

int ADS130B04::rreg(uint16_t a, uint16_t n){
    if(error_a_n(a,n)){
        return -99;
    }
    command = 0b1010000000000000 + (a<<7) + n;
    query(command, );
    return 0;
}

void ADS130B04::wreg(uint16_t n, uint16_t a){
    if(error_a_n(a,n)){
        return;
    }
    command = 0b0110000000000000 + (a<<7) + n;
}



void ADS130B04::increaseGain(uint8_t channel){
    int a = rreg(0,0);
}

void ADS130B04::decreaseGain(uint8_t channel){
    
}

void ADS130B04::setGain(uint8_t channel, uint8_t gain){
    
}

void ADS130B04::getGain(uint8_t channel){
    
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

void ADS130B04::query(uint16_t command, struct register *regs[], uint8_t n){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    for(uint8_t i; i<4; i++){
        if(i==0){CHx_VAL[i] = SPI.transfer16(gen_crc(command));}
        else CHx_VAL[i] = SPI.transfer16(0x00);  
    }
    uint16_t crc = SPI.transfer16(0x00);  
    digitalWrite(CS, HIGH);

    digitalWrite(CS, LOW);
    for (uint8_t i = 0; i < n; i++){
       regs->content = SPI.transfer16(0x00);
    }

    digitalWrite(pins.CS, HIGH);
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









