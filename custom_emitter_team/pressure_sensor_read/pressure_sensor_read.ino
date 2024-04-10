// Pressure sensor is connected to GPIO 33 (Analog ADC1_CH6) 
const int presPin = 33;

// variable for storing the potentiometer value
float presVolt = 0;
float VS = 5.0


void setup() {
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  // Reading pressure sensor voltage value
  // Want to read 0 PSIG to 100 PSIG
  // 14.7 PSIA to 114.7 PSIA
  // ADC is linear from approx 0.5V to 2.5V
  // 50 - 360 kpa
  presVolt = analogRead(presPin);
  Serial.print("Analog value: ");
  Serial.println(presVolt);


  delay(500);
}

float presCal(presVolt) {
  // Converting voltage value to pressure
  // Vout = VS*(0.0012858*P+0.04) ± Error
 float pres = (() presVolt / VS ) - 0.04) / 0.0012858
 return pres;
}