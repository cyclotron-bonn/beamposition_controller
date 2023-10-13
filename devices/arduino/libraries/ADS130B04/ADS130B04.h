#ifndef ADS130B04_h
#define ADS130B04_h

#include <SPI.h>

class ADS130B04;

struct _register;

class ADS130B04{
private:
    const uint16_t adress_lim = 0b111111;
    const uint16_t number_lim = 0b1111111;

    
    const uint16_t cNull = 0b00;
    const uint16_t cReset = 0b10001;
    const uint16_t cStandby = 0b100010;
    const uint16_t cWakeup = 0b110011;
    const uint16_t cLock = 0b10101010101;
    const uint16_t cUnlock = 0b11001010101;

    uint8_t crc_mode = 0;
    const uint16_t ccitt_crc_poly = 0b0001000000100001;
    const uint16_t ansi_crc_poly = 0b1000000000000101;

    void query(uint16_t, uint8_t);
    uint16_t genCRC(uint16_t);
    void transADC();
    bool error_a_n(uint16_t, uint16_t);
    void setSPIsetting(uint8_t);

    /*device settings and status indicators (read-only registers)
    detailed description in datasheet: https://www.ti.com/lit/ds/symlink/ads130b04-q1.pdf?HQS=dis-mous-null-mousermode-dsf-pf-null-wwe&ts=1695742377424&ref_url=https%253A%252F%252Fwww.mouser.de%252F*/
    
     
public:
    uint16_t CHx_VAL[4];

    ADS130B04(uint8_t, uint8_t);
    ~ADS130B04();
    void SPIsetup(bool doubleslit);
    
    void setPowerMode(uint8_t);
   
    void increaseGain(uint8_t channel);
    void decreaseGain(uint8_t channel);
    void setGain(uint8_t channel, uint8_t gain);
    void getGain(uint8_t channel);
    
   
    void getADCVal();
    void readRegister();
    void setCRC();
    void getCRC();
    void enableADC();
    void disableADC();

    void null();
    void reset();
    void standby();
    void wakeup();
    void lock();
    void unlock();
    int rreg(struct _register, uint16_t);
    void wreg(uint16_t, uint16_t);

};

#endif 
