from gzip import READ
from arduino_serial import ArduinoSerial
import logging
from time import sleep

class ArduinoController(ArduinoSerial):
    PARAM_SHIFT = 2**8
    #Delimiters
    ENABLE_delim = 'S' #set
    DISABLE_delim = 'U' #unset
    READ_delim = 'R' #read
    WRITE_delim = 'W' #write
    CHECK_delim = 'C' #check
    RESET_delim = 'X' #cross
    ADC_delim = 'A' #adc
    DAC_delim = 'D' #dac
    NORM_delim = 'N' #norm
    ERR_delim = 'E' #error
    #Terminator
    TERMI = '\n'
    #Letters for constants
    con_P = 'p' #proportional
    con_I = 'i' #integral
    con_HZ = 'f' #frequency

    def __init__(self, port = "/dev/cu.usbmodem14301", baudrate=115200, timeout=1., P=1, I=0, HZ=130000):
        super().__init__(port=port, baudrate=baudrate, timeout=timeout)
        self.check_con()
        self.frequency(value = HZ)
        self.proportional(input = P)
        self.integral(input = I)

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
    
    def enable_controller(self, controller):
        """enables given controller"""
        cmd = self.create_command(self.ENABLE_delim, controller)
        ans = self.query(cmd)
        if ans == '':
            raise RuntimeError("No answer received")
        if ans == 'X':
            raise ValueError("Given controller was not found")
        if ans == 'C':
            print("Controller enabled")
    
    def disable_controller(self, controller):
        """enables given controller"""
        cmd = self.create_command(self.DISABLE_delim, controller)
        ans = self.query(cmd)
        if ans == '':
            raise RuntimeError("No answer received")
        if ans == 'X':
            raise ValueError("Given controller was not found")
        if ans == 'C':
            print("Controller disabled")
    
    def read_adc(self, adc):
        cmd = self.create_command(self.READ_delim, self.ADC_delim, adc)
        ans = self.query(cmd)
        if ans == '':
            raise RuntimeError("No answer received")
        return ans
    
    def get_norm(self):
        """gets the calculated norm from an arduino

        Returns:
            norm: as arduino can only answer unsigned ints, a negative norm results in the first bit of 32 bit return to be flipped\\
                resulting in huge numbers. 2**32 is subtracted when norm is too great to solve this issue
        """
        norm = self._read_data(con = self.NORM_delim)
        if norm > 5000:
            norm-=2**32
        return norm

    def get_values(self, adc0, adc1):
        try:
            while True:
               print(self.read_adc(adc = adc0))
               print(self.read_adc(adc = adc1))
               print(self.get_norm())
               print("")
               sleep(0.5)
        except KeyboardInterrupt:
            pass
    
    @property
    def frequency(self, controller):
        """gets frequency used by PID-Controller. The value does not represent the actual sampling rate
        """
        return self._read_data(controller=controller, var = self.con_HZ)

    @frequency.setter
    def frequency(self, controller, value):
        """Sets the frequency used by PID-Controller. This does not change the actual sampling rate.
            Actual value is estimated to be around 130kHz.
        Args:
            value (int): new frequency
        """
        if(value > 0):
            self._write_data(controller=controller, var = self.con_HZ, value = int(value))
        else:
            raise ValueError("Frequency must be greater than 0")

    @property
    def proportional(self, controller):
        """gets P-value of PID-controller
        """
        return self._read_data(controller=controller, var = self.con_P)/self.PARAM_SHIFT

    @proportional.setter
    def prop(self, controller, input):
        """sets P-value of PID controller

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
            self._write_data(controller=controller, var = self.con_P, value = output)

    @property
    def integral(self, controller):
        """gets I-value of PID-controller
        """
        return self._read_data(controller=controller, var = self.con_I)/self.PARAM_SHIFT

    @integral.setter
    def integral(self, controller, input):
        if (input > self.PARAM_SHIFT or input < 0):
                raise ValueError("PID-constants must be smaller than "+ str(self.PARAM_SHIFT) + " and greater or equal to 0")
        else:
            output = int(input * self.PARAM_SHIFT)
            self._write_data(controller=controller, var=self.con_I, value=output)

    def _read_data(self, controller, var):
        """reads data/constants from the arduino.  <con> determines which

        Args:
            con (char): the constant to be read.\\
            p -> proportional;
            i -> integral;
            d -> differential;
            f -> frequecy;

        Returns:
            [float]: constant
        """
        cmd = self.create_command(self.READ_delim, controller, var)
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
    
    def _write_data(self, controller, var, value):
        """writes data to arduino
        Args:
            con (char): constant to be set\\
            p -> proportional;
            i -> integral;
            d -> differential;
            f -> frequecy;
            D -> DAC;
            value (float): new value
        """
        cmd = self.create_command(self.WRITE_delim, controller, var, value)
        self.write(cmd)

    
    
    