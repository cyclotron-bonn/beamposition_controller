#include <PIController.h>
#include <SerialComm.h>
#include <SPIComm.h>

#define NORM_RES ((0x1U<<12)-1)

const int CS_ADC = 2; //adjust when designing pcb
const int DSEL_ADC = 3;
const int CS_DAC = 4;
const int DSEL_DAC = 5;

SerialComm SCom;

//Allocate space for 'n_controllers' instances acting as seperate controllers (default = 4, defined globally in PIController.h)
PIController controllers[n_controllers];

float kp = 0;
float ki = 1;
uint8_t bits = 16;
void setupControllers() {
  for (size_t i = 0; i < n_controllers; i++) {
    controllers[i].configure(kp, ki, 2*i, 2*i+1, i, bits); //p,i,address adc_left, address adc_right, adcress dac, resolution
  }
}

void setup() {
  setupControllers();
  
  pinMode(CS_ADC, OUTPUT);
  pinMode(CS_DAC, OUTPUT);
  pinMode(DSEL_ADC, OUTPUT);
  pinMode(DSEL_DAC, OUTPUT);
  digitalWrite(CS_ADC, HIGH);
  digitalWrite(CS_DAC, HIGH);
  digitalWrite(DSEL_ADC, HIGH);
  digitalWrite(DSEL_DAC, HIGH);
  
  Serial.begin(115200);
  delay(1000);
}

int16_t output, norm, setpoint = 0;
int16_t shift = (0b1 << (bits - 1)) - 1;

uint16_t readADC(uint8_t channel) {
  digitalWrite(CS_ADC, LOW);
  uint16_t input = channel << 5;
  uint16_t output;
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE1)); //for max1168
  output = SPI.transfer16(input);
  digitalWrite(CS_ADC, HIGH);
  return output;

}

void writeDAC(uint8_t channel, uint16_t val) {
  digitalWrite(CS_DAC, LOW);
  uint16_t output = channel+val;
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE1));
  SPI.transfer16(output);
  digitalWrite(CS_DAC, LOW);
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
  for (uint8_t i = 0; i < n_controllers; i++) {
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

void loop() {
    if (Serial.available()) {
      SCom.process(controllers);
    }
    uint32_t start = micros();
    controller();
    while(micros()-start<controlDelayMicro);
}
