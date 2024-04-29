// Written for the 2024 Harvey Mudd College CowBell Labs Clinic Project
// Contributers: Dominick Quaye, Allison Marten
// Some code written by ChatGPT


// ------------------------------------------------------------------------------
// Hardware Notes:
//------------------------------------------------------------------------------
//1: The Motor controlling the Big Valve, called "Big_Motor", should be on the  driver with Step/Dir pins connected to esp32 pins 15 and 2

//2: The Motor controlling the Small Valve, called "Small_Motor", should be on the driver with Step/Dir pins connected to esp32 pins 0 and 4

//3: The small flow sensor, called "FlowSens_Small" should be connected to the I2C bus on pins 25 and 26 of the esp32

//4: The Big flow sensor, called "FlowSens_Big" should be connected to the default I2C bus, on pins 21 and 22 of the esp32

//5: Drivers should have the ms1 and ms2 pins connected to 3.3V, which sets them to to 1/16 Stepping. As a result the 200 steps/rev motors then do 3200 steps/rev, set Gain values with this in mind.

//6: Motor_Small is controlling a valve that takes 10 Rotations to open, so should stay between values of 0 and 31,000, or 0 and -31,000 depending on how direction is set, leaving ~1000 steps or ~120 degree buffer for safety

//7: Motor_Big is controlling a valve that takes 6.5 Rotations to open, so it should stay between values of 0 and 18,200, or 0 and -18,200, leaving ~1000 steps or ~120 degree buffer for safety.

//------------------------------------------------------------------------------

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
#include <algorithm>  // for std::remove_if
#include <string>
#include <PID_v1.h>

// End string parsing libraries

float failFlowRate = 10987654321;  // Dummy variable to check if flow rate check has failed
float endFlowRate = 12345678910;   // Dummy variable to check if flow rate check has ended

// Forward function declaration
std::string getPatternFlowRate();
std::vector<std::vector<float>> stringToArray(const std::string& input);
float currentDesiredFlowRate(std::vector<std::vector<float>> flowPattern, int current_time_ms);
float readAndAverageFlowSens_Big(int numReadings1);
float readAndAverageFlowSens_Small(int numReadings1);
void performMotorOperation(int stepPin, int dirPin, String which_sensor, int incrementSteps);
float calculateMean(float arr[], int n);
float calculateStandardDeviation(float arr[], int n, float mean);
void waitForOnCommand();
void restartRun();
String whichSensor();

// Forward variable declaration
std::vector<std::vector<float>> desiredFlowPattern;
std::string desiredFlowPatternString;
float time_start_ms;
int firstLoopFlag;

//----------------------------------------------------------
// Motor Control Setup
//----------------------------------------------------------
// #include <AccelStepper.h>   // For Stepper
// AccelStepper* Big_Motor;    // Pointer
// AccelStepper* Small_Motor;  // Pointer
#define motorInterfaceType 1
#define currentPin 34
#define monitorPin 13
unsigned int pulseDelay = 1000;              // Microseconds
float voltageValues[100];                    // Array to store voltage values
int valueCount = 0;                          // Counter for the number of voltage values collected
float stdDevHistory[5] = { 0, 0, 0, 0, 0 };  // Buffer for Sdev Vals
int stdDevHistoryIndex = 0;                  // Buffer Index

bool scriptRunning = false;  // Flag to indicate whether the script is running


// --------------------------------------------------------
//Flow Meter Read Setup
//----------------------------------------------------------
#include <Wire.h>
#define SDA_2 25  // Secondary I2C Bus
#define SCL_2 26  // Secondary I2C Bus

//----------------------------------------------------------
// Wifi Control Setup Stuff
//----------------------------------------------------------

// TBD

// ----------------------------------------------------------
// P control Parameter Setup
// ----------------------------------------------------------
// Proportional Gain
// float KpS = 600; // This is my guess based on 45 PSI input
// float KpB = 52;
float bigMotorCutoff = 25.0;  // SLPM, Below this, the big valve won't engage.
//---------------------------------------
// Desired flow rate
float desiredFlowRate;  // Change, units SLPM
int Stop_Flag;          // Flag for big valve only going once
// ----------------------------------------------------------


// ----------------------------------------------------------
// PID Setup
// ----------------------------------------------------------

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
double Setpoint_big, Input_big, Output_big;
double Output_bigsmall;
int numReadings1 = 50; // For average


//Specify the links and initial tuning parameters
double KpS1 = 2, KiS = 5, KdS = 1;
PID PIDsmall(&Input, &Output, &Setpoint, KpS1, KiS, KdS, DIRECT);  // Trying Indirect because controlEffort = -controlEffort

double KpB1 = 2, KiB = 5, KdB = 1;
PID PIDbig(&Input_big, &Output_big, &Setpoint_big, KpB1, KiB, KdB, DIRECT);

PID PIDbigsmall(&Input_big, &Output_bigsmall, &Setpoint_big, KpB1, KiB, KdB, DIRECT);

// http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-direction/

// DIRECT: If the Input is above the Setpoint, the Output goes LOWER.
// If the speed of the car is above the desired speed set the throttle point lower.

// REVERSE: If the Input is above the Setpoint the Output goes HIGHER.
// If the temperature of the refrigerator is above the desired temperature, turn on the compressor more often or longer to bring the temperature down.

void setup() {
  //--------------------------------------------------------
  // Motor Setup
  // -------------------------------------------------------
  pinMode(4, OUTPUT);   // Small_Motor 1 Step pin
  pinMode(0, OUTPUT);   // Small_Motor 1 Dir pin
  pinMode(2, OUTPUT);   // Big_Motor 2 Step pin
  pinMode(15, OUTPUT);  // Big_Motor 2 Dir pin
  pinMode(monitorPin, OUTPUT);
  Serial.begin(9600);
  Serial.flush();
  Serial.println("Code start.");
  // --------------------------------------------------------
  // Motor Calibration, comment out this sec below to skip
  // --------------------------------------------------------
  // delay(100);  // Just giving time to open monitor
  // Serial.println("Calibrating....");
  // digitalWrite(0, HIGH);               // Change to LOW if needed for Big_Motor
  // performMotorOperation(2, 15, 0.02);  // Calibrate Big_Motor 1 with threshold
  // delay(200);
  // Serial.println("Big_Motor Calibrated");
  // delay(200);
  // digitalWrite(15, HIGH);                                     // Change to LOW if needed for Big_motor
  // float stdDevHistory[5] = { 0.02, 0.02, 0.02, 0.02, 0.02 };  // Reset Buffer
  // performMotorOperation(4, 0, 0.04);                          // Calibrate Small_Motor with threshold
  // delay(200);
  // Serial.println("Small_Motor Calibrated");
  // delay(200);

  // ----------------------------------------------------------
  // Calibration, comment out this sec above to skip  ^^^^^
  // ----------------------------------------------------------

  // Motor setup (non-Calibration)

  // Big_Motor = new AccelStepper(motorInterfaceType, 2, 15);
  // Big_Motor->setMaxSpeed(5000);     // Maximum steps per second
  // Big_Motor->setAcceleration(3000);  // Steps per second squared
  // Big_Motor->setCurrentPosition(0);

  // Small_Motor = new AccelStepper(motorInterfaceType, 4, 0);
  // Small_Motor->setMaxSpeed(5000);     // Maximum steps per second
  // Small_Motor->setAcceleration(3000);  // Steps per second squared
  // Small_Motor->setCurrentPosition(0);

  // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

  // ----------------------------------------------------------
  // Dual Flow Read Setup
  // ----------------------------------------------------------

  // Initialize the default I2C bus for Flowsens_Big
  Wire.begin();

  // Initialize the secondary I2C bus for Flowsens_Small
  Wire1.begin(SDA_2, SCL_2);


  Wire.beginTransmission(byte(0x40));
  Wire.write(byte(0x10));  // Example command to Flowsens_Big
  Wire.write(byte(0x00));  // Example data to Flowsens_Big
  Wire.endTransmission();

  Wire1.beginTransmission(byte(0x40));
  Wire1.write(byte(0x10));  // Example command to Flowsens_Small
  Wire1.write(byte(0x00));  // Example data to Flowsens_Small
  Wire1.endTransmission();

  // ----------------------------------------------------------
  //
  // End of Setup!
  //
  // ----------------------------------------------------------
  // Serial.println("Mass Flow Controller Ready, Starting Control Loop in 1 Second...");
  delay(1000);
  //-----------------------------------------------------------
  Serial.println("***");  // Filler lines for for MATLAB csv read code
  Serial.println("***");
  Serial.println("***");

  Serial.println("MAKE SURE ALL VALVES ARE CLOSED!!!!!!");

  Serial.println("Type 'on' to start the script...");
  waitForOnCommand();

  int firstLoopFlag = 0;
  // std::string desiredFlowPatternString;

  // ----------------------------------------------------------
  // PID
  // ----------------------------------------------------------


  // //initialize the variables we're linked to
  Input = 0;
  Setpoint = 0;



  //turn the PID on
  PIDsmall.SetMode(AUTOMATIC);
  PIDbig.SetMode(AUTOMATIC);
  PIDbigsmall.SetMode(AUTOMATIC);

  KpB1 = 60;
  KiB = 0.002;
  KdB = 2;

  KpS1 = 900;
  KiS = 0.5;
  KdS = 8;


  PIDsmall.SetTunings(KpS1, KiS, KdS);
  PIDsmall.SetOutputLimits(-10000000, 10000000);

  PIDbig.SetTunings(KpB1, KiB, KdB);
  PIDbig.SetOutputLimits(-10000000, 10000000);

  PIDbigsmall.SetTunings(2.0, 0, 0);
  PIDbigsmall.SetOutputLimits(-10000000, 10000000);
}

// Serial.println("Setup Done, starting Loop");



void loop() {
  // Serial.print("firstLoopFlag: ");
  // Serial.println(firstLoopFlag);

  // ----------------------------------------------------------
  // NEW1 Checking pattern array to get current flow rate
  // ----------------------------------------------------------
  int currPos;
  int newPos;
  float flow;



  if (firstLoopFlag == 0) {
    desiredFlowPatternString = getPatternFlowRate();               // Get user-inputted string of flow rates and durations, ex. "[200,15;0.03,5;70,10]"
    desiredFlowPattern = stringToArray(desiredFlowPatternString);  // Convert string to actual array
    Serial.println("data_begin");
    // Serial.println("time_ms , set_point , flow_3300 , flow_3400 , total , control_effort , new_goal_position");
    delay(1000); // **** Give time to switch to data collection program, DELETE later if needed
    time_start_ms = millis();
  }

  String which_sensor = whichSensor();

  int n = 0;
  int stepsPerTurn = 3200;
  int incrementSteps;
  int crashFlag = 1;
  int delayBetween;
  float currentFlowRate;
  String dataPrintSweep;
  float flowRateRead;
  delay(1000);
  Serial.println(3);
  delay(2000);
  Serial.println(2);
  delay(2000);
  Serial.println(1);
  delay(2000);
  Serial.print("Averaging window:");
  Serial.println(numReadings1);
  Serial.println("time_ms , sensor , currpos , mflow");


  while (crashFlag)  { 
    if (n < 5 * stepsPerTurn) {
      incrementSteps = 400;
      delayBetween = 3000;
    }
    else if (n >= 5 * stepsPerTurn && n <= 8.25 * stepsPerTurn) { // Cutoff when flow meter would be at 26 (max reading for 20 SLPM alicat)
      incrementSteps = 800;
      delayBetween = 3000;
    }
    else if (n > 8.25 * stepsPerTurn) {
      incrementSteps = 0;
      delayBetween = 3000;
      crashFlag = 1;
      Serial.print(n);
      Serial.println(" steps done, stopping.");
      esp_restart();
    }

    n += incrementSteps;
    
    // currPos = Small_Motor->currentPosition();
    // newPos = currPos - incrementSteps;
    // Small_Motor->moveTo(newPos);

    // int motorSteps = 0;
    // int move_start_time = millis();

    // while (Small_Motor->distanceToGo() != 0) {
    //   Small_Motor->run();

    //   if ((millis() - move_start_time) % 50 == 0) {
    //     sweepDataAndPrint(which_sensor);
    //   }
    //   // Serial.println(motorSteps);

    //   // motorSteps ++;
    // }

    int dpoints = 0;

    performMotorOperation(4, 0, which_sensor, incrementSteps);
    
    int timerVal;
    int samps1 = 0;
    float flow;
    
    while (dpoints <= delayBetween / numReadings1) {
      flow = 0;
      timerVal = millis();
      samps1 = 0;
      while (samps1 <= numReadings1) {
        // get data to print
        if (which_sensor == "b") {
          flow += readAndAverageFlowSens_Big(1);
          delay(0.2);
        }
        else if (which_sensor == "s") {
          flow += readAndAverageFlowSens_Small(1);
          delay(0.2);
        }
        samps1 ++;
      }
      float flowAvg = flow/numReadings1;
      String dataToPrint = String(millis()) + " , " + which_sensor + " , " + String(flowAvg, 6);
      Serial.println(dataToPrint);        
      dpoints ++;
    }
/////
  int timeStartStop = micros();
  while micros() < (timeStartStop + (1000*delayBetween))) {
    
    if (which_sensor == "b") {
      flow += readAndAverageFlowSens_Big(1);
    }
    else if (which_sensor == "s") {
      flow += readAndAverageFlowSens_Small(1);
    }
    
    if (samps % numReadings == 0) {
      flowAvg = flow / numReadings;
      String dataToPrint = String(millis()) + " , " + which_sensor + " , " + String(flowAvg, 6);
      Serial.println(dataToPrint);
      flow = 0;


  }

  }

}


// ----------------------------------------------------------
// P-Control The Big Motor!!!
// ----------------------------------------------------------



// ----------------------------------------------------------
// Flow Measurement Functions
// ----------------------------------------------------------

void sweepDataAndPrint(String which_sensor) {
  float timerVal = millis();
  float flow;
  
  // get data to print
  if (which_sensor == "b") {
    flow = readAndAverageFlowSens_Big(numReadings1);
  }
  else if (which_sensor == "s") {
    flow = readAndAverageFlowSens_Small(numReadings1);
  }
  String dataToPrint = String(millis()) + " , " + which_sensor + " , " + " , " + String(flow, 6);
  Serial.println(dataToPrint);

  delay(50 - (millis()-timerVal));
}

float readAndAverageFlowSens_Big(int numReadings33) { // *** Does this actually average?

  float FlowSens_Big = 0;
  int sfm3300_scale = 120; // From data sheet
  int sfm3300_offset = 32768; // From data sheet
  int samps;

  for (samps = 0; samps < numReadings33; samps++) {
    delay(0.8); // Needed
    Wire.requestFrom(0x40, 2);  // Request data from Sensor 1
    uint16_t a1 = Wire.read();
    uint8_t b1 = Wire.read();
    a1 = (a1 << 8) | b1;
    float sensorReading = ((float)a1 - sfm3300_offset) / sfm3300_scale;
    FlowSens_Big += sensorReading;  // Convert the data from Sensor 1
  }

  float FlowSens_Big_Average = FlowSens_Big / numReadings33;
  return FlowSens_Big_Average;
}


float readAndAverageFlowSens_Small(int numReadings34) {
  
  float FlowSens_Small = 0;
  int sfm3400_scale = 800; // From data sheet
  int sfm3400_offset = 32768; // From data sheet
  int samps;

  for (samps = 0; samps < numReadings34; samps++) {
    delay(0.8); // Needed
    Wire1.requestFrom(0x40, 2);  // Request data from Sensor 2
    uint16_t a2 = Wire1.read();
    uint8_t b2 = Wire1.read();
    a2 = (a2 << 8) | b2;
    float sensorReading = ((float)a2 - sfm3400_offset) / sfm3400_scale;
    FlowSens_Small += sensorReading;  // Convert the data from Sensor 2    delay(1); // Needed
  }

  float FlowSens_Small_Average = FlowSens_Small / numReadings34;
  return FlowSens_Small_Average;
}


// ----------------------------------------------------------
// Initial Motor Calibration Functions below
// ----------------------------------------------------------

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void performMotorOperation(int stepPin, int dirPin, String which_sensor, int incrementSteps) {
  
  int samps = 1;
  float flow = 0;
  int numReadings = 50;
  int numSteps = 0;
  float flowAvg;
  unsigned long timerVal;

  while (numSteps <= incrementSteps/2) {  // Start of the  loop
    // Generate a step pulse
    digitalWrite(stepPin, HIGH);
    digitalWrite(monitorPin, HIGH);
    delayMicroseconds(pulseDelay);  // Pulse duration
    digitalWrite(stepPin, LOW);
    digitalWrite(monitorPin, LOW);
    timerVal = micros();  
    // get data to print
    if (which_sensor == "b") {
      flow += readAndAverageFlowSens_Big(1);
    }
    else if (which_sensor == "s") {
      flow += readAndAverageFlowSens_Small(1);
    }
    delayMicroseconds(pulseDelay - (micros() - timerVal));  // Maintain PWM timing

    digitalWrite(stepPin, HIGH);
    digitalWrite(monitorPin, HIGH);
    delayMicroseconds(pulseDelay);  // Pulse duration
    digitalWrite(stepPin, LOW);
    digitalWrite(monitorPin, LOW);


    if (samps % (numReadings/2) == 0) {
      flowAvg = flow / numReadings;
      String dataToPrint = String(millis()) + " , " + which_sensor + " , " + String(flowAvg, 6);
      Serial.println(dataToPrint);
      flow = 0;
    }

    samps ++;
    numSteps ++;
    // Serial.print("numSteps: ");
    // Serial.println(numSteps);

    delayMicroseconds(pulseDelay - (micros() - timerVal));  // Maintain PWM timing
  }                                                         // End of the infinite loop

  // Serial.println("Done with moving motor");
}


float readAndAverage() {
  const int numReadings = 5;
  float total = 0.0;
  for (int i = 0; i < numReadings; i++) {
    total += analogRead(currentPin);
    delayMicroseconds(4);  // Small delay to stabilize ADC reading
  }
  return (total / numReadings) * (3.3 / 4095.0);  // Convert to voltage
}

float calculateMean(float arr[], int n) {
  float sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += arr[i];
  }
  return sum / n;
}

float calculateStandardDeviation(float arr[], int n, float mean) {
  float sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += pow(arr[i] - mean, 2);
  }
  return sqrt(sum / (n - 1));
}


void waitForOnCommand() {
  while (!scriptRunning && !Serial.available())
    ;                                           // Wait for input
  String input = Serial.readStringUntil('\n');  // Read input
  if (input == "on") {
    Serial.println("Script started.");
    // scriptRunning = true;
    // Add your script start code here
  } else {
    Serial.println("Invalid command. Please type 'on' to start the script.");
    waitForOnCommand();  // Call the function recursively until valid input is received
  }
}


// ----------------------------------------------------------
// Functions for flow pattern input and string parsing
// ----------------------------------------------------------

std::string getPatternFlowRate() {
  Serial.println("");
  Serial.println("Input your flow pattern.");
  Serial.println("");
  Serial.println("Flow rates in SLPM, duration in minutes.");
  Serial.println("");
  Serial.println("Flow patterns should be of the form [flow rate 1 , duration 1 ; flow rate 2 , duration 2 ; .... ; flow rate n , duration n].");
  Serial.println("");
  Serial.println("All parameters must be valid numbers. Spaces around , and ; are optional.");
  Serial.println("");
  while (!scriptRunning && !Serial.available())
    ;  // Wait for input

  String inputString = Serial.readStringUntil('\n');  // Read input
  std::string desiredFlowPattern = inputString.c_str();
  if (desiredFlowPattern[0] == '[') {
    Serial.print("You inputted the following flow pattern: ");
    Serial.println(desiredFlowPattern.c_str());  // Convert to const char* for println    scriptRunning = true;
    // Add your script start code here
  } else {
    Serial.println("Your input was not accepted. Please enter a string in the provided form.");
    getPatternFlowRate();  // Call the function recursively until valid input is received
  }
  return desiredFlowPattern;
}


String whichSensor() {
  Serial.println("");
  Serial.println("b for big sensor s for small sensor.");
 
  while (!scriptRunning && !Serial.available())
    ;  // Wait for input

  String inputString = Serial.readStringUntil('\n');  // Read input
  String which_sensor = inputString;
  
  return which_sensor;
}


std::vector<std::vector<float>> stringToArray(const std::string& input) {
  std::vector<std::vector<float>> result;


  // Remove the leading '[' and trailing ']' from the input string
  std::string trimmedInput = input.substr(1, input.size() - 2);

  // Use stringstream to parse the string
  std::stringstream ss(trimmedInput);
  std::string token;

  // Parse by delimiter ';'
  while (std::getline(ss, token, ';')) {
    std::vector<float> row;
    std::stringstream row_ss(token);
    std::string element;
    bool isSecondColumn = false;  // Flag to track if we are parsing the second column

    // Parse each element of the row by delimiter ','
    while (std::getline(row_ss, element, ',')) {
      // Remove spaces from the element
      element.erase(std::remove_if(element.begin(), element.end(), ::isspace), element.end());

      // Convert string to float
      float value = std::stof(element);

      // If this is the second column, convert minutes to milliseconds
      if (isSecondColumn) {
        value *= 60000;  // Convert minutes to milliseconds
      }

      row.push_back(value);

      isSecondColumn = !isSecondColumn;  // Toggle flag for the next iteration
    }
    result.push_back(row);
  }

  return result;
}



float currentDesiredFlowRate(std::vector<std::vector<float>> flowPattern, int current_time_ms) {

  float desiredFlowRate;
  float flowStartTime = 0;
  float flowEndTime;
  int numRows = flowPattern.size();
  for (int i = 0; i <= numRows - 1; ++i) {
    // Serial.print("check time: ");
    // Serial.println(flowPattern[i][1]);
    float flowEndTime = flowStartTime + flowPattern[i][1];
    if (current_time_ms >= flowStartTime && current_time_ms <= flowEndTime) {
      desiredFlowRate = flowPattern[i][0];
      // Stop_Flag = 0;
      // std::cout << "desiredFlowRate: " << desiredFlowRate << "; time: " << current_time_ms << std::endl;
      return desiredFlowRate;
    } else {
      flowStartTime = flowEndTime;
    }
  }
  if (current_time_ms > flowEndTime) {
    return endFlowRate;
  }
  return failFlowRate;
}


void printArray(const std::vector<std::vector<float>>& array) {
  for (const auto& row : array) {
    Serial.print("{");
    for (size_t i = 0; i < row.size(); ++i) {
      Serial.print(row[i]);
      if (i < row.size() - 1) {
        Serial.print(", ");
      }
    }
    Serial.print("}");
    if (&row != &array.back()) {
      Serial.print(", ");
    }
  }
  Serial.println();
}


void restartRun() {
  // Serial.println("Closing Motors to Restart");

  // // Close valves
  // Big_Motor->moveTo(0);
  // while (Big_Motor->distanceToGo() != 0) {
  //   Big_Motor->run();
  // }

  // Small_Motor->moveTo(0);  // Set the stepper to the new position
  // // Perform the steps
  // while (Small_Motor->distanceToGo() != 0) {
  //   Small_Motor->run();
  // }
  // Serial.println("Motors closed. Returning to Setup.");

  // esp_restart();  // Same as pressing EN or power cycling
}



double getGainConst() {
  Serial.println("Waiting for Input");
  while (!scriptRunning && !Serial.available())
    ;                                           // Wait for input
  String input = Serial.readStringUntil('\n');  // Read input
  double K_new = atof(input.c_str());
  // if (liveDesiredFlowRate > 0.0) {
  Serial.print("New constant:");
  Serial.println(K_new);
  //scriptRunning = true;
  // Add your script start code here
  // } else {
  //   Serial.println("Please type a number greater than zero to start the script.");
  //   waitForOnCommand(); // Call the function recursively until valid input is received
  // }
  return K_new;
}