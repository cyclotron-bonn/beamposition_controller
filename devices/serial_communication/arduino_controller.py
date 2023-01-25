from arduino_serial import ArduinoSerial
import logging
from time import sleep

class ArduinoController(ArduinoSerial):
    PARAM_SHIFT = 2**8
    #commands
    READ_delim = 'R' #read
    WRITE_delim = 'W' #write
    RESET_delim = 'X' #cross
    #variables
    con_P = 'P' #proportional
    con_I = 'I' #integral
    con_HZ = 'F' #frequency
    con_HZ_BITS = 'B'
    con_ADD = 'A'
    con_ADCL = 'L'
    con_ADCR = 'R'
    con_ACT = 'O'
    
    #returns
    CHECK_delim = 'C' #check
    ERR_delim = 'E' #error
    
    #controller variables
    controller_address = 0

    def __init__(self, port = "/dev/cu.usbmodem14301", baudrate=115200, timeout=1., P=1, I=0, HZ=130000, controller = 0):
        super().__init__(port=port, baudrate=baudrate, timeout=timeout)
        self.check_con()
        self.frequency(value = HZ)
        self.proportional(input = P)
        self.integral(input = I)
        self.address = self.address

    def check_con(self):
        """checks if the arduino is responding

        Raises:
            RuntimeError: One of two things happend: either the arduino is stuck in code and is not responding\\
                or the serial connection was not successful
        """
        cmd = self.create_command(self.CHECK_delim)
        ans = self.query(cmd)
        if ans != 'C':
           raise RuntimeError("Serial connection to arduino not established or arduino is not responding")
    
    def reset_ard(self):
        """resets the arduino via software. not as powerful as arduino must be able to respond\\
            but this restores the default settings
        """
        cmd = self.create_command(self.RESET_delim)
        self.write(cmd)
    
    def frequency(self):
        """gets frequency used by PID-self.address. The value does not represent the actual sampling rate
        """
        return self._read_data( var = self.con_HZ)

    def frequency(self,value):
        """Sets the frequency used by PID-Controller. This does not change the actual sampling rate.
            Real value is estimated to be around 130kHz.
        Args:
            value (int): new frequency
        """
        if(value > 0):
            self._write_data(var = self.con_HZ, value = int(value))
        else:
            raise ValueError("Frequency must be greater than 0")

    
    def proportional(self):
        """gets P-value of PID-self.address
        """
        return self._read_data(var = self.con_P)/self.PARAM_SHIFT

    def proportional(self, input):
        """sets P-value of PID self.address

        Args:
            value (float): new value

        Raises:
            ValueError: input value must be <256 and >=0\\
                (Values smaller than 1/256 will be rounded to 0 or 1/256)
        """
        if (input > self.PARAM_SHIFT or input < 0):
                raise ValueError("PID-constants must be smaller than "+ str(self.PARAM_SHIFT) + " and greater or equal to 0")
        else:
            output = int(input * self.PARAM_SHIFT)
            self._write_data(var = self.con_P, value = output)

    
    def integral(self):
        """gets I-value of PID-self.address
        """
        return self._read_data(var = self.con_I)/self.PARAM_SHIFT

    def integral(self, input):
        if (input > self.PARAM_SHIFT or input < 0):
                raise ValueError("PID-constants must be smaller than "+ str(self.PARAM_SHIFT) + " and greater or equal to 0")
        else:
            output = int(input * self.PARAM_SHIFT)
            self._write_data(var=self.con_I, value=output)

    
    def active(self):
        """check if controller is active
        Returns:
            bool: whether self.address is active
        """
        return self._read_data(var = self.con_ACT)

    def active(self, value: bool):
        """enable/disable the controller
            enable: -> arduino will read its assigned adcs, calculate and set output to dac

        Args:
            value (bool): true: enable; false: disable
        """
        self._write_data(var = self.con_ACT, value = int(value))
    
    
    def address(self):
        """sets the SPI/I2C address of the self.addresss dac

        Args:
            self.address (int): self.address (0 to 3)
        """
        self._read_data(var=self.con_ADD)
        
    def address(self, value):
        self._write_data(var=self.con_ADD, value=value)
    
    
    def adc_left(self):
        """sets the left (or upper) adc channel

        Args:
            self.address (int): self.address (0 to 3)
        """
        self._read_data(var=self.con_ADCL)
        
    def adc_left(self, value):
        self._write_data(var=self.con_ADCL, value=value)
    
    def adc_right(self):
        """sets the right (or lower) adc channel

        Args:
            self.address (int): self.address (0 to 3)
        """
        self._read_data(var=self.con_ADCR)
        
    def adc_right(self, value):
        self._write_data(var=self.con_ADCR, value=value)
    
    def check_controller(self):
        for i in range(4):
            c = int(i)
            c_act = self.active(c)
            c_p = self.proportional(c) 
            c_i = self.integral(c)
            c_hz = self.frequency(c)
            c_add = self.address(c)
            c_adcl = self.adc_left(c)
            c_adcr = self.adc_right(c)
            if c_act:
                print("self.address {} is enabled")
            else: print("self.address {} is disabled")
            print("P={}, I={}, Freq={}, DAC-Address={}, ADCL={}, ADCR={}".format(c_p,c_i,c_hz,c_add,c_adcl,c_adcr))
    
    def _read_data(self, var):
        """reads data/constants from the arduino.  <con> determines which

        Args:
            var (char): the variable to be read.
        Returns:
            [float]: constant
        """
        cmd = self.create_command(self.READ_delim, self.controller_address, var)
        ans = self.query(cmd)
        if ans == '':
           raise RuntimeError("No answer received")
        #get int from transmitted string
        valuestr = ''.join(x for x in ans if x.isdigit())
        if(len(valuestr)>0):
            value = int(valuestr)
        else:
            raise RuntimeError("Answer does not contain any digits")
        self.reset_buffers()
        return value
    
    def _write_data(self, var, value):
        """writes data to arduino
        Args:
            var (char): variable to be set\\
            value (float): new value
        """
        cmd = self.create_command(self.WRITE_delim, self.controller_address, var, value)
        self.write(cmd)

    
    
    