#include <PIController.h>
#include <SerialComm.h>
#include <SPI.h>

const int CS_ADC = 2; //adjust when designing pcb
const int DSEL_ADC = 3;
const int CS_DAC = 4;
const int DSEL_DAC = 5;

SerialComm SCom;

//Allocate space for 4 instances acting as seperate controllers
PIController controllers[4];

float kp = 1;
float ki = 1;
float hz = 100;
uint8_t adcl = 0;
uint8_t adcr = 0;
uint8_t bits = 12;
void setupControllers() {
  for (uint8_t i = 0; i < 4; i++) {
    controllers[i].configure(kp, ki, adcl, adcr, bits);
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

int64_t resolution = (0x1LL << 16) - 1;
int16_t calcNorm(int32_t L, int32_t R) {
  /*
    Calculate the control variable N=(L - R)/(L + R)
    limit regulates if new output is generated, if SUM is lower -> don't control
  */
  int32_t limit = resolution >> 4;
  int32_t DIFF = L - R;
  int32_t SUM = L + R;
  if (SUM > limit) {
    int16_t NORM = DIFF * resolution / SUM;
    return NORM;//
  }
  return 0;
}

int32_t L, R, N;
void controller() {
  for (uint8_t i = 0; i < 4; i++) {
    PIController temp = controllers[i];
    if (temp.active) {
      L = readADC(temp.ADC_L);
      R = readADC(temp.ADC_R);
      N = calcNorm(L, R);
      if (N) {
        output = temp.step(setpoint, norm);
      }
    }
    Serial.println(output);
    //temp.setOutput(shift+output);
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
