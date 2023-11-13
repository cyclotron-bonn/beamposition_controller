#include "ADS130B04.h"

ADS130B04::ADS130B04(uint8_t CS_PIN, uint8_t EOC_PIN){
    CS = CS_PIN;
    EOC = EOC_PIN;
    SPIsetting = HighResolutionMode;
    setWordLength16();
}

ADS130B04::~ADS130B04(){
    reset();
}

void ADS130B04::null(){
    completeTransfer16(0x00);
}

void ADS130B04::reset(){
    completeTransfer16(cReset);
    setWordLength16();
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

void ADS130B04::setWordLength16(){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    uint16_t ad_command = 0b1010000000000000 + (MODE.addr<<7);
    _bitPos wl = _bitPos(8,9);
    uint16_t data = change_bits(MODE.content, 0b00, wl);
    SPI.transfer16(ad_command);
    SPI.transfer(0x00);
    SPI.transfer16(data);
    SPI.transfer(0x00);
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

void ADS130B04::increaseGain(_channel ch){
    uint16_t cgain = extract_bits(GAIN.content, ch.gain);
    if(cgain == 0b111){
        return; //dont do anything if gain already at max
    }
    uint16_t new_data = change_bits(GAIN.content, cgain+1, ch.gain);
    wreg(GAIN, 0, &new_data);
}

void ADS130B04::decreaseGain(_channel ch){
    uint16_t cgain = extract_bits(GAIN.content, ch.gain);
    if(cgain == 0b000){
        return; //dont do anything if gain already at min
    }
    uint16_t new_data = change_bits(GAIN.content, cgain+1, ch.gain);
    wreg(GAIN, 0, &new_data);
}

void ADS130B04::setGain(_channel ch, uint16_t gain){
    if(gain > 7){
        return; //dont do anything as max(gain) = 7 = 0b111
    }
    uint16_t new_data = change_bits(GAIN.content, gain, ch.gain);
    wreg(GAIN, 0, &new_data);
}

uint16_t ADS130B04::getGain(_channel ch){
    return extract_bits(GAIN.content, ch.gain);
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

void ADS130B04::enable(_channel ch){
    uint16_t new_data = change_bits(CLOCK.content, 1, ch.en);
    wreg(CLOCK, 0, &new_data);
}

void ADS130B04::disable(_channel ch){
    uint16_t new_data = change_bits(CLOCK.content, 0, ch.en);
    wreg(CLOCK, 0, &new_data);
}

uint16_t ADS130B04::change_bits(uint16_t data, uint16_t new_data, _bitPos pos){
    uint16_t b = ~0x0;
    uint16_t bit_mask = (b) >> (15-(pos.upper-pos.lower)) << pos.lower;
    return (data & (~bit_mask)) | (new_data<<pos.lower);
}

uint16_t ADS130B04::extract_bits(uint16_t data, _bitPos pos){
    uint16_t b = ~0x0;
    uint16_t bit_mask = ~(b >> (15-(pos.upper-pos.lower)) << pos.lower);
    return (data & ~bit_mask) >> pos.lower;
}









