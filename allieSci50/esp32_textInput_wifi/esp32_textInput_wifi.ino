/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// GPIO Setup
// int ledPin = 32;  //34 and 35 are read-only
// int internalLedPin = 2;
// int buttonPin = 33;

int ledPin;

// Web server???
AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid     = "Claremont-ETC";
const char* password = "Cl@remontI0T";

const char* PARAM_INPUT_1 = "pickLight";
const char* PARAM_INPUT_2 = "lightswitch";

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html_pick[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Pick an LED</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <p>Please select which LED to operate:</p>
  <p><i>Pin 2 is the internal LED</i></p>
  <form action="/get">
    Enter pin number: <input type="text" name="pickLight">
    <input type="submit" value="yep">
  </form><br>
</body></html>)rawliteral";


const char index_html_switch[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Turn LED On and Off</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    <input type="radio" id="ledON" name="lightswitch" value="1">
    <label for="ledON">ON</label><br>
    <input type="radio" id="ledOFF" name="lightswitch" value="0">
    <label for="ledOFF">OFF</label><br>
    <input type="submit" value="Let there be light, or not.">
  </form><br>
  <br><a href="/">Pick a different LED</a>
</body></html>)rawliteral";

// <input type="radio" id="internal" name="pickLight" value="2">
//     <label for="internal">Internal LED</label><br>
//     <input type="radio" id="external" name="pickLight" value="32">
//     <label for="external">External LED</label><br>


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  // pinMode(internalLedPin, OUTPUT);
  // pinMode(ledPin, OUTPUT);
  // pinMode(buttonPin, INPUT_PULLUP);

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html_pick);
  });

   // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
  
    String inputMessage;
    String inputParam;
    int inputNum;
    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputNum = inputMessage.toInt();

      ledPin = inputNum;

      pinMode(ledPin, OUTPUT);

      Serial.print("Turned on pin ");
      Serial.println(ledPin);


    }


    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      inputNum = inputMessage.toInt();
      // Serial.print("Input Param: ");
      // Serial.println(inputParam);
      // Serial.print("Input Message: ");
      // Serial.println(inputMessage);
      // Serial.print("Input Number: ");
      // Serial.println(inputNum);

      if (inputNum == 1) {
        digitalWrite(ledPin, HIGH);
        Serial.println("Lights ON");
      }
      else if (inputNum == 0) {
        digitalWrite(ledPin, LOW);
        Serial.println("Lights OFF");
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
      inputNum = 8888888;
    }

    request->send(200, "text/html", index_html_switch);
  });


  // Send web page with input fields to client
  server.on("/switchpage", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html_switch);
  });


  // // Send web page with input fields to client
  // server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
  
  //   String inputMessage;
  //   String inputParam;
  //   int inputNum;

    
  //   // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
  //   if (request->hasParam(PARAM_INPUT_2)) {
  //     inputMessage = request->getParam(PARAM_INPUT_2)->value();
  //     inputParam = PARAM_INPUT_2;
  //     inputNum = inputMessage.toInt();
  //     // Serial.print("Input Param: ");
  //     // Serial.println(inputParam);
  //     // Serial.print("Input Message: ");
  //     // Serial.println(inputMessage);
  //     // Serial.print("Input Number: ");
  //     // Serial.println(inputNum);


  //     if (inputNum == 1) {
  //       digitalWrite(ledPin, HIGH);
  //       Serial.println("Lights ON");
  //     }
  //     else if (inputNum == 0) {
  //       digitalWrite(ledPin, LOW);
  //       Serial.println("Lights OFF");
  //     }
  //   }
  //   else {
  //     inputMessage = "No message sent";
  //     inputParam = "none";
  //     inputNum = 8888888;
  //   }

  //   Serial.println("***");
  //   Serial.println(inputMessage);
  //   Serial.println(inputNum);
    
  //   if (inputParam == PARAM_INPUT_2) {
  //     if (inputNum == 1) {
  //       digitalWrite(ledPin, HIGH);
  //       Serial.println("Lights ON");
  //     }
  //     else if (inputNum == 0) {
  //       digitalWrite(ledPin, LOW);
  //       Serial.println("Lights OFF");
  //     }
  //   }
  //   else {Serial.println("Nice try, type again.");}

  //   request->send_P(200, "text/html", index_html_switch);

  // });



 
  server.onNotFound(notFound);
  server.begin();
}




void loop() {
  
}
