#include "ADS130B04.h"


SPISettings HighResolutionMode(8192000, MSBFIRST, SPI_MODE1);
SPISettings LowPowerMode(4092000, MSBFIRST, SPI_MODE1);
SPISettings VeryLowPowerMode(2048000, MSBFIRST, SPI_MODE1);
SPISettings SPIsetting;


uint8_t CS;
uint8_t EOC;

const size_t n_adc = 4;
uint16_t CMD_RESPONSE;
uint16_t CHx_VAL[n_adc] = {0};

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


const size_t n_regs = 11;
_register regs[n_regs] = {ID, STATUS, MODE, CLOCK, GAIN, GLOBAL_CHOP_CFG, CH0_CFG, CH1_CFG, CH2_CFG, CH3_CFG, REG_CRC};

ADS130B04::ADS130B04(uint8_t CS_PIN, uint8_t EOC_PIN){
    CS = CS_PIN;
    EOC = EOC_PIN;
    rreg(ID, 10);
}

ADS130B04::~ADS130B04(){
    
}

void ADS130B04::null(){
    completeTransfer16(0x00);
}

void ADS130B04::reset(){
    completeTransfer16(cReset);

}

void ADS130B04::standby(){
    completeTransfer16(cStandby);
}

void ADS130B04::wakeup(){
    completeTransfer16(cWakeup);
}

void ADS130B04::lock(){
    completeTransfer16(cLock);
}

void ADS130B04::unlock(){
    completeTransfer16(cUnlock);
}

void completeTransfer16(uint16_t command){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    SPI.transfer16(command);
    digitalWrite(CS, HIGH);
    SPI.endTransaction(SPIsetting);
}

void ADS130B04::rreg(_register r, uint16_t n=0){
    if(error_a_n(r.addr,n)){
        return -99;
    }
    uint16_t command = 0b1010000000000000 + (r.addr<<7) + n;
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    for(uint8_t i=0; i<4; i++){
        if(i==0){CHx_VAL[i] = SPI.transfer16(genCRC(command));}
        else CHx_VAL[i] = SPI.transfer16(0x00);  
    }
    uint16_t crc = SPI.transfer16(0x00);  
    digitalWrite(CS, HIGH);

    uint8_t n0 = 0;
    for(uint8_t i=0; i<n+1;i++){
        if(r.addr == regs[i].addr){
            n0 = i;
        }
    }
    digitalWrite(CS, LOW);
    for (uint8_t i = n0; i < n+1; i++){
       regs[i].content = SPI.transfer16(0x00);
    }
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS130B04::wreg(_register r, uint16_t n, uint16_t *new_data){
    if(error_a_n(r.addr,n)){
        return;
    }
    uint16_t command = 0b0110000000000000 + (r.addr<<7) + n;
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    if(n<n_adc){
        for(uint8_t i = 0; i<n; i++){
            CHx_VAL[i] = SPI.transfer16(new_data[i]);
        }
        for(uint8_t i = n; i<n_adc-1; i++){
            Chx_VAL[i] = SPI.transfer16(0x00);
        }
        Chx_VAL[n_adc] = SPI.transfer16(genCRC(command));
    }
    if(n==n_adc){
        for (size_t i = 0; i < n; i++){
            Chx_VAL[i] = SPI.transfer16(new_data[i]);
        }
        SPI.transfer16(genCRC(command));
    }
    if(n>n_adc){
        for (size_t i = 0; i < n_adc; i++){
            CHx_VAL[i] = SPI.transfer16(new_data[i]);
        }
        for (size_t i = n_adc; i < n; i++)
        {
            SPI.transfer16(new_data[i]);
        }
        SPI.transfer16(genCRC(command));
    }
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
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

void ADS130B04::updateADC(){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    for(uint8_t i=0; i<4; i++){
        if(i==0){CHx_VAL[i] = SPI.transfer16(genCRC(command));}
        else CHx_VAL[i] = SPI.transfer16(0x00);  
    }
    uint16_t crc = SPI.transfer16(0x00);  
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
    transADC();
}

uint16_t* ADS130B04::query(uint16_t command, uint8_t n){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    for(uint8_t i=0; i<4; i++){
        if(i==0){CHx_VAL[i] = SPI.transfer16(genCRC(command));}
        else CHx_VAL[i] = SPI.transfer16(0x00);  
    }
    uint16_t crc = SPI.transfer16(0x00);  
    digitalWrite(CS, HIGH);
    uint8_t register_loc = 0;
    uint16_t *response = new uint16_t[n+1];
    digitalWrite(CS, LOW);
    for (uint8_t i = 0; i < n+1; i++){
       response[i] = SPI.transfer16(0x00);
    }
    digitalWrite(CS, HIGH);
    return response;
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









