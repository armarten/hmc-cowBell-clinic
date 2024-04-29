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

int numReadings = 50;

// RunningAverage RA33(rolling_avg);
// RunningAverage RA34(rolling_avg);


bool scriptRunning = false; // Flag to indicate whether the script is running

// float total_delay_us = 100;

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
 
  float FlowSens_Big_Average = readAndAverageFlowSens_Big(numReadings);
  float FlowSens_Small_Average = readAndAverageFlowSens_Small(numReadings);

  float currentBigFlowRate = FlowSens_Big_Average; // Updates Flow 
  float currentSmallFlowRate = FlowSens_Small_Average; // Updates Flow 

  float total_flow= currentSmallFlowRate + currentBigFlowRate;


  String dataPrint = String(millis()) + " , " + String(currentBigFlowRate, 6) + " , " + String(currentSmallFlowRate, 6) + " , " + String(total_flow, 6);
  Serial.println(dataPrint);


}



  // ----------------------------------------------------------
  // Flow Measurement Functions 
  // ----------------------------------------------------------


float readAndAverageFlowSens_Big(int numReadings1) { // *** Does this actually average?

  float FlowSens_Big = 0;
  int sfm3300_scale = 120; // From data sheet
  int sfm3300_offset = 32768; // From data sheet
  int samps;

  for (samps = 0; samps < numReadings1; samps++) {

    delay(1); // Needed
    Wire.requestFrom(0x40, 2);  // Request data from Sensor 1
    uint16_t a1 = Wire.read();
    uint8_t b1 = Wire.read();
    a1 = (a1 << 8) | b1;
    float sensorReading = ((float)a1 - sfm3300_offset) / sfm3300_scale;
    FlowSens_Big += sensorReading;  // Convert the data from Sensor 1
  }
  float FlowSens_Big_Average = FlowSens_Big / numReadings1;
  return FlowSens_Big_Average;
}


float readAndAverageFlowSens_Small(int numReadings1) {
  
  float FlowSens_Small = 0;
  int sfm3400_scale = 800; // From data sheet
  int sfm3400_offset = 32768; // From data sheet
  int samps;

  for (samps = 0; samps < numReadings1; samps++) {

    delay(1); // Needed
    Wire1.requestFrom(0x40, 2);  // Request data from Sensor 2
    uint16_t a2 = Wire1.read();
    uint8_t b2 = Wire1.read();
    a2 = (a2 << 8) | b2;
    float sensorReading = ((float)a2 - sfm3400_offset) / sfm3400_scale;
    FlowSens_Small += sensorReading;  // Convert the data from Sensor 2

  }
  float FlowSens_Small_Average = FlowSens_Small / numReadings1;
  return FlowSens_Small_Average;
}

// float readAndAverageFlowSens_Big() {
//   delayMicroseconds(0.1 * total_delay_us);
//   Wire.requestFrom(0x40,2); // Request data from Sensor 1
//   uint16_t a1 = Wire.read();
//   uint8_t  b1 = Wire.read();
//   a1 = (a1 << 8) | b1;
//   float FlowSens_Big_Average = ((float)a1 - 32768) / 120; // Convert the data from Sensor 1
//   // Serial.print("Flow from Sensor 1: ");
//   // Serial.println(FlowSens_Big_Average);


//   return FlowSens_Big_Average;
// }



// float readAndAverageFlowSens_Big(numReadings) {

//   float FlowSens_Big;
//   int sfm3300_scale = 120;
//   int sfm3300_offset = 32768;
  
//   for (samps = 0; samps < numReadings; samps++) {
//     delayMicroseconds(1);
//     Wire.requestFrom(0x40,2); // Request data from Sensor 2
//     uint16_t a1 = Wire.read();
//     uint8_t  b1 = Wire.read();
//     a1 = (a1 << 8) | b1;
    
//     FlowSens_Big += ((float)a1 - sfm3300_offset) / sfm3300_scale; // Convert the data from Sensor 2
//     // Serial.print("Flow from Sensor 2: ");
//     // Serial.println(FlowSens_Small_Average);
//   }
//   float FlowSens_Big_Average = FlowSens_Big / rolling_avg;
  
//   return FlowSens_Big_Average;
// }



// float readAndAverageFlowSens_Small(int numReadings) {
//   int FlowSens_Small;
//   int sfm3400_scale = 800;
//   int sfm3400_offset = 32768;

//   for (samps = 0; samps < numReadings; samps++) {
//     delay(1);
//     Wire1.requestFrom(0x40,2); // Request data from Sensor 2
//     uint16_t a2 = Wire1.read();
//     uint8_t  b2 = Wire1.read();
//     a2 = (a2 << 8) | b2;
    
//     FlowSens_Small += ((float)a2 - sfm3400_offset) / sfm3400_scale; // Convert the data from Sensor 2
    
//     // Serial.print("Flow from Sensor 2: ");
//     // Serial.println(FlowSens_Small_Average);
//   }
//   float FlowSens_Small_Average = FlowSens_Small / rolling_avg;

//   return FlowSens_Small_Average;
// }




//   // Read from Sensor 2




// // Written for the 2024 Harvey Mudd College CowBell Labs Clinic Project
// // Contributers: Dominick Quaye, Allison Marten
// // Some code written by ChatGPT


// //Code starts here!!

// //Rando: #include
// //#include <avr/pgmspace.h>      // For flash storage

// //----------------------------------------------------------
// // NEW1 Pattern Input Setup
// //----------------------------------------------------------
// // For string parsing pattern input
// #include <iostream> 
// #include <vector> 
// #include <sstream> 
// #include <algorithm> // for std::remove_if 
// #include <string>
// // End string parsing libraries

// //----------------------------------------------------------
// // (end new1) Motor Setup Stuff
// //----------------------------------------------------------

// int numReadings = 1;
// int sampLen_ms = 100;


// // RunningAverage RA33(rolling_avg);
// // RunningAverage RA34(rolling_avg);


// bool scriptRunning = false; // Flag to indicate whether the script is running

// // float total_delay_us = 100;

// // --------------------------------------------------------
// //Flow Meter Setup Stuff
// //----------------------------------------------------------
// #include <Wire.h>
// #define SDA_2 25  // Secondary I2C Bus 
// #define SCL_2 26 // Secondary I2C Bus



// void setup() {
//   //--------------------------------------------------------
//   // Motor Setup 
//   // -------------------------------------------------------

//   Serial.begin(9600);
//   Serial.flush();

   
// }

// int firstLoopFlag = 0;
// float time_start_ms;
// // std::string desiredFlowPatternString;


// void loop() {sampLen_msntBigFlowRate, 3); // time for csv
//   Serial.print(" , ");
//   Serial.print(currentSmallFlowRate, 3); // time for csv
//   Serial.print(" , ");
//   Serial.print(total_flow, 3); // time for csv
//   Serial.println("");
//   delay(48);

//   // delayMicroseconds(100); // Just to give it some rest


// }

// float readAndAverageFlowSens_Big(int sampLen_ms1) { // *** Does this actually average?

//   float FlowSens_Big;
//   int sfm3300_scale = 120; // From data sheet
//   int sfm3300_offset = 32768; // From data sheet
//   int samps;
//   int samp_start_time = millis();
//   int samp_end_time = samp_start_time + sampLen_ms1; // Sample for 100 ms = 0.1 s

//   // for (samps = 0; samps < numReadings33; samps++) {
//   while (millis() <= samp_end_time) {
//     Wire.requestFrom(0x40, 2);  // Request data from Sensor 1
//     uint16_t a1 = Wire.read();
//     uint8_t b1 = Wire.read();
//     a1 = (a1 << 8) | b1;
//     FlowSens_Big += ((float)a1 - sfm3300_offset) / sfm3300_scale;  // Convert the data from Sensor 1
//     samps ++;
//     delay(1); // Needed
//   }

//   float FlowSens_Big_Average = FlowSens_Big / samps;
//   return FlowSens_Big_Average;

// }


// // float readAndAverageFlowSens_Small(int numReadings34) {
// float readAndAverageFlowSens_Small(int sampLen_ms1) {
  
//   float FlowSens_Small;
//   int sfm3400_scale = 800; // From data sheet
//   int sfm3400_offset = 32768; // From data sheet
//   int samps;
//   int samp_start_time = millis();
//   int samp_end_time = samp_start_time + sampLen_ms1; // Sample for 100 ms = 0.1 s

//   // for (samps = 0; samps < numReadings34; samps++) {
//   while (millis() <= samp_end_time) {
//     Wire1.requestFrom(0x40, 2);  // Request data from Sensor 2
//     uint16_t a2 = Wire1.read();
//     uint8_t b2 = Wire1.read();
//     a2 = (a2 << 8) | b2;
//     FlowSens_Small += ((float)a2 - sfm3400_offset) / sfm3400_scale;  // Convert the data from Sensor 2
//     samps ++;
//     delay(1); // Needed

//   }

//   float FlowSens_Small_Average = FlowSens_Small / samps;
//   return FlowSens_Small_Average;
  
// }


// //   // ----------------------------------------------------------
// //   // Flow Measurement Functions 
// //   // ----------------------------------------------------------


// // float readAndAverageFlowSens_Big(int numReadings1) { // *** Does this actually average?

// //   float FlowSens_Big = 0;
// //   int sfm3300_scale = 120; // From data sheet
// //   int sfm3300_offset = 32768; // From data sheet
// //   int samps;

// //   for (samps = 0; samps < numReadings1; samps++) {

// //     delay(1); // Needed
// //     Wire.requestFrom(0x40, 2);  // Request data from Sensor 1
// //     uint16_t a1 = Wire.read();
// //     uint8_t b1 = Wire.read();
// //     a1 = (a1 << 8) | b1;
// //     float sensorReading = ((float)a1 - sfm3300_offset) / sfm3300_scale;
// //     FlowSens_Big += sensorReading;  // Convert the data from Sensor 1
// //   }
// //   float FlowSens_Big_Average = FlowSens_Big / numReadings1;
// //   return FlowSens_Big_Average;
// // }


// // float readAndAverageFlowSens_Small(int numReadings1) {
  
// //   float FlowSens_Small = 0;
// //   int sfm3400_scale = 800; // From data sheet
// //   int sfm3400_offset = 32768; // From data sheet
// //   int samps;

// //   for (samps = 0; samps < numReadings1; samps++) {

// //     delay(1); // Needed
// //     Wire1.requestFrom(0x40, 2);  // Request data from Sensor 2
// //     uint16_t a2 = Wire1.read();
// //     uint8_t b2 = Wire1.read();
// //     a2 = (a2 << 8) | b2;
// //     float sensorReading = ((float)a2 - sfm3400_offset) / sfm3400_scale;
// //     FlowSens_Small += sensorReading;  // Convert the data from Sensor 2

// //   }
// //   float FlowSens_Small_Average = FlowSens_Small / numReadings1;
// //   return FlowSens_Small_Average;
// // }

// // float readAndAverageFlowSens_Big() {
// //   delayMicroseconds(0.1 * total_delay_us);
// //   Wire.requestFrom(0x40,2); // Request data from Sensor 1
// //   uint16_t a1 = Wire.read();
// //   uint8_t  b1 = Wire.read();
// //   a1 = (a1 << 8) | b1;
// //   float FlowSens_Big_Average = ((float)a1 - 32768) / 120; // Convert the data from Sensor 1
// //   // Serial.print("Flow from Sensor 1: ");
// //   // Serial.println(FlowSens_Big_Average);


// //   return FlowSens_Big_Average;
// // }



// // float readAndAverageFlowSens_Big(numReadings) {

// //   float FlowSens_Big;
// //   int sfm3300_scale = 120;
// //   int sfm3300_offset = 32768;
  
// //   for (samps = 0; samps < numReadings; samps++) {
// //     delayMicroseconds(1);
// //     Wire.requestFrom(0x40,2); // Request data from Sensor 2
// //     uint16_t a1 = Wire.read();
// //     uint8_t  b1 = Wire.read();
// //     a1 = (a1 << 8) | b1;
    
// //     FlowSens_Big += ((float)a1 - sfm3300_offset) / sfm3300_scale; // Convert the data from Sensor 2
// //     // Serial.print("Flow from Sensor 2: ");
// //     // Serial.println(FlowSens_Small_Average);
// //   }
// //   float FlowSens_Big_Average = FlowSens_Big / rolling_avg;
  
// //   return FlowSens_Big_Average;
// // }



// // float readAndAverageFlowSens_Small(int numReadings) {
// //   int FlowSens_Small;
// //   int sfm3400_scale = 800;
// //   int sfm3400_offset = 32768;

// //   for (samps = 0; samps < numReadings; samps++) {
// //     delay(1);
// //     Wire1.requestFrom(0x40,2); // Request data from Sensor 2
// //     uint16_t a2 = Wire1.read();
// //     uint8_t  b2 = Wire1.read();
// //     a2 = (a2 << 8) | b2;
    
// //     FlowSens_Small += ((float)a2 - sfm3400_offset) / sfm3400_scale; // Convert the data from Sensor 2
    
// //     // Serial.print("Flow from Sensor 2: ");
// //     // Serial.println(FlowSens_Small_Average);
// //   }
// //   float FlowSens_Small_Average = FlowSens_Small / rolling_avg;

// //   return FlowSens_Small_Average;
// // }




// //   // Read from Sensor 2













