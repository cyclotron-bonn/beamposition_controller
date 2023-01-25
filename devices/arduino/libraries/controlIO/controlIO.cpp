//
//  controlIO.cpp
//  
//
//  Created by Béla Knopp on 22.12.21.
//

#include "controlIO.h"


void initDAC(){

}

void initADC(){

}

controlIO::controlIO(){}
controlIO::~controlIO(){}

int16_t controlIO::calcNorm(uint8_t channel1, uint8_t channel2){
    /*
    * Read two input channels A0 und A1 'nc' number of times, add respective values up and store them in _UA0 and _UA1
    */
    uint8_t nc = 8;
    uint16_t _L=0, _R=0;
    int16_t limit = nc<<bit_control_limit;

    for(uint8_t c=0; c<nc;c++){ 
        _L += analogRead(channel1);
        _R += analogRead(channel2);
    }
    /*
    * Calculate the control variable which equals: (_UA0 - _UA1)/(_UA0 + _UA1).
    * As the control variable IOnorm is an int and the calculation returnes a value between -1 and 1, the result is multiplied with 2047 to get a ~12-bit value.
    * A feature of this implementation is the inclusion of a sample-and-hold condition.
    * If the sum of the voltages is high enough (> nc * limit) the function calculates the new value of the control variable and returns true.
    * If this is not the case (else) the function returns false.
    * True and false can turn the controller on and off for one loop.
    */
   int16_t DIFF = _L- _R;
   int16_t SUM = _L + _R;
   float INV = 1./SUM;
   int16_t normINV = 2047*INV;
    if(SUM>limit){ 
        return DIFF*normINV;// (int16_t, so no decimal places) 2047 is chosen because of 12-bit dac output range (2*2047+1=4095~4096)
    }
    return 0;
}

void controlIO::writeOutput(uint16_t dac_value){
    
}

uint16_t controlIO::analogRead(uint8_t channel){
    chooseChannel(channel);
    return readBuffer();
}

uint16_t controlIO::readBuffer(){
    uint16_t bits_cl = ADCL;
    uint16_t bits_ch = ADCH<<8;
    return bits_ch+bits_cl;
}

void controlIO::chooseChannel(uint8_t channel){
    uint8_t channel_bin = 0xA0 - channel; //input can be A0-A7 as hexadecimal int, is converted to 0-7 by subtracting A as int
    uint8_t reference_adjust = 0b01000000; //bit 7:6 (01) reference: voltage reference - bit 5:right adjust result
    ADMUX = reference_adjust | channel_bin;
}