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

// uint16_t controlIO::readInput(uint8_t _channel){
//     // uint16_t _adc_isr = _adc_channels_pow[_channel]; //calculate channel that should be read in binary
//     // uint16_t _adc_ch = _adc_channels[_channel]; //channel
//     // while((ADC->ADC_ISR & _adc_isr) == 0); //Interrupt Status Register; conversion complete?
//     // return ADC->ADC_CDR[_adc_ch]; //get value of channel
//     return 0;
// }


int16_t controlIO::calcNorm(){
//     /*
//     * Read two input channels A0 und A1 'nc' number of times, add respective values up and store them in _UA0 and _UA1
//     */
//     uint16_t _UA0=0, _UA1=0;
//     uint16_t limit=nc>>bit_control_limit;
//     for(uint8_t c=0; c<nc;c++){ 
//         while(!ADC_ISR_EOC7);
//         _UA0+=ADC->ADC_CDR[7];
//         while(!ADC_ISR_EOC6);
//         _UA1+=ADC->ADC_CDR[6];
//     }
//     /*
//     * Calculate the control variable which equals: (_UA0 - _UA1)/(_UA0 + _UA1).
//     * As the control variable IOnorm is an int and the calculation returnes a value between -1 and 1, the result is multiplied with 2047 to get a ~12-bit value.
//     * A feature of this implementation is the inclusion of a sample-and-hold condition.
//     * If the sum of the voltages is high enough (> nc * limit) the function calculates the new value of the control variable and returns true.
//     * If this is not the case (else) the function returns false.
//     * True and false can turn the controller on and off for one loop.
//     */
//    int16_t DIFF = _UA0- _UA1;
//    int16_t SUM = _UA0 + _UA1;
//    float INV = 1./SUM;
//    uint16_t normINV = 2047*INV;
//     if(SUM>limit){ 
//         return (DIFF*normINV);// (int16_t, so no decimal places) 2047 is chosen because of 12-bit dac output range (2*2047+1=4095~4096)
        
//     }
    return 0;
}

void controlIO::writeOutput(uint16_t dac_value){
    // while(!DACC_ISR_TXRDY); //Conversion complete?
    // DACC->DACC_CDR = dac_value; //write new value
}

uint16_t controlIO::analogRead(uint16_t channel){
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