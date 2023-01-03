//
//  SerialComm.h
//  
//
//  Created by Béla Knopp on 24.01.22.
//

#ifndef SerialComm_h
#define SerialComm_h

#include <Arduino.h>
#include "PIController.h"
#include "Fast_IO_Due.h"

class SerialComm:public PIController{
public:
    SerialComm();
    ~SerialComm();
    bool process(PIController* piController);
    void resetInputBuffer();
    void receive(void);
    
private:
    //general comm constants
    const static size_t BUF_SIZE = 16;
    char arg[BUF_SIZE];
    const char _DELIM = ':';
    const char _NL = '\n';
    const int _END = int(_NL);
    const char TER = '\r';
    const char _NULL_TERM = '\0';

    //which command is meant
    static constexpr char READ = 'R';
    static constexpr char WRITE = 'W';
    static constexpr char RESET = 'X';
    static constexpr char EN = 'S';
    static constexpr char DIS = 'U';
    static constexpr char ADDR = 'L';

    //what is asked
    static constexpr char PROP = 'P';
    static constexpr char INT = 'I';
    static constexpr char FREQ = 'F';
    static constexpr char NORM = 'N';
    static constexpr char _adc = 'A';
    static constexpr char _dac = 'D';

    //returns
    static constexpr char CHECK = 'C';
    static constexpr char ERR = 'E';
    
private:
    uint32_t fast_atoi(char*);  
    void write(PIController&);
    uint32_t read(PIController&);
};

#endif /* SerialComm_h */
