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
#include <ArduinoJson.h>

// GPIO Setup
// int ledPin = 32;  //34 and 35 are read-only
int internalLedPin = 2;  //to flash when get data request
// int buttonPin = 33;
int thermPin = 34;
int buttonPin = 25; // internal pullup, ADC_2, 
int ledPin;

// Output variable initialization
float runTime;
int buttonPressed;
float therm;



// Web server???
AsyncWebServer server(80);


// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid     = "Claremont-ETC";
const char* password = "Cl@remontI0T";

// const char* PARAM_INPUT_1 = "pickLight";
const char* PARAM_INPUT_1 = "ctl-light";
const char* PARAM_INPUT_2 = "read-data";
const char* PARAM_INPUT_3 = "ctl-ledPin";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


int readButton() {
  int buttonStatus = digitalRead(buttonPin);
  return buttonStatus;
}


void ledPattern(int buttonPressed1) { 
  int i = 0;
  int delayms;

  if (buttonPressed1 == 1) {
    delayms = 500;
  }
  else {
    delayms = 2000;
  }
  while (i < 5) {
    digitalWrite(ledPin, HIGH);
    delay(delayms);
    digitalWrite(ledPin, LOW);
    delay(delayms);
    i++;
  }
}



void setup() {
  Serial.begin(115200);
  Serial.flush();
  pinMode(internalLedPin, OUTPUT);
  // pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(thermPin, INPUT);
  
    
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
    request->send_P(200, "text/plain", "home_page");
  });

    // text/plain: Used for plain text content. This could be used for simple textual responses that are not HTML formatted.

    // application/json: Used for JSON data. If you're sending JSON data back to the client, you would use this Content-Type.

    // application/xml or text/xml: Used for XML data. If you're sending XML data back to the client, you would use one of these Content-Types.

    // image/jpeg, image/png, image/gif, etc.: Used for image data. If you're sending image files or binary data representing images, you would use one of these Content-Types.

    // application/pdf: Used for PDF documents. If you're sending PDF files, you would use this Content-Type.

    // application/octet-stream: Used for arbitrary binary data. If you're sending non-textual binary data, you might use this Content-Type.

    // const char* PARAM_INPUT_1 = "ctl-light";
    // const char* PARAM_INPUT_2 = "read-data";
    // const char* PARAM_INPUT_3 = "ctl-ledPin";


   // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
  
    String inputMessage;
    String inputParam;
    int inputNum;
    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      // inputNum = inputMessage.toInt();
      // ledPin = inputNum;
      if (inputMessage == "on") {
        Serial.println("ON");
        digitalWrite(ledPin, HIGH);
      }
      else if (inputMessage == "off") {
        Serial.println("OFF");
        digitalWrite(ledPin, LOW);
      }
      else {
        request->send_P(200, "text/plain", "Invalid LED State");
      }


    }
    if (request->hasParam(PARAM_INPUT_2)) {
      digitalWrite(internalLedPin, HIGH);
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;

      StaticJsonDocument<200> doc;

      doc["time"] = runTime;
      doc["button status"] = buttonPressed;
      doc["thermistor"] = therm;

      // doc.shrinkToFit();  // optional
      serializeJsonPretty(doc, Serial);

      // Print a minified JSON document to the serial port
      String json_text;
      serializeJson(doc, json_text);
      // serializeJson(doc, WifiClient);
      // Serial.print("textTest:");
      // Serial.println(json_text);
      // Serial.print("jsonSerial:");
      // serializeJson(doc, Serial);

      // Same with a prettified document

      // serializeJson(doc, WiFiClient);
      // json_str = serializeJson(s);

      request->send_P(200, "application/json", json_text.c_str());
      delay(100);
      digitalWrite(internalLedPin, LOW);
    }



    if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      inputNum = inputMessage.toInt();
      ledPin = inputNum;
      pinMode(ledPin, OUTPUT);
      String pinString = "pin ";
      String modeString = " mode changed to OUTPUT";
      String pinOutputString = pinString + inputMessage + modeString;

      request->send_P(200, "text/plain", pinOutputString.c_str());

    }
    

    else {
      inputMessage = "No message sent";
      inputParam = "none";
      inputNum = 8888888;
    }
        Serial.print("Input Param: ");
        Serial.println(inputParam);
        Serial.print("Input Message: ");
        Serial.println(inputMessage);
        Serial.print("Input Number: ");
        Serial.println(inputNum);
  });

  server.onNotFound(notFound);
  server.begin();
}




void loop() {

  // buttonStatus = readButton();
  int buttonState = readButton();
  buttonPressed = -1*(buttonState-1);
  runTime = millis();
  // char thermbuff[10];
  therm = analogRead(thermPin);
  Serial.print(buttonPressed);
  Serial.print(", ");
  Serial.println(therm);
  // dtostrf(therm, 8, 3, fbuff);

  delay(100);

  
}




