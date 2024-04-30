// Works! Red LED was dead, and need low reistor (230-450 ohms?)

int ledPin = 32;  //34 and 35 are read-only
int internalLedPin = 2;
int buttonPin = 33;

// int lastState = LOW; // the previous state from the input pin
// int currentState;     // the current reading from the input pin

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.flush();
  pinMode(internalLedPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println("on");
}

void loop() {
  // put your main code here, to run repeatedly:
  int buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);

  
  if(buttonState == 1) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(internalLedPin, LOW);
  }
  else if (buttonState == 0) {
    digitalWrite(ledPin, LOW);
    digitalWrite(internalLedPin, HIGH);
  }
  else {
    Serial.println(45);
  }
  delay(100);

  int i = 0;
  int j = 0;
  int blinks = 5;
  int delayms = 500;
  while (i < blinks) {
    // whiteLED(lightswitch);
    digitalWrite(32, HIGH);
    Serial.println("green led on");
    while (j < 100) {
      delay(delayms / 100);
      // whiteLED(lightswitch);
      j++;
    }
    j = 0;
    // delay(delayms);
    digitalWrite(32, LOW);
    Serial.println("green led off");
    while (j < 100) {
      delay(delayms / 100);
      // whiteLED(lightswitch);
      j++;
    }
    i++;
  }
}
