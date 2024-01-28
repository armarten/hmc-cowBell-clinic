// Define step and direction pins
#define stepPin 3
#define dirPin 11
#define monitorPin 12
#define currentPin A3

unsigned long timerVal = 0;
unsigned int pulseDelay = 1000; // Microseconds
int sampleVal = 0;
int sampleVal2 = 0;
int sampleVal3 = 0;
int sampleVal4 = 0;
int sampleVal5 = 0;
int avg = 0;


void setup() {
  // Set step and direction pins as OUTPUT
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(monitorPin, OUTPUT);
  Serial.begin(115200);

  // Set initial direction (HIGH or LOW) based on your motor's rotation direction
  digitalWrite(dirPin, HIGH); // Change to LOW if needed

  // Monitor pin goes high when the current sense is being sampled
  digitalWrite(monitorPin, LOW);
}

void loop() {
  // Generate a step pulse
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(1000); // Adjust delay as needed for your motor's speed
  digitalWrite(stepPin, LOW);
  // Log timing
  digitalWrite(monitorPin, HIGH);
  timerVal = micros();
  // Sample the current sense pin
  sampleVal = analogRead(currentPin);
  sampleVal2 = analogRead(currentPin);
  sampleVal3 = analogRead(currentPin);
  sampleVal4 = analogRead(currentPin);
  sampleVal5 = analogRead(currentPin);
  avg = (sampleVal2+sampleVal2+sampleVal3+sampleVal4+sampleVal5)/5;
  float voltage =(avg*5.0)/1024.0;
  Serial.println(voltage);
   if (voltage >= 2.46) {
        // infinite loop.
        while(1) {}
    }

  digitalWrite(monitorPin, LOW);
  delayMicroseconds(pulseDelay - (micros() - timerVal)); // Adjust delay as needed for your motor's speed
}
