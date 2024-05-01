// Works! Red LED was dead, and need low reistor (230-450 ohms?)

int gpioLedPin = 32; //34 and 35 are read-only
int ledPin = 2;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.flush();
  pinMode(ledPin, OUTPUT);
  pinMode(gpioLedPin, OUTPUT);
  // pinMode(gpioLedPin, INPUT_PULLUP);
  // digitalWrite(gpioLedPin, HIGH);
  // digitalWrite(ledPin, HIGH); 
  // Serial.println("on");

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(gpioLedPin, HIGH);
  digitalWrite(ledPin, HIGH); 
  Serial.println("on");
  delay(1000);
  digitalWrite(gpioLedPin, LOW);
  digitalWrite(ledPin, LOW); 
  Serial.println("off");
  delay(1000);

}
