// Works! Red LED was dead, and need low reistor (230-450 ohms?)
// Resources:
// https://techexplorations.com/guides/esp32/begin/digitalout/
// https://esp32io.com/tutorials/esp32-button
// File>Examples>WiFi>SimpleWifiServer


#include <WiFi.h>

int ledPin = 32;  //34 and 35 are read-only
int internalLedPin = 2;
int buttonPin = 33;

// Wifi network username and password
const char* ssid     = "Claremont-ETC";
const char* password = "Cl@remontI0T";

WiFiServer server(80);

// int lastState = LOW; // the previous state from the input pin
// int currentState;     // the current reading from the input pin

void setup() {
  // Declare LED and button pins
  Serial.begin(115200);
  Serial.flush();
  pinMode(internalLedPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println("on");
  delay(10);

  // Wifi stuff
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);


  Serial.println("");
  Serial.println("");
  Serial.println("Press the EN button to get the IP address.");
  Serial.println("");
  Serial.println("");


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

}

void loop() {

  WiFiClient client = server.available();     // listen for incoming clients

  if (client) {                               // if you get a client,
    Serial.println("New Client.");            // print a message out the serial port
    String currentLine = "";                  // make a String to hold incoming data from the client
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        char c = client.read();               // read a byte, then
        Serial.write(c);                      // print it out in the serial monitor
        if (c == '\n') {                      // if the byte is a newline character

          // if the current line is blank, you got two newline cahracters in a row. *** don't understand
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with the a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">this button</a> to turn ON the LEDs.<br>");
            client.print("Click <a href=\"/L\">THIS button</a> to turn OFF the LEDs.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            //break out of the while loop:
            break;
          }
          else {    // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r') {   // if you got anything else but a carriage return character,
          currentLine += c;     // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(internalLedPin, HIGH);
          digitalWrite(ledPin, HIGH);
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(internalLedPin, LOW);
          digitalWrite(ledPin, LOW);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.print("Client Disconnected.");
    }



  // put your main code here, to run repeatedly:
  // int buttonState = digitalRead(buttonPin);
  // Serial.println(buttonState);

  
  // if(buttonState == 1) {
  //   digitalWrite(ledPin, HIGH);
  //   digitalWrite(internalLedPin, LOW);
  // }
  // else if (buttonState == 0) {
  //   digitalWrite(ledPin, LOW);
  //   digitalWrite(internalLedPin, HIGH);
  // }
  // else {
  //   Serial.println(45);
  // }
  // delay(100);

}
