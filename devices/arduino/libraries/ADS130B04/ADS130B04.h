#ifndef ADS130B04_h
#define ADS130B04_h

#include <SPI.h>

struct _register{   
    uint8_t addr;
    uint16_t content;
    _register(uint8_t address) : addr(address){};
};
    
struct _bitPos{
    uint8_t lower;
    uint8_t upper;
    _bitPos(uint8_t l, uint8_t u) : lower(l), upper(u){};
};

struct _channel{
    _register CFG;
    _bitPos gain;
    _bitPos en;
    uint8_t status_bit;
    bool enable;
    uint16_t value;
    uint16_t last_value;
    _channel(_register cfg, _bitPos g, uint8_t sbit) : CFG(cfg), gain(g), status_bit(sbit){};
};

class ADS130B04{
private:
    const SPISettings HighResolutionMode = SPISettings(8192000, MSBFIRST, SPI_MODE1);
    const SPISettings LowPowerMode = SPISettings(4092000, MSBFIRST, SPI_MODE1);
    const SPISettings VeryLowPowerMode = SPISettings(2048000, MSBFIRST, SPI_MODE1);
    SPISettings SPIsetting;

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

    uint16_t genCRC(uint16_t);
    void transADC();
    bool error_a_n(uint16_t, uint16_t);
    void setSPIsetting(uint8_t);
    void completeTransfer16(uint16_t);
    uint16_t change_bits(uint16_t, uint16_t, _bitPos);
    uint16_t extract_bits(uint16_t, _bitPos);

    /*device settings and status indicators (read-only registers)
    detailed description in datasheet: https://www.ti.com/lit/ds/symlink/ads130b04-q1.pdf?HQS=dis-mous-null-mousermode-dsf-pf-null-wwe&ts=1695742377424&ref_url=https%253A%252F%252Fwww.mouser.de%252F*/

public:
    uint8_t CS;
    uint8_t EOC;

    _register NREG = _register(0xFF);
    _register ID = _register(0x00);
    _register STATUS = _register(0x01);
    _register MODE = _register(0x02);
    _register CLOCK = _register(0x03);
    _register GAIN = _register(0x04);
    _register GLOBAL_CHOP_CFG = _register(0x06);
    _register REG_CRC = _register(0x3E);  

    _register CH0_CFG = _register(0x09);
    _bitPos CH0_GAIN = _bitPos(0,2);

    _register CH1_CFG = _register(0x0E);
    _bitPos CH1_GAIN = _bitPos(4,6);

    _register CH2_CFG = _register(0x13);
    _bitPos CH2_GAIN = _bitPos(8,10);

    _register CH3_CFG = _register(0x18);
    _bitPos CH3_GAIN = _bitPos(12,14);


    _channel CH0 = _channel(CH0_CFG, CH0_GAIN, 0);
    _channel CH1 = _channel(CH1_CFG, CH1_GAIN, 1);
    _channel CH2 = _channel(CH2_CFG, CH2_GAIN, 2);
    _channel CH3 = _channel(CH3_CFG, CH3_GAIN, 3);

    static constexpr size_t n_regs = 11;
    _register regs[n_regs] = {ID, STATUS, MODE, CLOCK, GAIN, GLOBAL_CHOP_CFG, CH0_CFG, CH1_CFG, CH2_CFG, CH3_CFG, REG_CRC};

    static constexpr size_t n_adc = 4;
    _channel channels[n_adc] = {CH0, CH1, CH2, CH3};

public:
    //chip related
    ADS130B04(uint8_t, uint8_t);
    ~ADS130B04();

    void setPowerMode(uint8_t);
    void updateChannels();
    void setCRC();
    void getCRC();

    void null();
    void reset();
    void standby();
    void wakeup();
    void lock();
    void unlock();
    //register related
    void rreg(_register, uint8_t);
    void wreg(_register, uint8_t, uint16_t*);

    //channel related
    void enable(_channel);
    void disable(_channel);
    void setGain(_channel, uint16_t);
    uint16_t getGain(_channel);
    void increaseGain(_channel);
    void decreaseGain(_channel);
};


#endif 
