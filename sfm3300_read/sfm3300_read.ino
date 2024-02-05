
/*
  Reading flow rate from Sensirion SFM3300 mass flow sensor over I2C.
  Heavily based on code from https://github.com/dwerne/Sensirion_SFM3000_arduino/
  Adjusted by by Darren Lewis, May 2020
  Adjusted by CowBell Labs Clinic Team, Fall 2023-Spring 2024
  With input from ChatGPT
*/



#include <avr/pgmspace.h>
#include <Wire.h>
#include <sfm3000wedo.h>

SFM3000wedo measflow(64);

int offset = 32768;
int scale = 120;

#define orificeSensorPin A0

const float ADC_mV = 4.8828125;
const float sensitivity = 4.413;
const float mmh2O_cmH2O = 10;
const float mmh2O_kpa = 0.00981;
const float mmh2O_pa = 9.80665;
const int ADCoffset = 34;

void setup() {
  delay(500);
  Serial.begin(9600);
  measflow.init();
  Serial.println("Sensor initialized!");
}

int const a_size = 100;
float rollingAverage[a_size] = {0};  // Array to store the last 'a_size' SFM3300 sensor readings
int currentIndex = 0;

void loop() {
  int rawValue = analogRead(orificeSensorPin);
  float zeroedADCval = float(rawValue) - ADCoffset;
  zeroedADCval = constrain(zeroedADCval, 0, 1023);
  float pressurePa = zeroedADCval * ADC_mV / sensitivity * mmh2O_pa;

  float flowSFM = measflow.getvalue();
  if (flowSFM > 0) flowSFM = 0;
  else if (flowSFM < 0) flowSFM = flowSFM - offset;
  flowSFM = flowSFM / scale;

  // Update the rolling average array
  rollingAverage[currentIndex] = flowSFM;
  currentIndex = (currentIndex + 1) % a_size;

  // Calculate the average of the last 'a_size' readings
  float averageFlow = 0;
  for (int i = 0; i < a_size; ++i) {
    averageFlow += rollingAverage[i];
  }
  averageFlow /= a_size;

  Serial.print(millis());
  Serial.print(" , ");
  Serial.print(rawValue);
  Serial.print(" , ");
  Serial.print(flowSFM);
  Serial.print(" , ");
  Serial.println(averageFlow);

  // delay(100);
}
