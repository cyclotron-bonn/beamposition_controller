#ifndef PIController_H
#define PIController_H

#include <stdint.h>

#define PARAM_SHIFT  8
#define PARAM_BITS   16
#define PARAM_MAX    (((0x1ULL << PARAM_BITS)-1) >> PARAM_SHIFT)
#define PARAM_MULT   (((0x1ULL << PARAM_BITS)) >> (PARAM_BITS - PARAM_SHIFT))

/*
 A fixed point PID controller with a 32-bit internal calculation pipeline.
 */

class PIController {
    
public:
    PIController(){
        clear();
    }
    
    PIController(float kp, float ki, float hz, int bits=16){
        configure(kp, ki, hz, bits);
    }
    
    ~PIController();
    
    bool setCoefficients(float kp, float ki, uint32_t hz);
    bool setOutputConfig(int bits);
    bool setOutputRange(int16_t min, int16_t max);
    void clear();
    bool configure(float kp, float ki, float hz, int bits=16);
    int16_t step(int16_t sp, int16_t fb);
    
    bool err() {
        return _cfg_err;
    }
    
    uint32_t _p;
    uint32_t _i;
    int32_t _hz;

    bool active;

    uint8_t I2C_ADDRESS;

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
