
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
// float pres_atm_offset_pa = ;
// int presPin = 35;

#define orificeSensorPin A0

const float ADC_mV = 4.8828125;      // convesion multiplier from Arduino ADC value to voltage in mV
const float sensitivity = 6.4;     // in mV/kPa taken from datasheet
// const float mmh2O_cmH2O = 10;        // divide by this figure to convert mmH2O to cmH2O
// const float mmh2O_kpa = 0.00981;     // convesion multiplier from mmH2O to kPa
// const float mmh2O_pa = 9.80665;      // convesion multiplier from mmH2O to Pa
const int ADCoffset = 34;            // measured from Arduino when rig is off. Valid for MPX5010DP

float Vs = 5.14;

void setup() {
  delay(500);
  Serial.begin(9600);
  measflow.init();
  Serial.println("Sensor initialized!");
  Serial.println("data_begin");
  Serial.print("time_ms");
  Serial.print(" , ");
  Serial.print("raw_val");
  Serial.print(" , ");
  Serial.print("point_flow");
  Serial.print(" , ");
  Serial.print("avg_flow");
  Serial.print(" , ");
  Serial.print("pres_pa");
  Serial.print(" , ");
  Serial.println("pres_psia");
}

int const a_size = 50;
float rollingAverage[a_size] = {0};  // Array to store the last 'a_size' SFM3300 sensor readings
int currentIndex = 0;



void loop() {

  int rawValue = analogRead(orificeSensorPin);
  float Vout = rawValue * Vs / 1023;
  float P2 = (Vout/Vs - 0.04)/0.0012858;
  float P_sia_rough = (P2*1000)/6894.757293178;
  float P_sia_cal = 0.998*P_sia_rough + 0.503;

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
  Serial.print(averageFlow);
  Serial.print(" , ");
  Serial.print(presPa);
  Serial.print(" , ");
  Serial.println(prespsia);


  delay(100);
}
