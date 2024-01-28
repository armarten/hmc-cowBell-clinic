#include <avr/pgmspace.h>      // For flash storage
#include <Wire.h>              // Needed for SFM
#include <sfm3000wedo.h>

// Define pins for stepper motor and sensor
#define stepPin 3
#define dirPin 11
#define monitorPin 12
#define currentPin A3
#define orificeSensorPin A0

// Constants for flow sensor
SFM3000wedo measflow(64);
int offset = 32768; // SFM Offset for the SFM3300
int scale = 120;    // SFM Scale

// Constants for motor control
unsigned long timerVal = 0;
unsigned int pulseDelay = 1000; // Microseconds
const float goalFlowRate = 100.0; // Goal volumetric flow rate in ml/s
float P_gain = 1.0; // Proportional gain for P control

// Variables for current sensing and motor zeroing
int avg = 0;
bool isMotorZeroed = false; // Flag to check if motor has been zeroed

// Constants for orifice flow sensor reading
const float ADC_mV = 4.8828125; // Conversion multiplier from Arduino ADC value to voltage in mV
const float sensitivity = 4.413; // in mV/mmH2O taken from datasheet
const float mmh2O_pa = 9.80665; // Conversion multiplier from mmH2O to Pa
const int ADCoffset = 34; // Measured offset

// Variables to keep track of motor steps
long motorSteps = 0;

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(monitorPin, OUTPUT);
  Serial.begin(9600);

  digitalWrite(dirPin, HIGH); // Set initial direction
  digitalWrite(monitorPin, LOW);

  measflow.init();
  Serial.println("Sensors initialized");
}

void loop() {

  if (!isMotorZeroed) {
    zeroMotor();
    isMotorZeroed = true;
  }

  float flowRate = readFlowRate();
  pControlStepper(flowRate);

  printStatus(flowRate);
}

void zeroMotor() {
  while (true) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);

    digitalWrite(monitorPin, HIGH);
    timerVal = micros();

    avg = analogReadAverage(currentPin);
    float voltage = (avg * 5.0) / 1024.0;
    if (voltage >= 2.46) {
      break;
    }

    digitalWrite(monitorPin, LOW);
    delayMicroseconds(pulseDelay - (micros() - timerVal));
    motorSteps--; // Decrement steps as we are moving to zero
  }
}

int analogReadAverage(int pin) {
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(pin);
  }
  return total / 5;
}

float readFlowRate() {
  int rawValue = analogRead(orificeSensorPin);
  float zeroedADCval = float(rawValue) - ADCoffset;
  zeroedADCval = constrain(zeroedADCval, 0, 1023);
  float pressurePa = zeroedADCval * ADC_mV / sensitivity * mmh2O_pa;

  float flowSFM = measflow.getvalue();
  if (flowSFM > 0) flowSFM = 0;
  else if (flowSFM < 0) flowSFM = flowSFM - offset;
  flowSFM = flowSFM / scale * 16.6666; // Convert to ml/s

  return flowSFM;
}

void pControlStepper(float currentFlowRate) {
  float error = goalFlowRate - currentFlowRate;
  int stepChange = (int)(P_gain * error);
  if (stepChange != 0) {
    digitalWrite(dirPin, stepChange > 0 ? LOW : HIGH);
    for (int i = 0; i < abs(stepChange); i++) {
      stepMotor();
      motorSteps += (stepChange > 0) ? 1 : -1; // Update step count based on direction
    }
  }
}

void stepMotor() {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(pulseDelay - 1000);
}

void printStatus(float flowRate) {
  Serial.print("Steps: ");
  Serial.print(motorSteps);
  Serial.print(", Flow Rate: ");
  Serial.println(flowRate);
}
