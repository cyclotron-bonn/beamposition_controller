#ifndef PIController_H
#define PIController_H

#include <stdint.h>

#define PARAM_SHIFT  8
#define PARAM_BITS   16
//PARAM_MAX: 1 shifted PARAM_BITS left -> 1 0000 0000 0000 0000 -> -1 -> 1111 1111 1111 1111 -> shifted PARAM_SHiFT right -> 1111 1111
//maximum of float value (255) -> is multiplied with PARAM_MULT to have a maximum of 1111 1111 0000 0000 = 65280 (2^16 = 65536)
#define PARAM_MAX    (((0x1ULL << PARAM_BITS)-1) >> PARAM_SHIFT)
//PARAM_MULT: 1 shifted PARAM_BITS left -> 1 0000 0000 0000 0000 -> shifted PARAM_BITS-PARAM_SHIFT=8 right -> 1 0000 0000
#define PARAM_MULT   (((0x1ULL << PARAM_BITS)) >> (PARAM_BITS - PARAM_SHIFT))

/*
A class containing the IO-information
*/
class IOController{
public:
    IOController(){

    }

    IOController(uint8_t ADD, uint8_t A0, uint8_t A1){
        setup(ADD, A0, A1);
    }

    ~IOController();
    bool setup(uint8_t ADD, uint8_t A0, uint8_t A1);

    uint8_t DAC_ADDRESS;
    uint8_t ADC_A0;
    uint8_t ADC_A1;
}

/*
 A fixed point PID controller with a 32-bit internal calculation pipeline.
 */
class PIController:public IOController{
    
public:
    PIController(){
        clear();
    }
    
    PIController(float kp, float ki, float hz, int bits=16, uint8_t ADD, uint8_t A0, uint8_t A1){
        IOController io(ADD, A0, A1);
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
