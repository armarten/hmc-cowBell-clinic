// Works! Red LED was dead, and need low reistor (230-450 ohms?)

int thermPin = 25; //34 and 35 are read-only
int ledPin = 2;
float therm;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.flush();
  pinMode(ledPin, OUTPUT);
  pinMode(thermPin, INPUT);
  Serial.print("start");

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin, HIGH); 
  therm = analogRead(thermPin);
  Serial.println(therm);
  // Serial.println("on");
  delay(500);
  digitalWrite(ledPin, LOW); 
  therm = analogRead(thermPin);
  Serial.println(therm);
  // Serial.println("off");
  delay(500);

}
