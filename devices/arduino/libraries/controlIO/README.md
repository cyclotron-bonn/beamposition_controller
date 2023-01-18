#  Fast_IO_Due
A library to enable fast adc and dac conversion

## About
This library tries to make full use of the build-in ADC and DAC of the arduino due.
Any adc channel can be used. DAC0/2 is default output.
    
## How to use
1) There are two setup functions <initialize_adc(std::initializer_list 'uint8_t' _channels = {0})> and <initialize_dac()>. Call these to initialize the fast I/O.
See code example on how to use <initialize_adc()>
2) To read an adc value from A0, use <read_adc()>. It returns a value of type <uint16_t>.
2.1) Any other adc can be read with <read_anyadc(uint8_t)> passing the adc number (A0 -> 0 etc.) 
3) To write a value to the dac, use <write_dac(uint16_t)>, it doesnt return a value.
4) Diff/Sum of two channels can also be calculated using <bool get_norm(size_t)>, passing the number of reads per channel. It sets IOnorm a value in between -2047 and 2047 (~12bit).
If the sum is too low (under 300 per channel) IOnorm isnt set to the new value and <get_norm> returns false, as the value wasnt updated.
ADC's A0 and A1 are used for the norm. Be sure they are enabled in the <initialize_adc()> function.
##ADC-Channels of Arduino Due
ADC Channels: The ADC channels do not match the port-number. I.e. channel 0 is not A0 and so on. Here is a list of the matchings:
    - A0 -> 7   A5 -> 2    A10 -> 12
    - A1 -> 6   A6 -> 1    A11 -> 13
    - A2 -> 5   A7 -> 0    A15 -> Temperature sensor
    - A3 -> 4   A8 -> 10
    - A4 -> 3   A9 -> 11
    
## Code
A simple arduino code on how to initialize adc and dac as well as reading and writing values. (May only work with most up to date version).

```c++
#include <Fast_IO_Due.h>
Fast_IO_Due fastIO;

void setup(){
    fastIO.initialize_adc({0,1}); //Enable Pin A0 and A1
    fastIO.initialize_dac();
}

uint16_t val;
uint16_t val2;
int16_t norm;
void loop(){
    norm = fastIO.get_norm(5)
    val= fastIO.read_adc(); //always A0
    fastIO.write_dac(val);
    val2=fastIO.read_anyadc(1); //read channel A1
    fastIO.write_dac(val2);
}
```

