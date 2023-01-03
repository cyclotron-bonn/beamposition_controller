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

class SerialComm:public PIController, public Fast_IO_Due{
public:
    SerialComm();
    ~SerialComm();
    bool process(PIController piController[], Fast_IO_Due&);
    void resetInputBuffer();
    void receive(void);
    
private:
    const static size_t BUF_SIZE = 16;
    char arg[BUF_SIZE];
    const char _DELIM = ':';
    static constexpr char READ = 'R';
    static constexpr char WRITE = 'W';
    static constexpr char RESET = 'X';
    static constexpr char CHECK = 'C';
    static constexpr char ERR = 'E';
    static constexpr char EN = 'S';
    static constexpr char DIS = 'U';
    static constexpr char P = 'p';
    static constexpr char I = 'i';
    static constexpr char F = 'f';
    static constexpr char NORM = 'N';
    static constexpr char _adc = 'A';
    static constexpr char _dac = 'D';
    const char DELIM = ':';
    const char NL = '\n';
    const int _END = int(NL);
    const char TER = '\r';
    const char _NULL_TERM = '\0';
    
private:
    uint32_t fast_atoi(char*);  
    void write(PIController&,Fast_IO_Due&, char);
    uint32_t read(PIController&, Fast_IO_Due&, char);
};

#endif /* SerialComm_h */
