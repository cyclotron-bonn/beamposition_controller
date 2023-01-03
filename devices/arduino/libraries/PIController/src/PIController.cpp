#include "PIController.h"
#include <Arduino.h>


IOController::~IOController(){
}

void IOController::setup(uint8_t ADD, uint8_t A0, uint8_t A1){
    DAC_ADDRESS = ADD;
    ADC_A0 = A0;
    ADC_A1 = A1;
}


PIController::~PIController() {
}

void PIController::clear() {
    _sum = 0;
    _cfg_err = false;
} 

bool PIController::setCoefficients(float kp, float ki, uint32_t hz) {
    _hz=hz;
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

bool PIController::configure(float kp, float ki, float hz, int bits) {
    clear();
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
        I = int64_t(_sum)/uint32_t(_hz);
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
