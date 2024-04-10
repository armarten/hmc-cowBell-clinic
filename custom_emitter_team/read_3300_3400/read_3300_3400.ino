// Written for the 2024 Harvey Mudd College CowBell Labs Clinic Project
// Contributers: Dominick Quaye, Allison Marten
// Some code written by ChatGPT


//Code starts here!!

//Rando: #include
//#include <avr/pgmspace.h>      // For flash storage

//----------------------------------------------------------
// NEW1 Pattern Input Setup
//----------------------------------------------------------
// For string parsing pattern input
#include <iostream> 
#include <vector> 
#include <sstream> 
#include <algorithm> // for std::remove_if 
#include <string>
// End string parsing libraries

//----------------------------------------------------------
// (end new1) Motor Setup Stuff
//----------------------------------------------------------


bool scriptRunning = false; // Flag to indicate whether the script is running


// --------------------------------------------------------
//Flow Meter Setup Stuff
//----------------------------------------------------------
#include <Wire.h>
#define SDA_2 25  // Secondary I2C Bus 
#define SCL_2 26 // Secondary I2C Bus


void setup() {
  //--------------------------------------------------------
  // Motor Setup 
  // -------------------------------------------------------
 
  Serial.begin(9600);
  Serial.flush();
  
  // Initialize the default I2C bus for Flowsens_Big
  Wire.begin();

  // Initialize the secondary I2C bus for Flowsens_Small
  Wire1.begin(SDA_2, SCL_2);


  Wire.beginTransmission(byte(0x40));
  Wire.write(byte(0x10)); // Example command to Flowsens_Big
  Wire.write(byte(0x00)); // Example data to Flowsens_Big
  Wire.endTransmission();

  Wire1.beginTransmission(byte(0x40));
  Wire1.write(byte(0x10)); // Example command to Flowsens_Small
  Wire1.write(byte(0x00)); // Example data to Flowsens_Small
  Wire1.endTransmission();


  
}

int firstLoopFlag = 0;
float time_start_ms;
// std::string desiredFlowPatternString;


void loop() {
 
  float FlowSens_Big_Average = readAndAverageFlowSens_Big();
  float FlowSens_Small_Average = readAndAverageFlowSens_Small();

  float currentBigFlowRate = FlowSens_Big_Average; // Updates Flow 
  float currentSmallFlowRate = FlowSens_Small_Average; // Updates Flow 

  float total_flow= currentSmallFlowRate + currentBigFlowRate;

  Serial.print(millis()); // NEW1, so the time printed reflects time the flow is checked against
  Serial.print(" , ");
  Serial.print(currentBigFlowRate, 3); // time for csv
  Serial.print(" , ");
  Serial.print(currentSmallFlowRate, 3); // time for csv
  Serial.print(" , ");
  Serial.print(total_flow, 3); // time for csv
  Serial.println("");


  delay(100); // Just to give it some rest


}


  // ----------------------------------------------------------
  // Flow Measurement Functions 
  // ----------------------------------------------------------

float readAndAverageFlowSens_Big() {
  delay(10);
  Wire.requestFrom(0x40,2); // Request data from Sensor 1
  uint16_t a1 = Wire.read();
  uint8_t  b1 = Wire.read();
  a1 = (a1 << 8) | b1;
  float FlowSens_Big_Average = ((float)a1 - 32768) / 120; // Convert the data from Sensor 1
  // Serial.print("Flow from Sensor 1: ");
  // Serial.println(FlowSens_Big_Average);
  return FlowSens_Big_Average;
}


float readAndAverageFlowSens_Small() {
  delay(10);
  Wire1.requestFrom(0x40,2); // Request data from Sensor 2
  uint16_t a2 = Wire1.read();
  uint8_t  b2 = Wire1.read();
  a2 = (a2 << 8) | b2;
  float FlowSens_Small_Average = ((float)a2 - 32768) / 800; // Convert the data from Sensor 2
  // Serial.print("Flow from Sensor 2: ");
  // Serial.println(FlowSens_Small_Average);
  return FlowSens_Small_Average;
}





  // Read from Sensor 2






