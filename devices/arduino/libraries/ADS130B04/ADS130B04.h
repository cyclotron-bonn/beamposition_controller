#ifndef ADS130B04_h
#define ADS130B04_h

#include <SPI.h>

class ADS130B04{
private:
    uint16_t ADDRESS_LIM = 0b111111;
    uint16_t NUMBER_LIM = 0b1111111;
    uint8_t n_reg;
    uint16_t command;
    uint16_t crc;
    uint8_t crc_mode = 0;
    uint16_t ccitt_crc_poly = 0b0001000000100001;
    uint16_t ansi_crc_poly = 0b1000000000000101;

    void query();
    uint16_t gen_crc(uint16_t);
    void transADC();
    bool error_a_n(uint16_t, uint16_t);
    void setSPIsetting(uint8_t);


    /*device settings and status indicators (read-only registers)
    detailed description in datasheet: https://www.ti.com/lit/ds/symlink/ads130b04-q1.pdf?HQS=dis-mous-null-mousermode-dsf-pf-null-wwe&ts=1695742377424&ref_url=https%253A%252F%252Fwww.mouser.de%252F*/

    struct ID{
        uint16_t addr = 0x00;
        uint16_t content;
    };

    struct STATUS{
        uint16_t addr = 0x01;
        uint16_t content;
    };

    struct MODE{
        uint16_t addr = 0x02;
        uint16_t content;
    };

    struct CLOCK{
        uint16_t addr = 0x03;
        uint16_t content;
    };

    struct GAIN{
        uint16_t addr = 0x04;
        uint16_t content;
    };

    struct GLOBAL_CHOP_CFG{
        uint16_t addr = 0x06;
        uint16_t content;
    };

    struct Chx_CFG{
        uint16_t addr[4] = {0x09, 0x0E, 0x13, 0x18};
        uint16_t content;
    };

    struct REG_CRC{
        uint16_t addr = 0x3E;
        uint16_t content;
    };
    
     
public:
    ADS130B04(uint8_t, uint8_t);
    ~ADS130B04();
    void SPIsetup(bool doubleslit);
    
    void setPowerMode(uint8_t);
    struct GAIN{
        void increase(uint8_t channel);
        void decrease(uint8_t channel);
        void set(uint8_t channel, uint8_t gain);
        void get(uint8_t channel);
    };
    
   
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
    int rreg(uint16_t, uint16_t);
    void wreg(uint16_t, uint16_t);
    

 

   

};

#endif 
