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
#include <algorithm> // for std::remove_if 
#include <string>
// End string parsing libraries

float failFlowRate = 10987654321; // Dummy variable to check if flow rate check has failed
float endFlowRate = 12345678910; // Dummy variable to check if flow rate check has ended

// Forward function declaration
std::string getPatternFlowRate();
std::vector<std::vector<float>> stringToArray(const std::string& input);
float currentDesiredFlowRate(std::vector<std::vector<float>> flowPattern, int current_time_ms);
void controlBigMotor(float targetFlow);
float readAndAverageFlowSens_Big();
float readAndAverageFlowSens_Small();
void performMotorOperation(int stepPin, int dirPin, float threshold);
float readAndAverage();
float calculateMean(float arr[], int n);
float calculateStandardDeviation(float arr[], int n, float mean);
void waitForOnCommand();

// Forward variable declaration
std::vector<std::vector<float>> desiredFlowPattern;
std::string desiredFlowPatternString;

//----------------------------------------------------------
// (end new1) Motor Setup Stuff
//----------------------------------------------------------
#include <AccelStepper.h>    // For Stepper 
AccelStepper *Big_Motor; // Pointer 
AccelStepper *Small_Motor; // Pointer 
#define motorInterfaceType 1
#define currentPin 34
#define monitorPin 13
unsigned int pulseDelay = 1000; // Microseconds
float voltageValues[100]; // Array to store voltage values
int valueCount = 0; // Counter for the number of voltage values collected
float stdDevHistory[5] = {0, 0, 0, 0, 0}; // Buffer for Sdev Vals
int stdDevHistoryIndex = 0; // Buffer Index


bool scriptRunning = false; // Flag to indicate whether the script is running


// --------------------------------------------------------
//Flow Meter Setup Stuff
//----------------------------------------------------------
#include <Wire.h>
#define SDA_2 25  // Secondary I2C Bus 
#define SCL_2 26 // Secondary I2C Bus
// float FlowSens_Big_Average = 0; //Declare Globally 
// float FlowSens_Small_Average = 0; //Declare Globally 
//----------------------------------------------------------
// Wifi Control Setup Stuff
//---------------------------------------------------------- 

// TBD

// ----------------------------------------------------------
// P control Parameter Setup
// ----------------------------------------------------------
// Proportional Gain
float KpS = 600; // This is my guess based on 45 PSI input 
float KpB = 60;
float bigMotorCutoff = 25.0; // SLPM, Below this, the big valve won't engage.
//---------------------------------------
// Desired flow rate
float desiredFlowRate = 0; // Change, units SLPM
int Stop_Flag = 0; // Flag for big valve only going once 
// ----------------------------------------------------------


void setup() {
  //--------------------------------------------------------
  // Motor Setup 
  // -------------------------------------------------------
  pinMode(4, OUTPUT); // Small_Motor 1 Step pin
  pinMode(0, OUTPUT); // Small_Motor 1 Dir pin
  pinMode(2, OUTPUT); // Big_Motor 2 Step pin
  pinMode(15, OUTPUT); // Big_Motor 2 Dir pin
  pinMode(monitorPin, OUTPUT);
  Serial.begin(9600);
  Serial.flush();
  Serial.println("Code start.");
  // --------------------------------------------------------
  // Motor Calibration, comment out this sec below to skip 
  // --------------------------------------------------------
  //delay(100); // Just giving time to open monitor
 // Serial.println("Calibrating....");
  //  digitalWrite(0, HIGH); // Change to LOW if needed for Big_Motor
  //  performMotorOperation(2, 15, 0.02); // Calibrate Big_Motor 1 with threshold
  //  delay(200);
  //  Serial.println("Big_Motor Calibrated");
  //  delay(200);
   // digitalWrite(15, HIGH); // Change to LOW if needed for Big_motor
 //   float stdDevHistory[5] = {0.02, 0.02, 0.02, 0.02, 0.02}; // Reset Buffer
  //  performMotorOperation(4, 0, 0.04); // Calibrate Small_Motor with threshold
  //  delay(200);
  //  Serial.println("Small_Motor Calibrated");
  //  delay(200);

  // ----------------------------------------------------------
  // Calibration, comment out this sec above to skip  ^^^^^
  // ----------------------------------------------------------

  // Motor setup (non-Calibration)

  Big_Motor = new AccelStepper(motorInterfaceType, 2, 15);
  Big_Motor->setMaxSpeed(2000); // Maximum steps per second
  Big_Motor->setAcceleration(600); // Steps per second squared 
  Big_Motor->setCurrentPosition(0);

  Small_Motor = new AccelStepper(motorInterfaceType, 4, 0);
  Small_Motor->setMaxSpeed(2000); // Maximum steps per second 
  Small_Motor->setAcceleration(600); // Steps per second squared
  Small_Motor->setCurrentPosition(0);

  // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

  // ----------------------------------------------------------
  // Dual Flow Read Setup Stuff 
  // ----------------------------------------------------------


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

  // ----------------------------------------------------------
  //
  // End of Setup!
  //
  // ----------------------------------------------------------
 // Serial.println("Mass Flow Controller Ready, Starting Control Loop in 1 Second...");
  delay(1000);
  //-----------------------------------------------------------
  Serial.println("***"); // Filler lines for for MATLAB csv read code
  Serial.println("***");
  Serial.println("***");
  
  Serial.println("MAKE SURE ALL VALVES ARE CLOSED!!!!!!");

  Serial.println("Type 'on' to start the script...");
  waitForOnCommand();


  // ----------------------------------------------------------
  // NEW1 Pattern input function calls
  // ----------------------------------------------------------

  // Serial.println("Enter desired flow rate (SLPM).");

  // Ask for flow pattern input
  // std::string desiredFlowPatternString = getPatternFlowRate();
  // Convert input string to proper array
  // desiredFlowPattern = stringToArray(desiredFlowPatternString);
  

  // ----------------------------------------------------------
  // (end new1) Print headers for serial output
  // ----------------------------------------------------------
  
}

int firstLoopFlag = 0;
float time_start_ms;
// std::string desiredFlowPatternString;


void loop() {
  // Serial.print("firstLoopFlag: ");
  // Serial.println(firstLoopFlag);

// ----------------------------------------------------------
// NEW1 Checking pattern array to get current flow rate
// ----------------------------------------------------------


  if (firstLoopFlag == 0) {
     desiredFlowPatternString = getPatternFlowRate(); // Input string of flow rates and durations
    //  Serial.println("desiredFlowPatternString");
    //  Serial.println(desiredFlowPatternString.c_str());
     desiredFlowPattern = stringToArray(desiredFlowPatternString); // Convert string to actual array
     Serial.println("data_begin");
     Serial.println("time_ms, set_point, flow_3300, flow_3400, total, control_effort, new_goal_position");
     time_start_ms = millis();
  }


  // Check flow pattern and get current flow rate
  // Serial.println("time_start_ms");
  // Serial.println(time_start_ms);
  float time_check_ms = millis() - time_start_ms;
  
  // Serial.println("desiredFlowPattern:");
  // printArray(desiredFlowPattern);
  // Serial.println("Done printing desiredFlowPattern in loop:");
  float oldDesiredFlowRate = desiredFlowRate;

  float desiredFlowRate = currentDesiredFlowRate(desiredFlowPattern, time_check_ms);

  if (oldDesiredFlowRate != desiredFlowRate) {Stop_Flag = 0;} // So the big valve re-checks if it should move every time flow rate changes

  if (desiredFlowRate == endFlowRate) { // Set a condition if desiredFlowRate is 0?
    std::cout << "FLOW PATTERN ENDED AT " << time_check_ms << " ms, CLOSING VALVES" << std::endl;
    // Close valves
    Big_Motor->moveTo(0); 
    while (Big_Motor->distanceToGo() != 0) {
      Big_Motor->run();
    }

    Small_Motor->moveTo(0);   // Set the stepper to the new position
    // Perform the steps
    while (Small_Motor->distanceToGo() != 0) {
      Small_Motor->run();
    abort();
    while(1);  // Infinite loop to halt the program
  }
  }
  else if (desiredFlowRate == failFlowRate) {
    std::cout << "FAILURE TO MATCH TIME AT " << time_check_ms << " ms, CLOSING VALVES" << std::endl;

    // Close valves
    Big_Motor->moveTo(0); 
    while (Big_Motor->distanceToGo() != 0) {
      Big_Motor->run();
    }

    Small_Motor->moveTo(0);   // Set the stepper to the new position
    // Perform the steps
    while (Small_Motor->distanceToGo() != 0) {
      Small_Motor->run();
    }
    abort();
    while(1);  // Infinite loop to halt the program
  }

  // ----------------------------------------------------------
  // (end new1)
  // ----------------------------------------------------------

  // This first if statment moves the big motor to open the big valve for any desired flow above 25 SLM

  // P control for the big motor until it is within 10% of the setpoint - 10 SLPM
  // Serial.println(Stop_Flag);
  if (Stop_Flag == 0 && desiredFlowRate > bigMotorCutoff) {
    controlBigMotor(constrain(desiredFlowRate - 10, 0, 250)); // If the big motor goal flow rate is above 250, set it to 250
    Stop_Flag = 1;
    delay(3000);
  }


  // if (Stop_Flag == 0 && desiredFlowRate > bigMotorCutoff) {
  //   long BigMotorPosition = ((desiredFlowRate-10)/50)*3200; // 10 here is offset so it leaves last 10 SLM up to small valve, 50 is a assumtion that 1 turn = 50slm output (based on data at 45 PSI), and 3000 is slightly conservative steps/rev estimate 
  //   // Now the motor moves!
  //   BigMotorPosition= -BigMotorPosition;
  //   Big_Motor->moveTo(BigMotorPosition); 
  //   while (Big_Motor->distanceToGo() != 0) {
  //     Big_Motor->run();
  //   }
  //   Stop_Flag=1;
  //   delay(3000);
  // }

  

  float FlowSens_Big_Average = readAndAverageFlowSens_Big();
  float FlowSens_Small_Average = readAndAverageFlowSens_Small();

  float currentBigFlowRate = FlowSens_Big_Average; // Updates Flow 
  float currentSmallFlowRate = FlowSens_Small_Average; // Updates Flow 

  // ----------------------------------------------------------
  // NEW2 Eliminating 0.0333 float on SFM3300
  // ----------------------------------------------------------
  if (desiredFlowRate<bigMotorCutoff && abs(currentBigFlowRate) <= 0.04 && abs(currentBigFlowRate) >= 0.02 ) { // SFM3300 increments in 0.033333... and floats a value of 0.033333...
    // Potential issue: how to differentiate an actual 0.033333 leak from a float? Check that before and after values are 0? Don't want to have to save old values...
    // For now, will just wipe it out if it's between 0.02 and 0.04
    // It the first three values SFM3300 can give are 0, 0.0333333 and 0.0666667
    currentBigFlowRate = 0.0; 
    }
  // ----------------------------------------------------------
  // (end new2)
  // ----------------------------------------------------------
  
  float total_flow= currentSmallFlowRate + currentBigFlowRate;

  float error = desiredFlowRate - (total_flow);  // Calculate error

  long controlEffort = long(KpS * error);   // Calculate control effort 
  
  controlEffort = -controlEffort;
  
  long newgoalposition = Small_Motor->currentPosition() + controlEffort;
  long currpos = Small_Motor->currentPosition();



  // Serial.println("");
  // Serial.print("  Big sensor flow rate:  ");
  // Serial.println(currentBigFlowRate); // time for csv
  // delay(100); 
  // Serial.print("Small sensor flow rate:  ");
  // Serial.println(currentSmallFlowRate); // time for csv
  // delay(100); 
  // Serial.print("       Total flow rate:  ");
  // Serial.println(total_flow);
  // delay(100); 
  // Serial.print("        Control effort:  ");
  // Serial.println(controlEffort); // time for csv
  // delay(100); 
  // Serial.print("     New goal position:  ");
  // Serial.println(newgoalposition); // time for csv
  // delay(100); 


  Serial.print(time_check_ms); // NEW1, so the time printed reflects time the flow is checked against
  Serial.print(" , ");
  Serial.print(desiredFlowRate, 3);
  Serial.print(" , ");
  Serial.print(currentBigFlowRate, 3); // time for csv
  Serial.print(" , ");
  Serial.print(currentSmallFlowRate, 3); // time for csv
  Serial.print(" , ");
  Serial.print(total_flow, 3); // time for csv
  Serial.print(" , ");
  Serial.print(controlEffort); // time for csv
  Serial.print(" , ");
  Serial.println(newgoalposition); // time for csv


  // Future crash Check
  if (newgoalposition > 0  || newgoalposition < -31000) {
        Serial.println(newgoalposition);
        Serial.println("Crash Course detected!...Motor stopped, Please restart program!");
        while(1);  // Infinite loop to halt the program
    }
  // delay(100); 

  // Print out all the needed info before moving the motor
  // Serial.print(millis()); // time for csv
  // Serial.print(" , ");
  // Serial.print(String(currentBigFlowRate)); // time for csv
  // Serial.print(" , ");
  // Serial.print(String(currentSmallFlowRate)); // time for csv
  // Serial.print(" , ");
  // Serial.print(String(total_flow)); // time for csv
  // Serial.print(" , ");
  // Serial.print(String(desiredFlowRate)); // time for csv
  // Serial.print(" , ");
  // Serial.print(String(error)); // error for csv
  // Serial.print(" , ");
  // Serial.print(String(Kp));
  // Serial.print(" , ");
  // Serial.print(String(controlEffort)); // error for csv
  // Serial.print(" , ");
  // Serial.print(String(currpos)); // for csv
  // Serial.print(" , ");
  // Serial.print(String(newgoalposition)); // for csv
  // Serial.println();
  // Print out all the needed info before moving the motor

  Small_Motor->moveTo(newgoalposition);   // Set the stepper to the new position
  // Perform the steps
  while (Small_Motor->distanceToGo() != 0) {
    Small_Motor->run();
  }
 
 
  firstLoopFlag = 1; //NEW1



  delay(100); // Just to give it some rest


}


  // ----------------------------------------------------------
  // P-Control The Big Motor!!! 
  // ----------------------------------------------------------
  
void controlBigMotor(float targetFlow) {

  if (targetFlow == 250) {targetFlow = 277.778;} // *****So the big motor doesn't try to go above 250 SLPM
float currentBigFlowRate = 0;
float error = targetFlow; // **** POSSIBLE ISSUE WHEN CHANGING FLOW RATES
long controlEffort = 0;
long newPosition = 0;

  while (abs(error) > targetFlow * 0.1) {
    currentBigFlowRate = readAndAverageFlowSens_Big();
    error = targetFlow - currentBigFlowRate;
    controlEffort = long(KpB* error);
    controlEffort = -controlEffort;
    newPosition = Big_Motor->currentPosition() + controlEffort;

    // Crash check
    if (newPosition > 0 || newPosition < -18200) { 
      Serial.println(newPosition);
      Serial.println("Crash Course detected!...Motor stopped, Please restart program!");
      while (1); // Infinite loop to halt the program
    }

    Big_Motor->moveTo(newPosition);

    while (Big_Motor->distanceToGo() != 0) {
      Big_Motor->run();
    }

    delay(100);
  }
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





  // ----------------------------------------------------------
  // Initial Motor Calibration Functions below 
  // ----------------------------------------------------------


  // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void performMotorOperation(int stepPin, int dirPin, float threshold) {
  while (true) { // Start of the infinite loop
    // Generate a step pulse
    digitalWrite(stepPin, HIGH);
    digitalWrite(monitorPin, HIGH);
    delayMicroseconds(1000); // Pulse duration
    digitalWrite(stepPin, LOW);
    digitalWrite(monitorPin, LOW);

    unsigned long timerVal = micros();

    // Read and average the current sensor value
    float voltage = readAndAverage();

    // Check if we have collected less than 100 values
    if (valueCount < 100) {
      voltageValues[valueCount++] = voltage;
    }

    // Once 100 values are collected, calculate and print the standard deviation
    if (valueCount == 100) {
      float mean = calculateMean(voltageValues, 100);
      float standardDeviation = calculateStandardDeviation(voltageValues, 100, mean);
      // Serial.println(standardDeviation);

      // Update circular buffer with the latest standard deviation value
      stdDevHistory[stdDevHistoryIndex] = standardDeviation;
      stdDevHistoryIndex = (stdDevHistoryIndex + 1) % 5; // Move to the next position in a circular manner

      // Check if all of the last 5 standard deviation values are greater than the threshold
      bool allGreaterThanThreshold = true;
      for (int i = 0; i < 5; i++) {
        if (stdDevHistory[i] <= threshold) { // Modified to use threshold parameter
          allGreaterThanThreshold = false;
          break;
        }
      }

      if (allGreaterThanThreshold) {
        valueCount = 0;
        break; // Exit the loop if all of the last 5 values are greater than the threshold
      }

      valueCount = 0; // Reset the counter for the next set of readings
    }

    delayMicroseconds(pulseDelay - (micros() - timerVal)); // Maintain PWM timing
  } // End of the infinite loop
}


float readAndAverage() {
  const int numReadings = 5;
  float total = 0.0;
  for (int i = 0; i < numReadings; i++) {
    total += analogRead(currentPin);
    delayMicroseconds(4); // Small delay to stabilize ADC reading
  }
  return (total / numReadings) * (3.3 / 4095.0); // Convert to voltage
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
  while (!scriptRunning && !Serial.available()); // Wait for input
  String input = Serial.readStringUntil('\n'); // Read input
  if (input == "on") {
    Serial.println("Script started.");
    // scriptRunning = true;
    // Add your script start code here
  } else {
    Serial.println("Invalid command. Please type 'on' to start the script.");
    waitForOnCommand(); // Call the function recursively until valid input is received
  }
}


// ----------------------------------------------------------
// NEW1 functions for flow pattern input and parsing
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
  while (!scriptRunning && !Serial.available()); // Wait for input
  // string input = Serial.readStringUntil('\n'); // Read input
  // std::string inputString = Serial.readStringUntil('\n');
  // std::string desiredFlowPattern = inputString;
  String inputString = Serial.readStringUntil('\n'); // Read input
  std::string desiredFlowPattern = inputString.c_str();
  if (desiredFlowPattern[0] == '[') {
    Serial.print("You inputted the following flow pattern: ");
    Serial.println(desiredFlowPattern.c_str()); // Convert to const char* for println    scriptRunning = true;
    // Add your script start code here
  } else {
    Serial.println("Your input was not accepted. Please enter a string in the provided form.");
    getPatternFlowRate(); // Call the function recursively until valid input is received
  }
  return desiredFlowPattern;
}





std::vector<std::vector<float>> stringToArray(const std::string& input) {
    std::vector<std::vector<float>> result;
    // Serial.println("inputted array: ");
    // Serial.println(input);
    
    // Remove the leading '[' and trailing ']' from the input string
    std::string trimmedInput = input.substr(1, input.size() - 2);
    // Serial.println("trimmedInput: ");
    // Serial.println(input);
    
    // Use stringstream to parse the string
    std::stringstream ss(trimmedInput);
    std::string token;
    
    // Parse by delimiter ';'
    while (std::getline(ss, token, ';')) {
        std::vector<float> row;
        std::stringstream row_ss(token);
        std::string element;
        bool isSecondColumn = false; // Flag to track if we are parsing the second column
        
        // Parse each element of the row by delimiter ','
        while (std::getline(row_ss, element, ',')) {
            // Remove spaces from the element
            element.erase(std::remove_if(element.begin(), element.end(), ::isspace), element.end());
            
            // Convert string to float
            float value = std::stof(element);
            
            // If this is the second column, convert minutes to milliseconds
            if (isSecondColumn) {
                value *= 60000; // Convert minutes to milliseconds
            }
            
            row.push_back(value);
            // Serial.println("value: ");
            // Serial.println(value);
            isSecondColumn = !isSecondColumn; // Toggle flag for the next iteration
        }
        // Serial.println("row: ");
        // Serial.println(row);
        result.push_back(row);
    }
    
    return result;
}



// std::vector<std::vector<float>> stringToArray(const std::string& input) {
//     std::vector<std::vector<float>> result;
//     Serial.print("inputted array: ");
//     Serial.println(input.c_str()); // Convert std::string to const char*

//     // Remove the leading '[' and trailing ']' from the input string
//     std::string trimmedInput = input.substr(1, input.size() - 2);
//     Serial.print("trimmedInput: ");
//     Serial.println(trimmedInput.c_str()); // Convert std::string to const char*

//     // Use stringstream to parse the string
//     std::stringstream ss(trimmedInput);
//     std::string token;

//     // Parse by delimiter ';'
//     while (std::getline(ss, token, ';')) {
//         std::vector<float> row;
//         std::stringstream row_ss(token);
//         std::string element;
//         bool isSecondColumn = false; // Flag to track if we are parsing the second column

//         // Parse each element of the row by delimiter ','
//         while (std::getline(row_ss, element, ',')) {
//             // Remove spaces from the element
//             element.erase(std::remove_if(element.begin(), element.end(), ::isspace), element.end());

//             // Convert string to float
//             float value = std::stof(element);

//             // If this is the second column, convert minutes to milliseconds
//             if (isSecondColumn) {
//                 value *= 60000; // Convert minutes to milliseconds
//             }

//             row.push_back(value);
//             Serial.print("value: ");
//             Serial.println(value);
//             isSecondColumn = !isSecondColumn; // Toggle flag for the next iteration
//         }
//         // Print the row
//         Serial.print("row: ");
//         for (const auto& elem : row) {
//             Serial.print(elem);
//             Serial.print(" ");
//         }
//         Serial.println();
//         result.push_back(row);
       
//     }

//     printArray(result);
//   return result;
// }


float currentDesiredFlowRate(std::vector<std::vector<float>> flowPattern, int current_time_ms) {

    // // Display the result
    // std::cout << "Output Array:" << std::endl;
    // for (const auto& row : flowPattern) {
    //     std::cout << "{";
    //     for (size_t i = 0; i < row.size(); ++i) {
    //         std::cout << row[i];
    //         if (i < row.size() - 1)
    //             std::cout << ", ";
    //     }
    //     std::cout << "}" << std::endl;
    // }

    // if (startFlag == 0) { time_start_count_delay = current_time_ms}
 
    float desiredFlowRate;
    float flowStartTime = 0;
    float flowEndTime;
    int numRows = flowPattern.size();
    // Serial.print("numRows: ");
    // Serial.println(numRows);
        for (int i = 0; i <= numRows-1; ++i) {
          // Serial.print("check time: ");
          // Serial.println(flowPattern[i][1]);
          float flowEndTime = flowStartTime + flowPattern[i][1];
          if (current_time_ms >= flowStartTime && current_time_ms <= flowEndTime) {
            desiredFlowRate = flowPattern[i][0];
            // Stop_Flag = 0;
            // std::cout << "desiredFlowRate: " << desiredFlowRate << "; time: " << current_time_ms << std::endl;
            return desiredFlowRate;
          }
          else {
            flowStartTime = flowEndTime;
          }
        }
        if (current_time_ms > flowEndTime) {
            return endFlowRate;
          }
        // Serial.print("current_time_ms: ");
        // Serial.println(current_time_ms);
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


  // ----------------------------------------------------------
  // (end new1)
  // ----------------------------------------------------------

// void askForFlowType() {
//   Serial.println("Type 'c' to initiate constant flow and 'p' to initiate pattern flow");
//   while (!scriptRunning && !Serial.available()); // Wait for input
//   String input = Serial.readStringUntil('\n'); // Read input
//   if (input == "c") {
//     Serial.println("Script started.");
//     // scriptRunning = true;
//     // Add your script start code here
//   } else if (input == "p")  {

//   }
//   else {}
//     Serial.println("Invalid command. Please type 'c' or 'p' to start the script.");
//     waitForOnCommand(); // Call the function recursively until valid input is received
//   }
// }



// float getConstFlowRate() {
//   while (!scriptRunning && !Serial.available()); // Wait for input
//   String input = Serial.readStringUntil('\n'); // Read input
//   float liveDesiredFlowRate = atof(input.c_str());
//   if (liveDesiredFlowRate > 0.0) {
//     Serial.print("Desired Flow Rate: ");
//     Serial.println(liveDesiredFlowRate);
//     scriptRunning = true;
//     // Add your script start code here
//   } else {
//     Serial.println("Please type a number greater than zero to start the script.");
//     getConstFlowRate(); // Call the function recursively until valid input is received
//   }
//   return desiredFlowRate;

