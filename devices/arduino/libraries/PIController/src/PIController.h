#ifndef PIController_H
#define PIController_H

#include <stdint.h>

#define PARAM_SHIFT  8
#define PARAM_BITS   16
//PARAM_MAX: 1 shifted PARAM_BITS left -> 1 0000 0000 0000 0000 -> -1 -> 1111 1111 1111 1111 -> shifted PARAM_SHiFT right -> 1111 1111
//maximum of float value (255) -> is multiplied with PARAM_MULT to have a maximum of 1111 1111 0000 0000 = 65280 (2^16 = 65536)
#define PARAM_MAX    (((0x1ULL << PARAM_BITS)-1) >> PARAM_SHIFT)
//PARAM_MULT: 1 shifted PARAM_BITS left -> 1 0000 0000 0000 0000 -> shifted PARAM_BITS-PARAM_SHIFT=8 right -> 1 0000 0000 is 2^9
#define PARAM_MULT  (((0x1ULL << PARAM_BITS)) >> (PARAM_BITS - PARAM_SHIFT))


/*
 A fixed point PID controller with a 32-bit internal calculation pipeline.
 */

extern uint32_t controlFrequency; //frequency of controlling
extern uint32_t controlDelayMicro;

class PIController{
    
public:
    bool setCoefficients(float kp, float ki);
    void setAddresses(uint8_t AL, uint8_t AR);
    bool setOutputConfig(uint16_t bits);
    bool setOutputRange(int16_t min, int16_t max);
    void clear();
    void configure(float, float, uint8_t, uint8_t, uint8_t);
    void setDelay(uint32_t);

    int16_t step(int16_t sp, int16_t fb);

    bool err() {
        return _cfg_err;
    }
    
    uint32_t _p;
    uint32_t _i;

    //DAC-address, which ADCs to use and wether the controller is active
    bool active;
    uint8_t ADC_L;
    uint8_t ADC_R;

private:
    uint32_t floatToParam(float);
    float paramToFloat(uint32_t);
    void setCfgErr();
    
private:
    // Configuration
    int64_t _outmax, _outmin;
    int64_t INTEG_MAX, INTEG_MIN;
    bool _cfg_err;
    
    // State
    int64_t _sum;
};

#endif
