#include <PIController.h>
#include <controlIO.h>
#include <SerialComm.h>


SerialComm SCom;

//Allocate space for 4 instances acting as seperate controllers
PIController controllers[4];

float kp = 1;
float ki= 1;
float hz = 100;
uint8_t add = 0;
uint8_t adcl = 0;
uint8_t adcr = 0;
uint8_t bits = 12;
void setupControllers(){
  for(uint8_t i=0; i<4;i++){
    controllers[i].configure(kp, ki, hz, add, adcl, adcr, bits);
  }
}

void setup() {
  setupControllers();
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  //myPID.setOutputRange(-2047,2047);
  delay(1000);
}

int16_t output, norm, setpoint=0;
int16_t shift = (0b1<<(bits-1))-1;

void controller(){
  for(uint8_t i=0; i<4; i++){
    PIController temp=controllers[i];
    if(temp.active){
      norm = temp.getNorm();
      Serial.println(norm);
      if(norm){
        output = temp.step(setpoint, 100);
      }
    }
    Serial.println(output);
    //temp.setOutput(shift+output);
  }
//  control = fastIO.calc_norm(5);
//  if(control){
//    output=myPID.step(setpoint, fastIO.IOnorm); //If PID-Controller is turned on (control == true), calculate new output.
//  }
  //fastIO.write_dac(int16_t(2047+output)); //return output (can be old held one or new calculated one).
}

void loop() {
  uint32_t start = micros();
  for(uint32_t t = 0; t<1000; t++){
    if(Serial.available()){
      SCom.process(controllers);
    }
    controller();
    }
  Serial.println(micros()-start);
}
