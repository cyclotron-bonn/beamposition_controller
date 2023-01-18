//
//  SerialComm.cpp
//  
//
//  Created by Béla Knopp on 24.01.22.
//
#include "SerialComm.h"

SerialComm::SerialComm(){
}
SerialComm::~SerialComm(){
}

bool SerialComm::process(PIController* piController){
    receive();
    uint8_t cn;
    switch(arg[0]){
        case READ:
            receive(); 
            cn = arg[0];
            receive();
            Serial.println(read(piController[cn], arg[0]));
            break;
        case WRITE:
            receive();
            cn = arg[0];
            receive();
            write(piController[cn], arg[0]); //set value
            break;
        case RESET:
            break;
        case CHECK:
            Serial.println(CHECK);
            break;
        default:
            return true;
    }
    return false;
}

uint32_t SerialComm::read(PIController &pi_controller, char con){
    switch (con){
    case PROP:
        return pi_controller._p;
    case INT:
        return pi_controller._i;
    case FREQ:
        return pi_controller._hz;
    case FREQ_BITS:
        return pi_controller._hz_bits;
    default:
        return 0;
    }
}

void SerialComm::write(PIController &pi_controller, char con){
    receive();
    uint32_t value = fast_atoi(arg);
    switch (con){
        case PROP:
            pi_controller._p = value;
            break;
        case INT:
            pi_controller._i = value;
            break;
        case FREQ:
            pi_controller._hz = value;
            pi_controller._hz_bits = pi_controller.hzToBits(value);
            break;
        case ADDR:
            pi_controller.ADDRESS = value;
        case cADC_L:
            pi_controller.ADC_L = value;
        case cADC_R:
            pi_controller.ADC_R = value;
        case ACT:
            if(value){
                pi_controller.active=true;
            }
            else{pi_controller.active=false;}
    }
}

void SerialComm::receive(){
   /*
   * reads data from serial buffer and seperates at given _DELIM delimiter.
   * halts reading when _END character is found or args cant fit any more data (argnum)
   * empties serial buffer at the end
   */
    size_t curarglen;
    if(Serial.peek() == _END){
        Serial.read();
        arg[0] = _NULL_TERM;
    }
    else{
        curarglen = Serial.readBytesUntil(_DELIM, arg, BUF_SIZE);
        arg[curarglen] = _NULL_TERM;
        Serial.read();
    }
}

void SerialComm::resetInputBuffer(void){
  /*
   * reads all data serial buffer and discharges them
   */
    while(Serial.available()){
        Serial.read();
    }
}

uint32_t SerialComm::fast_atoi(char* str){
    int val = 0;
    while( *str ) {
        val = val*10 + (*str++ - '0');
    }
    return val;
}

