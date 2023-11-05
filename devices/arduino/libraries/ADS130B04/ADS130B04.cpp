#include "ADS130B04.h"


// SPISettings HighResolutionMode(8192000, MSBFIRST, SPI_MODE1);
// SPISettings LowPowerMode(4092000, MSBFIRST, SPI_MODE1);
// SPISettings VeryLowPowerMode(2048000, MSBFIRST, SPI_MODE1);


// const size_t n_adc = 4;
// _channel channels[n_adc] = {CH0, CH1, CH2, CH3};

ADS130B04::ADS130B04(uint8_t CS, uint8_t EOC_PIN){
    CS = CS;
    EOC_pin = EOC_PIN;
    SPIsetting = HighResolutionMode;
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

void ADS130B04::completeTransfer16(uint16_t command){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    SPI.transfer16(command);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS130B04::rreg(_register s_reg, uint8_t n=0){
    if(error_a_n(s_reg.addr,n)){
        return;
    }
    uint16_t command = 0b1010000000000000 + (s_reg.addr<<7) + n;
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    for(uint8_t i=0; i<4; i++){
        if(i==0){channels[i].value = SPI.transfer16(genCRC(command));}
        else channels[i].value = SPI.transfer16(0x00);  
    }
    uint16_t crc = SPI.transfer16(0x00);  
    digitalWrite(CS, HIGH);

    uint8_t n0 = 0;
    for(uint8_t i=0; i<n+1;i++){
        if(s_reg.addr == regs[i].addr){
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
}

void ADS130B04::wreg(_register s_reg, uint8_t n, uint16_t *new_data){
    if(error_a_n(s_reg.addr,n)){
        return;
    }
    uint16_t command = 0b0110000000000000 + (s_reg.addr<<7) + n;
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(command);
    if(n<n_adc){
        for(uint8_t i = 0; i<n; i++){
            channels[i].value = SPI.transfer16(new_data[i]);
        }
        for(uint8_t i = n; i<n_adc-2; i++){
            channels[i].value = SPI.transfer16(0x00);
        }
        channels[n_adc-1].value = SPI.transfer16(genCRC(command));
    }
    if(n==n_adc){
        for (size_t i = 0; i < n; i++){
            channels[i].value = SPI.transfer16(new_data[i]);
        }
        SPI.transfer16(genCRC(command));
    }
    if(n>n_adc){
        for (size_t i = 0; i < n_adc; i++){
            channels[i].value = SPI.transfer16(new_data[i]);
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

void ADS130B04::updateChannels(){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = SPI.transfer16(0x00);
    for(uint8_t i=0; i<4; i++){
        if(i==0){channels[i].value = SPI.transfer16(genCRC(0x00));}
        else channels[i].value = SPI.transfer16(0x00);  
    }
    uint16_t crc = SPI.transfer16(0x00);  
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
    transADC();
}

void _channel::increaseGain(){
    uint16_t new_data = this->gain.upper + (1<<this->gain.lower);
    if((new_data<<(16-this->gain.upper+1))>>15){
        return;
    }
    wreg(gain_reg, 0, new_data);
}

void _channel::decreaseGain(){
    return;
}

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



// uint16_t* ADS130B04::query(uint16_t command, uint8_t n){
//     SPI.beginTransaction(SPIsetting);
//     digitalWrite(CS, LOW);
//     STATUS.content = SPI.transfer16(command);
//     for(uint8_t i=0; i<4; i++){
//         if(i==0){channels[i].value = SPI.transfer16(genCRC(command));}
//         else channels[i].value = SPI.transfer16(0x00);  
//     }
//     uint16_t crc = SPI.transfer16(0x00);  
//     digitalWrite(CS, HIGH);
//     uint8_t register_loc = 0;
//     uint16_t *response = new uint16_t[n+1];
//     digitalWrite(CS, LOW);
//     for (uint8_t i = 0; i < n+1; i++){
//        response[i] = SPI.transfer16(0x00);
//     }
//     digitalWrite(CS, HIGH);
//     SPI.endTransaction();
//     return response;
// }

void ADS130B04::setCRC(){
    return;
}

void ADS130B04::getCRC(){
    return;
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
        if(channels[i].value>=0x7FFF){
            channels[i].value-=0x10000;
        }
    }
}

bool ADS130B04::error_a_n(uint16_t a, uint16_t n){
    if(a>adress_lim||n>number_lim){
        return true;
    }
    return false;
}

uint16_t ADS130B04::masked_bit(uint16_t bit, uint16_t bit2, _bitPos pos){
    uint16_t bit_mask = (pos.lower<<pos.upper);
    uint16_t newbit = (bit & (~bit_mask)) | (bit2<<pos.upper);
    return newbit;
}

void ADS130B04::enable(_channel ch){
    return;
}

void ADS130B04::disable(_channel ch){
    return;
}










