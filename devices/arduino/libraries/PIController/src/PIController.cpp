#include "PIController.h"
#include <Arduino.h>

uint32_t controlFrequency=1; //frequency of controlling
uint32_t controlDelayMicro=1000000; //fitting delay in microseconds
uint8_t n_controllers = 0;

void PIController::clear() {
    
    _sum = 0;
    _cfg_err = false;
}

void PIController::configure(float kp, float ki, uint8_t AL, uint8_t AR, uint8_t DA, uint8_t bits=16) {
    clear();
    setCoefficients(kp, ki, false);
    setAddresses(AL, AR, DA);
    setOutputRange(-32568, 32568);
    controlFrequency=1;
    setDelay(controlFrequency);
}

bool PIController::setCoefficients(float kp, float ki, bool act) {
    _p = floatToParam(kp);
    _i = floatToParam(ki);
    active = act;
    return ! _cfg_err;
}

void PIController::setAddresses(uint8_t AL, uint8_t AR, uint8_t DA){
    adresses.adc_left = AL;
    adresses.adc_right = AR;
    adresses.dac = DA;
}

bool PIController::setOutputConfig(uint16_t bits) {
    // Set output bits
    if (bits > 16 || bits < 1) {
        setCfgErr();
    }
    _outmax = (0x1ULL << bits) - 1;
    _outmin = - (0x1ULL << bits) + 1;
    INTEG_MAX = (int32_t(controlFrequency) * int16_t(_outmax) * PARAM_MULT)-1;
    INTEG_MIN = (int32_t(controlFrequency) * int16_t(_outmin) * PARAM_MULT)+1;
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

uint8_t PIController::floatToParam(float in) {
    if (in > PARAM_MAX || in < 0) {
        _cfg_err = true;
        return 0;
    }

    uint8_t param = PARAM_MULT * in;
    
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

int16_t PIController::step(int16_t fb) {
    // int16 + int16 = int17
    int32_t err = int32_t(-fb); //12-bit maximum
    int32_t P = 0, I = 0;
    
    if (_p) {
        // uint8 * int12 < int32
        P = uint16_t(_p) * int32_t(err); 
    }
    
    if (_i) {
        // int17 * int16 = int33
        _sum += uint16_t(_i) * int32_t(err);
        //Limit sum to 32-bit signed value so that it saturates, never overflows.
        if (_sum > INTEG_MAX)
            _sum = INTEG_MAX;
        else if (_sum < INTEG_MIN)
            _sum = INTEG_MIN;
        
        //int32
        I = _sum;///controlFrequency;
        Serial.print("I:");
        Serial.println(I);
    }
    
    // int32 (P) + int32 (I)= int34
    int64_t out = P + I;
    
    // Remove the integer scaling factor.
    int16_t rval = (out>>PARAM_SHIFT);
    
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
