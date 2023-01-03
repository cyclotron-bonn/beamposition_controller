#include <PIController.h>
#include <Fast_IO_Due.h> 
#include <SerialComm.h>


Fast_IO_Due fastIO;
SerialComm SCom;

float Hz=130000;
float Kp=0, Ki=0, Kd=0;
int output_bits=11;
bool output_signed=true;
PIController control1(Kp, Ki, Hz, output_bits);
PIController control2(Kp, Ki, Hz, output_bits);
PIController control3(Kp, Ki, Hz, output_bits);
PIController control4(Kp, Ki, Hz, output_bits);

//Allocate space for 4 instances acting as seperate controllers
PIController controllers[4] = {control1, control2, control3, control4};


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  fastIO.initialize_adc({0,1,2,3,4,5});
  fastIO.initialize_dac();
  //myPID.setOutputRange(-2047,2047);
  delay(1000);
}

int16_t output, setpoint=0;
bool control;

void controller(){
  for(uint8_t i=0; i<4; i++){
    //readADC()
    PIController temp=controllers[i];
    if(temp.active){
      temp.step(setpoint, 100);
    }
  }
//  control = fastIO.calc_norm(5);
//  if(control){
//    output=myPID.step(setpoint, fastIO.IOnorm); //If PID-Controller is turned on (control == true), calculate new output.
//  }
  fastIO.write_dac(int16_t(2047+output)); //return output (can be old held one or new calculated one).
}

void loop() {
  //int start = micros();
  //for(size_t t = 0; t<1000000; t++){
  if(Serial.available()){
    SCom.process(controllers, fastIO);
  }
  controller();
  //}
  //Serial.println(micros()-start);
}
