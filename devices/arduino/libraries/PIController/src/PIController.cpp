#include "PIController.h"
#include <Arduino.h>



void PIController::clear() {
    _sum = 0;
    _cfg_err = false;
} 

bool PIController::setCoefficients(float kp, float ki, uint32_t hz) {
    _hz=hz;
    _hz_bits = hzToBits(hz);
    _p = floatToParam(kp);
    _i = floatToParam(ki);
    return ! _cfg_err;
}

bool PIController::setOutputConfig(int bits) {
    // Set output bits
    if (bits > 16 || bits < 1) {
        setCfgErr();
    }
    else {
        if (bits == 16) {
            _outmax = (0xFFFFULL >> (17 - bits)) * PARAM_MULT;
        }
        else{
            _outmax = (0xFFFFULL >> (16 - bits)) * PARAM_MULT;
        }
       //signed output
        _outmin = -((0xFFFFULL >> (17 - bits)) + 1) * PARAM_MULT;
       
    }
    return ! _cfg_err;
}

bool PIController::setOutputRange(int16_t min, int16_t max)
{
    if (min >= max) {
        setCfgErr();
        return ! _cfg_err;
    }
    _outmin = int16_t(min);
    _outmax = int16_t(max);
    INTEG_MAX = (int32_t(_hz) * int16_t(_outmax)*PARAM_MULT)-1;
    INTEG_MIN = (int32_t(_hz) * int16_t(_outmin)*PARAM_MULT)+1;
    return ! _cfg_err;
}

bool PIController::configure(float kp, float ki, uint32_t hz, int bits, uint8_t ADD, uint8_t AL, uint8_t AR) {
    clear();
    active=true;
    ADDRESS=ADD;
    ADC_L=AL;
    ADC_R=AR;
    setCoefficients(kp, ki, hz);
    setOutputConfig(bits);
    return ! _cfg_err;
}

uint32_t PIController::floatToParam(float in) {
    if (in > PARAM_MAX || in < 0) {
        _cfg_err = true;
        return 0;
    }
    uint32_t param = in * PARAM_MULT;
    
    if (in != 0 && param == 0) {
        _cfg_err = true;
        return 0;
    }
    
    return param;
}

uint8_t PIController::hzToBits(uint32_t hz){
    uint32_t v = hz;
    uint8_t l;
    for(uint8_t i=1;i<32,i++){
        if(pow(2,i)>v){
            if(pow(2,i-1)-v<v-pow(2,i)){
                return i-1;
            }
            else{return i;}
        }
    }
    setCfgError();
    return 0;
}

int16_t PIController::step(int16_t sp, int16_t fb) {
    // int16 + int16 = int17
    int16_t err = int16_t(sp) - int16_t(fb);
    int32_t P = 0, I = 0;
    
    if (_p) {
        // uint16 * int16 = int32
        P = uint16_t(_p) * int16_t(err);
    }
    
    if (_i) {
        // int17 * int16 = int33
        _sum += uint16_t(_i) * int16_t(err);
        
        // Limit sum to 32-bit signed value so that it saturates, never overflows.
        if (_sum > INTEG_MAX)
            _sum = INTEG_MAX;
        else if (_sum < INTEG_MIN)
            _sum = INTEG_MIN;
        
        // int32
        I = int64_t(_sum)>>_hz_bits;
        //Serial.println(I);
    }
    
    // int32 (P) + int32 (I)= int34
    int32_t out = int32_t(P) + int32_t(I);
    
    // Remove the integer scaling factor.
    int16_t rval = out>>PARAM_SHIFT;
    
    // Make the output saturate
    if (rval > _outmax)
        rval = _outmax;
    else if (rval < _outmin)
        rval = _outmin;
    
    return rval;
}

void PIController::setCfgErr() {
    _cfg_err = true;
    _p = _i = 0;
}
