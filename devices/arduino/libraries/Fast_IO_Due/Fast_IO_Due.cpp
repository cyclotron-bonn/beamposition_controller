//
//  Fast_IO_Due.cpp
//  
//
//  Created by Béla Knopp on 22.12.21.
//

#include "Fast_IO_Due.h"

Fast_IO_Due::Fast_IO_Due(){
    size_t ci;
    for(ci=0; ci<12;ci++){
        _adc_channels_pow[ci]=pow(2,_adc_channels[ci]); //array with powers so they dont have to be calculated each time.
    }
}
Fast_IO_Due::~Fast_IO_Due(){}

void Fast_IO_Due::_setup_adc(uint32_t adc_cher){
    PMC->PMC_PCER1 |= PMC_PCER1_PID37;  // ADC power on
    ADC->ADC_CR = ADC_CR_SWRST; // simulating Hardware-Reset of ADC
    ADC->ADC_CHDR=0xFFFF; //Disable all channels: Channel-Disable-Register (easier to write in hexcode than to disable every single channel manually)
    ADC->ADC_IDR = 0xFFFFFFFF; //Interrupt Disable Register; Disable any interrupt on all channels
    ADC->ADC_COR=0x0; //Channel Offset Register; Disable offsets and differential inputs
    ADC->ADC_CGR = 0x0;   // All gains set to x1 for single ended inputs Channel Gain Register
    ADC->ADC_IER=adc_cher; // Interrupt Enable Register; Enable end-of-conversion interrupt on enabled channels
    ADC->ADC_MR=ADC_MR_TRGEN_DIS //Disable hardware trigger
    | ADC_MR_LOWRES_BITS_12 //Resolution 12-bit
    | ADC_MR_SLEEP_NORMAL //ADC Core and reference voltage circuitry are kept on between conversions
    | ADC_MR_FREERUN_ON //Never wait for trigger (may overwrite trgen_dis setting)
    | ADC_MR_PRESCAL (0) //ADCClock = MCK / ( (PRESCAL+1) * 2 ) (MCK=Masterclock)
    | ADC_MR_STARTUP_SUT0 //Startup-Time = x periods of adcclock (0,8,16,24,64,80..)
    | ADC_MR_SETTLING_AST3 //Analog settling time = 3 periods of adcclock (minimum) (3,5,9,17)
    | ADC_MR_TRACKTIM (0) //Tracking Time = (TRACKTIM + 1) * ADCClock periods.
    | ADC_MR_TRANSFER (0); //Transfer Period = (TRANSFER * 2 + 3) ADCClock periods.
    ADC->ADC_CHER = adc_cher; // enable channels
    ADC->ADC_CR=ADC_CR_START; //Start analog to digital conversion
}

void Fast_IO_Due::_setup_dac(){
    PMC->PMC_PCER1|= PMC_PCER1_PID38; //DACC power on
    DACC->DACC_CR = DACC_CR_SWRST;    // Reset DACC
    DACC->DACC_MR = DACC_MR_TRGEN_DIS                   // Free running mode
    | DACC_MR_USER_SEL_CHANNEL0         // select channel 1
    | DACC_MR_REFRESH (1) //Refresh Period = 1024*REFRESH/DACC Clock 0 doesnt work
    | DACC_MR_STARTUP_8 //Startup time of _x periods of dac. 0 doesnt work
    | DACC_MR_MAXS; //Max speed mode.
    
    NVIC_EnableIRQ(DACC_IRQn); // NVIC=Nesten Void Interrupt Controller; NVIC is an on-chip controller that provides fast and low latency response to interrupt-driven events
    DACC->DACC_CHER = DACC_CHER_CH0;      // enable channel 0 = DAC0 (DAC2)
}

void Fast_IO_Due::initialize_adc(std::initializer_list<uint8_t> _channels = {0}){
    _setup_adc(_enable_adc_channels(_channels)); //setup and enable channels
}

void Fast_IO_Due::initialize_dac(){
    _setup_dac(); //settings
}

uint16_t Fast_IO_Due::read_adc(){
    while(!ADC_ISR_EOC7); //wait for channel to be ready
    return ADC->ADC_CDR[7]; //return value from corresponding register
}

uint16_t Fast_IO_Due::read_anyadc(uint8_t _channel){
    uint16_t _adc_isr = _adc_channels_pow[_channel]; //calculate channel that should be read in binary
    uint16_t _adc_ch = _adc_channels[_channel]; //channel
    while((ADC->ADC_ISR & _adc_isr) == 0); //Interrupt Status Register; conversion complete?
    return ADC->ADC_CDR[_adc_ch]; //get value of channel
}

uint16_t Fast_IO_Due::med_anyadc(uint8_t channel){
    //returns the arithmetic average of any channel over 10 reads
    uint32_t sum = 0;
    for(uint8_t i = 0; i<10; i++){
        sum += read_anyadc(channel);
    }
    return sum/10;
}
bool Fast_IO_Due::calc_norm(size_t nc=10){
    /*
    * Read two input channels A0 und A1 'nc' number of times, add respective values up and store them in _UA0 and _UA1
    */
    int32_t _UA0=0, _UA1=0;
    for(uint8_t c=0; c<nc;c++){ 
        while(!ADC_ISR_EOC7);
        _UA0+=ADC->ADC_CDR[7];
        while(!ADC_ISR_EOC6);
        _UA1+=ADC->ADC_CDR[6];
    }
    /*
    * Calculate the control variable which equals: (_UA0 - _UA1)/(_UA0 + _UA1).
    * As the control variable IOnorm is an int and the calculation returnes a value between -1 and 1, the result is multiplied with 2047 to get a ~12-bit value.
    * A feature of this implementation is the inclusion of a sample-and-hold condition.
    * If the sum of the voltages is high enough (> nc * 600) the function calculates the new value of the control variable and returns true.
    * If this is not the case (else) the function returns false.
    * True and false can turn the controller on and off for one loop.
    */
    if((_UA0+_UA1)>(nc*600)){ 
        IOnorm = (2047*(_UA0-_UA1))/(_UA0+_UA1);//norm*2047 (int16_t, so no decimal places) 2047 is chosen because of 12-bit dac output range (2*2047+1=4095~4096)
        return true;
    }
    return false;
}

void Fast_IO_Due::write_dac(uint16_t dac_value){
    while(!DACC_ISR_TXRDY); //Conversion complete?
    DACC->DACC_CDR = dac_value; //write new value
}

uint32_t Fast_IO_Due::_enable_adc_channels(std::initializer_list<uint8_t> _channels){
    uint32_t _adc_cher=0; //value to enable channels (in decimal)
    for(auto cp: _channels){ //iterate over all elements and change adc_cher to enable channels
        _adc_cher+=(_adc_channels_pow[cp]*(cp<=11)); //change _adc_cher accordingly; invalid entries will be ignored
    }
    return _adc_cher;
}
