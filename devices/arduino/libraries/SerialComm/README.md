#  SerialComm
Library used to make the arduino understand string commands and make a valid reply.
A counterpart with similar design is needed to establish communication

#About
SerialComm only works together with other libarys i.e. Fast_IO_Due and PastPID as the commands have to do something. Can be changed in code fairly easily to fit any needs.

## How to use
Two functions:
1) <char* process(char* cmd, size_t cmdlen)>. <char* cmd> should be an array of size <size_t cmdlen>.
The passed command must have a form of: one letter constant, one letter delimiter and if needed for the command an unsigned value. Example command: <P=100> to set the value P to 100.
It will return a command of type char array.
2) <void reset_input_buffer()> clears any left data in the input serial buffer.

##List of constants and delimiters (03.02.22):
Constants:
    P for proportional  C for check
    I for integral      R for reset
    D for differential
    F for frequency
Delimiters:
    ? to get    doesnt expect a value, would be ignored anyway
    = to set    expects a value, if none is given arduino will return a get command 
    ! to execute    doesnt expect a value (C! and R! are only executed)
    
    
## Code
A simple arduino code on how to initialize the communication and make it work (May only work with most up to date version).

```c++
#include <SerialComm.h>
SerialComm SCom(); //initialize 

void setup(){
    Serial.begin(115200); //any baudrate may be chosen
}

void getcommand(){
    char cmdraw[16];
    size_t cmdlen = Serial.readBytesUntil('\n', cmdraw, 16);
    char cmd[cmdlen+1];
    for(size_t i =0; i<cmdlen; i++){
     cmd[i]=cmdraw[i];
    }
    char* ans = (char*) malloc((16)*sizeof(char));
    ans = SCom.process(cmd, cmdlen);
    Serial.print(ans);
    Serial.flush();
    reset_input_buffer();
    free(ans);
}

void loop(){
    if(Serial.available()){
        getcommand();
    }
}
```

