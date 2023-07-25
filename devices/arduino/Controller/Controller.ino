#include <PIController.h>
#include <SerialComm.h>
#include <SPIComm.h>

#define NORM_RES ((0x1U<<12)-1)


//Allocate space for 'n_controllers' instances acting as seperate controllers (default = 4, defined globally in PIController.h)
bool doubleslit = true;

PIController *controllers;

float kp = 1;
float ki = 1;
uint8_t bits = 16;

void setupControllers() {
  if(doubleslit){
    n_controllers = 8;
  }
  else{
    n_controllers = 4;
  }
  controllers = new PIController[n_controllers]; 
  for (size_t i = 0; i < n_controllers; i++) {
    controllers[i].configure(kp, ki, 2 * i, 2 * i + 1, i, bits); //p,i,address adc_left, address adc_right, adcress dac, resolution
  }
}

void setup() {
  setupControllers();
  SPIsetup(true);
  //SPI.begin();
  Serial.begin(115200);
  
  delay(1000);

//  digitalWrite(CS_DAC, LOW);
//  uint8_t output = 0b01001111;
//  uint16_t value = 0xFFFF;
//  SPI.beginTransaction(SPISettings(1400000, MSBFIRST, SPI_MODE1));
//  SPI.transfer(output);
//  SPI.transfer16(value);
//  digitalWrite(CS_DAC, HIGH);
}

int16_t output, norm, setpoint = 0;
int16_t shift = (0b1 << (bits - 1)) - 1;



void loop(){
  uint16_t voltages[] = {0,0,0,0,0,0,0,0};
  analogReadPrec(4);
  delay(1000);
  //while (micros() - start < controlDelayMicro);
}



int16_t calcNorm(int32_t L, int32_t R) {
  /*
    Calculate the control variable N=(L - R)/(L + R)
    limit regulates if new output is generated, if SUM is lower -> don't control
  */
  int32_t limit = NORM_RES >> 4;
  int32_t DIFF = L - R;
  int32_t SUM = L + R;
  if (SUM > limit) {
    int16_t NORM = DIFF * NORM_RES / SUM;
    return NORM;
  }
  return 0;
}

int32_t L, R, N;
void controller() {
  for (uint8_t i=0; i<n_controllers; i++) {
    //for (uint8_t i = 0; i < n_controllers; i++) {
    //PIController temp = controllers[i];
    if (controllers[i].active) {
      //L = readADC(controllers[i].ADC_L);
      //R = readADC(controllers[i].ADC_R);
      N = calcNorm(10000, 20000);
      Serial.print("N:");
      Serial.println(N);
      if (N) {
        output = controllers[i].step(N);
      }
    }
    Serial.println(output);
    //writeDAC(controllers[i].DAC, output);
  }
}

//void loop() {
//  if (Serial.available()) {
//    SCom.process(controllers);
//  }
//  uint32_t start = micros();
//  controller();
//  while (micros() - start < controlDelayMicro);
//}



//void loop(){
//  writeDAC(1,2);
//  delay(1000);
//  }
