//HMC Cowbell Clinic 1/30/24 ---> This is a first go at P control Flow using stepper motor 
// -----------------------------------------
//Libs
#include <AccelStepper.h>    // For Stepper 
#include <avr/pgmspace.h>      // For flash storage
#include <Wire.h>              // Needed for SFM
#include <sfm3000wedo.h>      // For Flow Meter 
// -----------------------------------------
//Initializing Flow Meter 
SFM3000wedo measflow(64);
int offset = 32768;
int scale = 120;

//-----------------------------------------
//Pin Declarations (Flow Meter)
#define orificeSensorPin A0

//---------------------------------------
//Pin Declarations (Stepper + Calibration )
#define stepPin 3
#define dirPin 11
#define monitorPin 12
#define currentPin A3
#define motorInterfaceType 1

//---------------------------------------
//Vars (Flow Meter)
const float ADC_mV = 4.8828125;
const float sensitivity = 4.413;
const float mmh2O_cmH2O = 10;
const float mmh2O_kpa = 0.00981;
const float mmh2O_pa = 9.80665;
const int ADCoffset = 34;
float averageFlow = 0;
//---------------------------------------
//Vars (Motor+Calibration)
unsigned int pulseDelay = 1000; // Microseconds
//---------------------------------------
// Declare a pointer for the stepper motor object
AccelStepper *stepper;
//---------------------------------------
// Proportional Gain
float Kp = 350; // Adjust this based on your system's response
//---------------------------------------
// Desired flow rate
float desiredFlowRate = 5.0; // Example value, set this according to your needs
//---------------------------------------


void setup() {

  //----------------------------------------------------
  // Calibration Stuff 
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(monitorPin, OUTPUT);
  Serial.begin(9600);
  delay(1000); // Delete Just giving time to open monitor 
  Serial.println("Calibrating....");
  // Set initial direction
  digitalWrite(dirPin, HIGH); // Change to LOW if needed
  // Monitor pin goes low when the current sense is not being sampled
  digitalWrite(monitorPin, LOW);
  performMotorOperation();
  delay(500);
  Serial.println("Valve Motor Calibrated, Position Set to Zero");
  digitalWrite(dirPin, LOW);
  //-------------------------------------------
  // Initializing Flow Meter
  delay(500);
  measflow.init();
  Serial.println("Flow Sensor Initialized");
  //--------------------------------------------------
  // Declaring Stepper For library 
  stepper = new AccelStepper(motorInterfaceType, stepPin, dirPin);
  stepper->setMaxSpeed(1000); // Maximum steps per second, adjust as needed
  stepper->setAcceleration(600); // Steps per second squared, adjust as needed
  stepper->setCurrentPosition(0);
  Serial.println("Mass Flow Controller Ready, Starting Control Loop in 2 Seconds...");
  delay(2000);
 // ---------------------------------------------------------------------------

}
  // Rolling Avg Stuff for Flow Sensor 
  int const a_size = 100;
  float rollingAverage[a_size] = {0};  // Array to store the last 'a_size' SFM3300 sensor readings
  int currentIndex = 0;
  // Rolling Avg Stuff for Flow Sensor 

void loop() {

  readSensors(); // Read Flow rate 
  float currentFlowRate = (averageFlow); // Updates Flow Measurement

  // Calculate error
  float error = desiredFlowRate - currentFlowRate;
  //Serial.println("error" + String(error));

  // Calculate control effort (desired position)
  long controlEffort = long(Kp * error); 
  //Serial.println("Control Effort: " + String(controlEffort));
  
  controlEffort = -controlEffort;
  //Serial.println("Control Effort: " + String(controlEffort));

  long newgoalposition = stepper->currentPosition() + controlEffort;
  long currpos = stepper->currentPosition();

  //Serial.println();
 // Serial.println("error" + String(error));  
  //Serial.println("Current Position " + String(currpos)); 
  //Serial.println("Control Effort: " + String(controlEffort));
  //Serial.println("New Goal Position: " + String(newgoalposition)); // Debugging Prints
 
  if (newgoalposition > 0  || newgoalposition < -3995) {
        Serial.println("Crash Course detected!...Motor stopped, Please restart program!");
        while(1);  // Infinite loop to halt the program
    }
 
  // Set the stepper to the new position

  stepper->moveTo(newgoalposition);
  // Perform the steps
  while (stepper->distanceToGo() != 0) {
    stepper->run();
  }
  //Serial.println("Position: " + String(stepper->currentPosition()));
  //delay(10); //Giving Flow some time to stabalize before reading its value again

}

//-------------------------------------
//-------------------------------------
//      Get Flow Meter Readings
//-------------------------------------
//-------------------------------------

void readSensors() {
  int rawValue = analogRead(orificeSensorPin);
  float zeroedADCval = float(rawValue) - ADCoffset;
  zeroedADCval = constrain(zeroedADCval, 0, 1023);
  float pressurePa = zeroedADCval * ADC_mV / sensitivity * mmh2O_pa;

  averageFlow = 0; // Reset averageFlow to calculate new average

  for (int i = 0; i < 100; ++i) {
    float flowSFM = measflow.getvalue();
    if (flowSFM > 0) flowSFM = 0;
    else if (flowSFM < 0) flowSFM = flowSFM - offset;
    flowSFM = flowSFM / scale;

    rollingAverage[currentIndex] = flowSFM;
    currentIndex = (currentIndex + 1) % a_size;

    averageFlow += flowSFM; // Accumulate the sum of 30 readings
  }

  averageFlow /= 100; // Calculate the average of 30 readings

 // Serial.print("averageFlow:");
  Serial.print(averageFlow); // Print the new rolling average of 30 values
  Serial.print(",");
 // Serial.print("desiredFlowRate:");
  Serial.print(desiredFlowRate);
  Serial.println();
}


//-------------------------------------
//-------------------------------------
//         Motor Calibration
//-------------------------------------
//-------------------------------------

void performMotorOperation() {

  while(true) {
    // Generate a step pulse
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000); // Adjust delay as needed for your motor's speed
    digitalWrite(stepPin, LOW);

    // Log timing
    digitalWrite(monitorPin, HIGH);
    unsigned long timerVal = micros();

    // Read and average the current sensor value
    float voltage = readAndAverage();

    // Check the voltage condition
    if (voltage >= 2.49) {
      digitalWrite(stepPin, LOW);
      Serial.println("Zeroed");
      digitalWrite(stepPin, LOW); // Stop the motor
      return; // Exit
    }

    digitalWrite(monitorPin, LOW);
    delayMicroseconds(pulseDelay - (micros() - timerVal)); // Adjust delay as needed for your motor's speed
  }
}

//-------------------------------------
//-------------------------------------
// Currentsensor Reading for Calibration
//-------------------------------------
//-------------------------------------

float readAndAverage() {
  const int numReadings = 5;
  float total = 0.0;

  for (int i = 0; i < numReadings; i++) {
    total += analogRead(currentPin);
  }

  float average = total / numReadings;
  float voltage = (average * 5.0) / 1024.0;
 // Serial.println(voltage);
  return voltage;
}

//-------------------------------------
//-------------------------------------
//-------------------------------------


















