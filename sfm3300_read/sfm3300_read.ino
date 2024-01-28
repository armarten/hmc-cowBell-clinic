/*
   Reading flow rate from DIY orifice flow sesnor. This code reads from the following sensors and outputs
   data to the serial terminal which can be copied into notepad and saved as a .csv file for importing into Excel:
   - Reads flow rate from the DIY orifice flow sensor based on a lookup table (will need recalibrating for your test rig)
     (not compensated for temperature or downstream pressure)
   - Reads the sensirion SFM3300 sensor over i2c for calibration purposes
  Code written by Darren Lewis, May 2020
*/

#include <avr/pgmspace.h>      // for flash storage
#include <Wire.h>              // needed for SFM
#include <sfm3000wedo.h>

SFM3000wedo measflow(64);      // connects to A4(SDA) A5(SCL) on UNO

int offset = 32768;            // SFM Offset for the SFM3300, check datasheet if you're using a different sensor
int scale = 120;               // SFM Scale

#define orificeSensorPin A0    // connection pin for MPX5010DP

// ADC count, flow (ml/s) lookup table MPX5010DP


const float ADC_mV = 4.8828125;      // convesion multiplier from Arduino ADC value to voltage in mV
const float sensitivity = 4.413;     // in mV/mmH2O taken from datasheet
const float mmh2O_cmH2O = 10;        // divide by this figure to convert mmH2O to cmH2O
const float mmh2O_kpa = 0.00981;     // convesion multiplier from mmH2O to kPa
const float mmh2O_pa = 9.80665;      // convesion multiplier from mmH2O to Pa
const int ADCoffset = 34;            // measured from Arduino when rig is off. Valid for MPX5010DP
// the offset value from the datasheet I found was too high at atmospheric, that's why I used a measured value for offset

void setup() {
  //Wire.begin();
  delay(500); // let serial console settle
  Serial.begin(9600);

  // initialize the sesnor
  measflow.init();
  Serial.println("Sensor initialized!");
}

void loop() {
  // for reading the raw pressure values of the orifice flow senror
  int rawValue = analogRead(orificeSensorPin);
  float zeroedADCval = float(rawValue) - ADCoffset;                    // normalise readings with the offset
  zeroedADCval = constrain(zeroedADCval, 0, 1023);
  float pressurePa = zeroedADCval * ADC_mV / sensitivity * mmh2O_pa;  // result in Pa use this line when using ADCoffset


  // for reading SFM3300
  float flowSFM = measflow.getvalue();
  //if (flowSFM > 0) flowSFM = flowSFM + offset;        // if you want to include negatives
  if (flowSFM > 0) flowSFM = 0;                         // rectifies values
  else if (flowSFM < 0) flowSFM = flowSFM - offset;     // needed to correct values. The problem could be that the raw
  flowSFM = flowSFM / scale;      // L/m                   readings are passed in 2 bytes which are in the wrong order?
  flowSFM = flowSFM * 16.6666;    // convert to ml/s

 

  Serial.print(millis());           // record time in ms
  Serial.print(" , ");              // needed to create .csv file
  Serial.print(rawValue);           // raw ADC value
  Serial.print(" , ");
  Serial.print(pressurePa);         // reading of differential pressure over the orifice
  Serial.print(" , ");
  Serial.println(flowSFM);          // flow rate reading from SFM3300 sensor
  // using serial print format above, you can copy your data from the arduino terminal,
  // paste it into NotePad app, than save as a .CSV file which can be opened in Excel

// to use the serial plotter (under tools menu); uncomment serial prints below and comment out serial prints above

//  Serial.print(rawValue);           // raw ADC value
//  Serial.print("  ");
//  Serial.print(pressurePa);         // reading of differential pressure over the orifice
//  Serial.print("  ");
//  Serial.print(flowRate);           // flow rate reading from orifice sensor
//  Serial.print("  ");
//  Serial.print(flowSFM);            // flow rate reading from SFM3300 sensor
//  Serial.print("  ");
//  Serial.println("uT");

  //delay(100);     // uncomment this delay to make it easier to read number to the screen
}
