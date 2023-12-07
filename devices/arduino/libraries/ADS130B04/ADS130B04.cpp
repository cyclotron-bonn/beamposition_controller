#include "ADS130B04.h"

ADS130B04::ADS130B04(uint8_t CS_PIN, uint8_t EOC_PIN){
    //set stuff
    pinMode(CS_PIN, OUTPUT);
    CS = CS_PIN;
    EOC = EOC_PIN;
    SPIsetting = VeryLowPowerMode;
   
}

ADS130B04::~ADS130B04(){
    reset();
}

void ADS130B04::null(){
    completeTransfer(0x00);
}

void ADS130B04::reset(){
    completeTransfer(cReset);
    setup();
}

void ADS130B04::standby(){
    completeTransfer(cStandby);
}

void ADS130B04::wakeup(){
    completeTransfer(cWakeup);
}

void ADS130B04::lock(){
    completeTransfer(cLock);
}

void ADS130B04::unlock(){
    completeTransfer(cUnlock);
}

uint16_t ADS130B04::transfer(uint16_t command){
    uint8_t command0 = command >> 8;
    uint8_t command1 = (command << 8) >> 8;
    uint16_t response0 = SPI.transfer(command0);
    uint16_t response1 = SPI.transfer(command1);
    pad();
    return (response0 << 8) + response1 ;
}

uint16_t ADS130B04::completeTransfer(uint16_t command){
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    uint16_t response = 0;
    Serial.print("mode:");
    Serial.println(mode);
    response = transfer(command);
    pad();
    uint16_t crc_code = genCRC(command);
    Serial.print("crc");
    Serial.println(crc_code);
    transfer(crc_code);
    pad();
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
    return response;
}

void ADS130B04::pad(){
    uint8_t word_length = extract_bits(STATUS.content, _bitPos(9,8));
    switch (word_length){
        case 0b00:
            break;
        case 0b01:
            SPI.transfer(0x00);
            break;
        case 0b10: 
            SPI.transfer(0x00);
            SPI.transfer(0x00);
            break;
        default:
            break;
    }
}

void ADS130B04::setup(){
     //initialize communication with adc and manipulate settings to fit this script
    STATUS.content = completeTransfer(0x0000);
    Serial.print("Status:");
    Serial.println(STATUS.content, BIN);
    Serial.print("WL0:");
    Serial.println(word_length, BIN);
    Serial.print("WL1:");
    Serial.println(word_length);
    completeTransfer(cUnlock);
    setWordLength16();
}

void ADS130B04::setWordLength16(uint8_t mode){ //mode = current word length
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    uint16_t ad_command = 0b1010000000000000 + (MODE.addr<<7);
    _bitPos wl = _bitPos(9,8); //position of word length
    uint16_t data = change_bits(MODE.content, 0b00, wl);
    //communicate with 24-bit where first 16-bits hold information
    transfer(ad_command);
    transfer(data);
    Serial.print("mode:");
    Serial.println(mode);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}


void ADS130B04::rreg(_register s_reg, uint8_t n=0){
    if(error_a_n(s_reg.addr,n)){
        return;
    }
    uint16_t command = 0b1010000000000000 + (s_reg.addr<<7) + n;
    SPI.beginTransaction(HighResolutionMode);
    digitalWrite(CS, LOW);
    STATUS.content = transfer(command);
    Serial.print("Status:");
    Serial.println(STATUS.content, BIN);
    //update all channels as response from earlier action (prior to this execution of this function (or other read function))
    for(uint8_t i=0; i<4; i++){
        if(i==0){channels[i].value = transfer(genCRC(command));}
        else channels[i].value = transfer(0x00);  
    }
    uint16_t crc = transfer(0x00);  
    digitalWrite(CS, HIGH);

    //find the position of the starting register in the array of registers
    uint8_t n0 = 0;
    for(uint8_t i=0; i<n+1;i++){
        if(s_reg.addr == regs[i].addr){
            n0 = i;
        }
    }
    //update all registers
    digitalWrite(CS, LOW);
    for (uint8_t i = n0; i < n+1; i++){
       regs[i].content = transfer(0x00);
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
    STATUS.content = transfer(command);
    //on every new data given the channel value is returned for the first 4 values.
    if(n<n_adc){
        for(uint8_t i = 0; i<n; i++){
            channels[i].value = transfer(new_data[i]);
        }
        for(uint8_t i = n; i<n_adc-2; i++){
            channels[i].value = transfer(0x00);
        }
        channels[n_adc-1].value = transfer(genCRC(command));
    }
    if(n==n_adc){
        for (size_t i = 0; i < n; i++){
            channels[i].value = transfer(new_data[i]);
        }
        transfer(genCRC(command));
    }
    if(n>n_adc){
        for (size_t i = 0; i < n_adc; i++){
            channels[i].value = transfer(new_data[i]);
        }
        for (size_t i = n_adc; i < n; i++)
        {
            transfer(new_data[i]);
        }
        transfer(genCRC(command));
    }
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS130B04::updateChannels(){
    //usiing the null command one can retrive the channel data
    SPI.beginTransaction(SPIsetting);
    digitalWrite(CS, LOW);
    STATUS.content = transfer(0x00);
    for(uint8_t i=0; i<4; i++){
        if(i==0){channels[i].value = transfer(genCRC(0x00));}
        else channels[i].value = transfer(0x00);  
    }
    uint16_t crc = transfer(0x00);  
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
    transADC();
}

void ADS130B04::increaseGain(_channel ch){
    //increase the gain of a specific channel
    uint16_t cgain = extract_bits(GAIN.content, ch.gain);
    if(cgain == 0b111){
        return; //dont do anything if gain already at max
    }
    uint16_t new_data = change_bits(GAIN.content, cgain+1, ch.gain);
    wreg(GAIN, 0, &new_data);
}

void ADS130B04::decreaseGain(_channel ch){
    //decrease gain of specific channel
    uint16_t cgain = extract_bits(GAIN.content, ch.gain);
    if(cgain == 0b000){
        return; //dont do anything if gain already at min
    }
    uint16_t new_data = change_bits(GAIN.content, cgain+1, ch.gain);
    wreg(GAIN, 0, &new_data);
}

void ADS130B04::setGain(_channel ch, uint16_t gain){
    //set gain of specific channel
    if(gain > 7){
        return; //dont do anything as max(gain) = 7 = 0b111
    }
    uint16_t new_data = change_bits(GAIN.content, gain, ch.gain);
    wreg(GAIN, 0, &new_data);
}

uint16_t ADS130B04::getGain(_channel ch){
    //get gain of specific channel
    return extract_bits(GAIN.content, ch.gain);
}

void ADS130B04::setSPIsetting(uint8_t mode){
    //set spi setting on device
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
    //set power mode on device
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
    //gen crc command depending on poly used
    if(crc_mode == 1){
        return command^ccitt_crc_poly;
    }
    if(crc_mode == 2){
        return command^ansi_crc_poly;
    }
    return 0;
    
}

void ADS130B04::transADC(){
    //translate unsigned to signed adc values using given formula
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
    uint16_t bit_mask = (b) >> (15-(pos.upper-pos.lower)) << pos.lower; //well, i mean.. this works. just think about it
    return (data & (~bit_mask)) | (new_data<<pos.lower);
}

uint16_t ADS130B04::extract_bits(uint16_t data, _bitPos pos){
    uint16_t b = ~0x0;
    uint16_t bit_mask = ~(b >> (15-(pos.upper-pos.lower)) << pos.lower); //same as above
    return (data & ~bit_mask) >> pos.lower;
}


