//------------------------------------------------------------------------------
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
// Motor Setup Stuff
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
// --------------------------------------------------------
//Flow Meter Setup Stuff
//----------------------------------------------------------
#include <Wire.h>
#define SDA_2 25  // Secondary I2C Bus 
#define SCL_2 26 // Secondary I2C Bus
float FlowSens_Big_Average = 0; //Declare Globally 
float FlowSens_Small_Average = 0; //Declare Globally 
//----------------------------------------------------------
// Wifi Control Setup Stuff
//---------------------------------------------------------- 

// TBD

// ----------------------------------------------------------
// P control Parameter Setup
// ----------------------------------------------------------
// Proportional Gain
float Kp = 800; // This is my guess based on 45 PSI input 
//---------------------------------------
// Desired flow rate
float desiredFlowRate = 100.0; // Change
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
  Serial.begin(115200);
  // --------------------------------------------------------
  // Motor Calibration, comment out this sec below to skip 
  // --------------------------------------------------------
  delay(100); // Just giving time to open monitor
  Serial.println("Calibrating....");
  digitalWrite(0, HIGH); // Change to LOW if needed for Big_Motor
  performMotorOperation(2, 15, 0.02); // Calibrate Big_Motor 1 with threshold
  delay(200);
  Serial.println("Big_Motor Calibrated");
  delay(200);
  digitalWrite(15, HIGH); // Change to LOW if needed for Big_motor
  float stdDevHistory[5] = {0.02, 0.02, 0.02, 0.02, 0.02}; // Reset Buffer
  performMotorOperation(4, 0, 0.04); // Calibrate Small_Motor with threshold
  delay(200);
  Serial.println("Small_Motor Calibrated");
  delay(200);

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

  // ----------------------------------------------------------
  //
  // End of Setup!
  //
  // ----------------------------------------------------------
  Serial.println("Mass Flow Controller Ready, Starting Control Loop in 1 Second...");
  delay(1000);
  //-----------------------------------------------------------
  

}

void loop() {

  // This first if statment moves the big motor to open the big valve for any desired flow above 25 SLM

  if (Stop_Flag == 0 && desiredFlowRate>25) {
    long BigMotorPosition = ((desiredFlowRate-10)/50)*3000; // 10 here is offset so it leaves last 10 SLM up to small valve, 50 is a assumtion that 1 turn = 50slm output (based on data at 45 PSI), and 3000 is slightly conservative steps/rev estimate 
    // Now the motor moves!
    Big_Motor->moveTo(BigMotorPosition); 
    while (Big_Motor->distanceToGo() != 0) {
      Big_Motor->run();
    }
    Stop_Flag=1;
  }

  readAndAverageFlowSens_Big();
  readAndAverageFlowSens_Small();

  float currentBigFlowRate = (FlowSens_Big_Average); // Updates Flow 
  float currentSmallFlowRate = (FlowSens_Small_Average); // Updates Flow 
  float total_flow= currentSmallFlowRate + currentBigFlowRate;
 
  float error = desiredFlowRate - (total_flow);  // Calculate error

  long controlEffort = long(Kp * error);   // Calculate control effort 
  
  controlEffort = -controlEffort;
  
  long newgoalposition = Small_Motor->currentPosition() + controlEffort;
  long currpos = Small_Motor->currentPosition();
 
  // Future crash Check
  if (newgoalposition > 0  || newgoalposition < -31000) {
        Serial.println("Crash Course detected!...Motor stopped, Please restart program!");
        while(1);  // Infinite loop to halt the program
    }
  

  // Print out all the needed info before moving the motor
  Serial.print(millis()); // time for csv
  Serial.print(" , ");
  Serial.print(String(currentBigFlowRate)); // time for csv
  Serial.print(" , ");
  Serial.print(String(currentSmallFlowRate)); // time for csv
  Serial.print(" , ");
  Serial.print(String(total_flow)); // time for csv
  Serial.print(" , ");
  Serial.print(String(desiredFlowRate)); // time for csv
  Serial.print(" , ");
  Serial.print(String(error)); // error for csv
  Serial.print(" , ");
  Serial.print(String(Kp));
  Serial.print(" , ");
  Serial.print(String(controlEffort)); // error for csv
  Serial.print(" , ");
  Serial.print(String(currpos)); // for csv
  Serial.print(" , ");
  Serial.print(String(newgoalposition)); // for csv
  Serial.println();
  // Print out all the needed info before moving the motor

  Small_Motor->moveTo(newgoalposition);   // Set the stepper to the new position
  // Perform the steps
  while (Small_Motor->distanceToGo() != 0) {
    Small_Motor->run();
  }

  delay(100); // Just to give it some rest


}











  // ----------------------------------------------------------
  // Flow Measurement Functions 
  // ----------------------------------------------------------

void readAndAverageFlowSens_Big() {
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    delay(2); // delay to give sensor rest, so total read should take 200ms 
    Wire.requestFrom(0x40, 2); 
    uint16_t a1 = Wire.read();
    uint8_t  b1 = Wire.read();
    a1 = (a1 << 8) | b1;
    float flow_Big = ((float)a1 - 32768) / 120; //offset Cal Math 
    sum += flow_Big;
  }
  float FlowSens_Big_Average = sum / 100.0;
  //Serial.print("Average Flow from FlowSens_Big: ");
  //Serial.println(FlowSens_Big_Average);
}

void readAndAverageFlowSens_Small() {
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    delay(2); // delay to give sensor rest, so total read should take 200ms
    Wire1.requestFrom(0x40, 2); 
    uint16_t a2 = Wire1.read();
    uint8_t  b2 = Wire1.read();
    a2 = (a2 << 8) | b2;
    float flow_Small = ((float)a2 - 32768) / 800; // offset Cal Math 
    sum += flow_Small;
  }
  float FlowSens_Small_Average = sum / 100.0;
  //Serial.print("Average Flow from FlowSens_Small: ");
  //Serial.println(FlowSens_Small_Average);
}

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
      Serial.println(standardDeviation);

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