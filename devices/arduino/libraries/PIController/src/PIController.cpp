#include "PIController.h"
#include <Arduino.h>

uint32_t controlFrequency; //frequency of controlling
uint32_t controlDelayMicro; //fitting delay in microseconds

void PIController::clear() {
    _sum = 0;
    _cfg_err = false;
}

void PIController::configure(float kp, float ki, uint8_t AL, uint8_t AR, uint8_t bits) {
    //controlIO IO();
    clear();
    setCoefficients(kp, ki);
    setAddresses(AL, AR);
    setOutputConfig(bits);
}

bool PIController::setCoefficients(float kp, float ki) {
    _p = floatToParam(kp);
    _i = floatToParam(ki);
    active=true;
    return ! _cfg_err;
}

void PIController::setAddresses(uint8_t AL, uint8_t AR){
    ADC_L = AL;
    ADC_R = AR;
}

bool PIController::setOutputConfig(uint16_t bits) {
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
    INTEG_MAX = (int32_t(controlFrequency) * int16_t(_outmax)*PARAM_MULT)-1;
    INTEG_MIN = (int32_t(controlFrequency) * int16_t(_outmin)*PARAM_MULT)+1;
    return ! _cfg_err;
}

uint32_t PIController::floatToParam(float in) {
    if (in > PARAM_MAX || in < 0) {
        _cfg_err = true;
        return 0;
    }

    uint32_t param = PARAM_MULT * in;
    
    if (in != 0 && param == 0) {
        _cfg_err = true;
        return 0;
    }
    
    return param;
}

void PIController::setDelay(uint32_t controlFrequency){
    if (controlFrequency>0){
        controlDelayMicro = uint32_t(1000000/controlFrequency);
    }
    else{
        controlDelayMicro=0;
        setCfgErr();
    }
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
        I = int64_t(_sum)/controlFrequency;
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
