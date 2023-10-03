#ifndef PIController_H
#define PIController_H

#include <stdint.h>

#define PARAM_SHIFT  4
#define PARAM_BITS   8
#define PARAM_MAX    (((0x1ULL << PARAM_BITS)-1) >> PARAM_SHIFT)
#define PARAM_MULT  (((0x1ULL << PARAM_BITS)) >> (PARAM_BITS - PARAM_SHIFT))


/*
 A fixed point PID controller with a 32-bit internal calculation pipeline.
 */

extern uint32_t controlFrequency; //frequency of controlling
extern uint32_t controlDelayMicro;
extern uint8_t n_controllers;

class PIController{
    
public:
    bool setCoefficients(float kp, float ki, bool act);
    void setAddresses(uint8_t AL, uint8_t AR, uint8_t DA);
    bool setOutputConfig(uint16_t bits);
    bool setOutputRange(int16_t min, int16_t max);
    void clear();
    void configure(float, float, uint8_t, uint8_t, uint8_t, uint8_t);
    void setDelay(uint32_t);

    int16_t step(int16_t fb);

    bool err() {
        return _cfg_err;
    }
    
    uint8_t _p;
    uint8_t _i;
    int32_t _sum;
    bool active;
    //DAC-address, which ADCs to use and wether the controller is active
    
    struct{
        uint8_t adc_left;
        uint8_t adc_right;
        uint8_t dac;
    }adresses;

private:
    uint8_t floatToParam(float);
    float paramToFloat(uint32_t);
    void setCfgErr();
    
private:
    // Configuration
    int64_t _outmax, _outmin;
    int64_t INTEG_MAX, INTEG_MIN;
    bool _cfg_err;
};

#endif
