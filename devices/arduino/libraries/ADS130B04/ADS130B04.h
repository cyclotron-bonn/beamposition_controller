#ifndef ADS130B04_h
#define ADS130B04_h

#include <SPI.h>

class ADS130B04{
private:
    uint16_t ADDRESS_LIM = 0b111111;
    uint16_t NUMBER_LIM = 0b1111111;

    void query();
    uint16_t gen_crc(uint16_t);
    void transADC();
    bool error_a_n(uint16_t, uint16_t);

public:
    ADS130B04(uint8_t, uint8_t);
    ~ADS130B04();
    void SPIsetup(bool doubleslit);
    void setSPIsetting(uint8_t);
    void setPowerMode(uint8_t);
    void null();
    void reset();
    void standby();
    void wakeup();
    void lock();
    void unlock();
    int rreg(uint16_t, uint16_t);
    void wreg(uint16_t, uint16_t);

    uint8_t n_reg;
    uint16_t command;
    uint16_t crc;
    uint8_t crc_mode = 0;
    uint16_t ccitt_crc_poly = 0b0001000000100001;
    uint16_t ansi_crc_poly = 0b1000000000000101;

    /*device settings and status indicators (read-only registers)
    detailed description in datasheet: https://www.ti.com/lit/ds/symlink/ads130b04-q1.pdf?HQS=dis-mous-null-mousermode-dsf-pf-null-wwe&ts=1695742377424&ref_url=https%253A%252F%252Fwww.mouser.de%252F*/
    uint16_t ID = 0x00;
    uint16_t STATUS = 0x01;
    //global settings across channels
    uint16_t MODE = 0x02;
    uint16_t CLOCK = 0x03;
    uint16_t GAIN = 0x04;
    uint16_t GLOBAL_CHOP_CFG = 0x06;
    //channel-specific settings
    uint16_t CHx_CFG[4] = {0x09, 0x0E, 0x13, 0x18};
    //register map crc register
    uint16_t REG_CRC = 0x3E;

};

#endif 
