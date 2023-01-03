//
//  Fast_IO_Due.h
//  
//
//  Created by Béla Knopp on 22.12.21.
//

#ifndef Fast_IO_Due_h
#define Fast_IO_Due_h

#include <initializer_list>
#include "Arduino.h"

class Fast_IO_Due{
    
public:
    Fast_IO_Due();
    ~Fast_IO_Due();
    uint16_t read_adc(); //read 12-bit adc value
    uint16_t read_anyadc(uint8_t);
    uint16_t med_anyadc(uint8_t);
    bool calc_norm(size_t);
    void write_dac(uint16_t); //write 12-bit value to dac
    void initialize_adc(std::initializer_list<uint8_t>);
    void initialize_dac();

    int32_t IOnorm;
private:
    void _setup_dac();
    void _setup_adc(uint32_t);
    uint32_t _enable_adc_channels(std::initializer_list<uint8_t>);
    uint8_t _adc_channels[12]={7,6,5,4,3,2,1,0,10,11,12,13}; //array containing the corresponding channels to adcs (A0 -> _adc_channels[0] ...)
    uint16_t _adc_channels_pow[12]; //array containing the powers (2^_adc_channels)
};

#endif /* Fast_IO_Due_h */
