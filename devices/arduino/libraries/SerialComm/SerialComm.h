#ifndef SerialComm_h
#define SerialComm_h

#include <Arduino.h>
#include "PIController.h"

class SerialComm:public PIController{
public:
    SerialComm();
    ~SerialComm();
    bool process(PIController* piController);
    void resetInputBuffer();
    void receive(void);
    
private:
    //general constants
    const static size_t BUF_SIZE = 16;
    char arg[BUF_SIZE];
    const char _DELIM = ':';
    const char _NL = '\n';
    const int _END = int(_NL);
    const char TER = '\r';
    const char _NULL_TERM = '\0';

    //commands
    static constexpr char READ = 'R';
    static constexpr char WRITE = 'W';
    static constexpr char RESET = 'X';

    //variables
    static constexpr char PROP = 'P';
    static constexpr char INT = 'I';
    static constexpr char FREQ = 'F';
    static constexpr char ACT = 'O';

    //returns
    static constexpr char CHECK = 'C';
    static constexpr char ERR = 'E';
    
private:
    uint32_t fast_atoi(char*);  
    void write(PIController&, char);
    uint32_t read(PIController&, char);
};

#endif /* SerialComm_h */