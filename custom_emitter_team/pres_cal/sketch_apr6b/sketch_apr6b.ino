#include <Wire.h>
float Vs = 5.14; // measured with multimeter
// float V_atm = 0.835; // multimeter voltage reading from data pin at atm
// float raw_atm = 165; // raw arduino reading at atm
// float sensitivity = 6.4; // V/P, mV/kPa

#define orificeSensorPin A0


void setup() {
  // put your setup code here, to run once:
  delay(500);
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

  int rawValue = analogRead(orificeSensorPin);
  // float rawValueZeroedGauge = rawValue - raw_atm;
  float Vout = rawValue * Vs / 1023;
  // float P1 = Vout / sensitivity;
  float P2 = (Vout/Vs - 0.04)/0.0012858;
  // float presVout = constrain(rawValue, 0, 1023);
  // float presVoutZeroedGauge = Vout - V_atm;
  // float P2 = presVoutZeroedGauge / sensitivity;
  // float 
  // float zeroedtoGauge = 
  // float presVout = constrain(rawValue, 0, 1023);
  float P_sia_rough = (P2*1000)/6894.757293178;
  float P_sia_cal = 0.998*P_sia_rough + 0.503;
  
  // float zeroedADCval = float(rawValue) - ADCoffset;
  // // float zeroedADCval = rawValue;
  // zeroedADCval = constrain(zeroedADCval, 0, 1023);
  // float presPa = zeroedADCval * ADC_mV / sensitivity * mmh2O_pa;


  // float preskPa = ((presVout/Vs)-0.04)/0.0012858;
  // // float preskPa = (rawValue - 40.92)/1.31537 ;
  // float presPa = preskPa * 1000;
  // float prespsia = presPa / (6894.757293178);


  Serial.print(millis());
  Serial.print(" , ");
  // Serial.print(rawValue);
  // Serial.print(" , ");
  // Serial.print(Vout);
  // Serial.print(" , ");
  // // Serial.print(presVoutZeroedGauge);
  // Serial.print(" , ");
  Serial.print(P_sia_rough);
  Serial.print(" , ");
  Serial.print(P_sia_cal);
  Serial.println("");
  delay(500);
}




