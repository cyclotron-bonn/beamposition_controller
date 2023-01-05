from gzip import READ
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

    @property
    def active(self, controller):
        """check if controller is active

        Args:
            controller (int): controller to be checked (0 to 3)

        Returns:
            bool: whether controller is active
        """
        return self._read_data(controller=controller, var = self.con_ACT)

    @active.setter
    def active(self, controller, value: bool):
        """enable/disable a controller
            enable: -> arduino will read its assigned adcs, calculate and set output to dac

        Args:
            controller (int): controller to be enabled/disabled (0 to 3)
            value (bool): true: enable; false: disable
        """
        self._write_data(controller=controller, var = self.con_ACT, value = int(value))
    
    @property
    def address(self, controller):
        """sets the SPI/I2C address of the controllers dac

        Args:
            controller (int): controller (0 to 3)
        """
        self._read_data(controller=controller, var=self.con_ADD)
        
    @address.setter
    def address(self, controller, value):
        self._write_data(controller=controller, var=self.con_ADD, value=value)
    
    @property
    def adc_left(self, controller):
        """sets the left (or upper) adc channel

        Args:
            controller (int): controller (0 to 3)
        """
        self._read_data(controller=controller, var=self.con_ADCL)
        
    @adc_left.setter
    def adc_left(self, controller, value):
        self._write_data(controller=controller, var=self.con_ADCL, value=value)
    
    @property
    def adc_right(self, controller):
        """sets the right (or lower) adc channel

        Args:
            controller (int): controller (0 to 3)
        """
        self._read_data(controller=controller, var=self.con_ADCR)
        
    @adc_right.setter
    def adc_right(self, controller, value):
        self._write_data(controller=controller, var=self.con_ADCR, value=value)
    
    def check_controllers(self):
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
                print("Controller {} is enabled")
            else: print("Controller {} is disabled")
            print("P={}, I={}, Freq={}, DAC-Address={}, ADCL={}, ADCR={}".format(c_p,c_i,c_hz,c_add,c_adcl,c_adcr))
            
    def _read_data(self, controller, var):
        """reads data/constants from the arduino.  <con> determines which

        Args:
            controller (int): the controller from which the variable is to be read
            var (char): the variable to be read.
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
            controller (int): controller on which variable is to be changed
            var (char): variable to be set\\
            value (float): new value
        """
        cmd = self.create_command(self.WRITE_delim, controller, var, value)
        self.write(cmd)

    
    
    