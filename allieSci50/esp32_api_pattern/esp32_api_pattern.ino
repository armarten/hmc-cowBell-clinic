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
#include <string>

// GPIO Setup
// int ledPin = 32;  //34 and 35 are read-only
int internalLedPin = 2;  //to flash when get data request
// int buttonPin = 33;
int thermPin = 34;
int buttonPin = 25;  // internal pullup, ADC_2,
int ledPinG = 32;
int ledPinR = 33;
int ledPinW = 26;


// Output variable initialization
float runTime;
int buttonPressed;
float therm;

// Operation variable initialization
int runningFlag = 0;
int blinks = 0;
int lightswitch = 0;

// Web server???
AsyncWebServer server(80);


// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Claremont-ETC";
const char* password = "Cl@remontI0T";

// const char* PARAM_INPUT_1 = "pickLight";
const char* PARAM_INPUT_1 = "ctl-power";
const char* PARAM_INPUT_2 = "read-data";
const char* PARAM_INPUT_3 = "ctl-blinks";
const char* PARAM_INPUT_4 = "ctl-lightswitch";


void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}


int readButton() {
  int buttonStatus = digitalRead(buttonPin);
  return buttonStatus;
}

void whiteLED(int s) {
  digitalWrite(ledPinW, s);
}


void ledPattern(int buttonPressed1) {
  whiteLED(lightswitch);
  int ledPin = ledPinG;
  int i = 0;
  int j = 0;
  // int blinks = 5;
  int delayms;
  // int shrink;

  if (buttonPressed1 == 1) {
    delayms = 100;
    // shrink = 250;
  } else {
    delayms = 500;
    // shrink = 500;
  }
  while (i < blinks) {
    whiteLED(lightswitch);
    digitalWrite(ledPin, HIGH);
    while(j < 100) {
      delay(delayms/100);
      whiteLED(lightswitch);
      j = j+1;
    }
    j = 0;
    // delay(delayms);
    digitalWrite(ledPin, LOW);
    while(j < 100) {
      delay(delayms/100);
      whiteLED(lightswitch);
      j = j+1;
    }
    i++;
  }
  // delay(delayms + 1000);
}



void setup() {
  Serial.begin(115200);
  Serial.flush();
  pinMode(internalLedPin, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinW, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(thermPin, INPUT);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    delay(1000);
    esp_restart();
  }

  IPAddress ip = WiFi.localIP();

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(ip);
  delay(10000);
  

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    String params_all_text;

    params_all_text = String(PARAM_INPUT_1) + "\n" + String(PARAM_INPUT_2) + "\n" + String(PARAM_INPUT_3);

    request->send_P(200, "text/plain", params_all_text.c_str());
  });

  // text/plain: Used for plain text content. This could be used for simple textual responses that are not HTML formatted.

  // application/json: Used for JSON data. If you're sending JSON data back to the client, you would use this Content-Type.

  // application/xml or text/xml: Used for XML data. If you're sending XML data back to the client, you would use one of these Content-Types.

  // image/jpeg, image/png, image/gif, etc.: Used for image data. If you're sending image files or binary data representing images, you would use one of these Content-Types.

  // application/pdf: Used for PDF documents. If you're sending PDF files, you would use this Content-Type.

  // application/octet-stream: Used for arbitrary binary data. If you're sending non-textual binary data, you might use this Content-Type.

  // const char* PARAM_INPUT_1 = "ctl-power";
  // const char* PARAM_INPUT_2 = "read-data";
  // const char* PARAM_INPUT_3 = "ctl-blinks";


  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
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
        runningFlag = 1;
        Serial.println("Running Flag ON");
        request->send_P(200, "text/plain", "SUCCESS: Power ON");
      } 
      else if (inputMessage == "off") {
        runningFlag = 0;
        Serial.println("Running Flag OFF");

        request->send_P(200, "text/plain", "SUCCESS: Power OFF");
      } 
      else if (inputMessage == "reset") {
        Serial.println("Resetting.");
        request->send_P(200, "text/plain", "SUCCESS: Resetting");
        esp_restart();
      }
      else {
        request->send_P(200, "text/plain", "Invalid Power Command");
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


      request->send_P(200, "application/json", json_text.c_str());
      delay(100);
      digitalWrite(internalLedPin, LOW);
    }



    if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      inputNum = inputMessage.toInt();
      blinks = inputNum;
      String setString = "set to ";
      String modeString = " blinks";
      String pinOutputString = setString + inputMessage + modeString;

      request->send_P(200, "text/plain", pinOutputString.c_str());

    }


    if (request->hasParam(PARAM_INPUT_4)) {
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      // inputNum = inputMessage.toInt();
      if (inputMessage == "on") {
        lightswitch = 1;
      }
      else if (inputMessage == "off") {
        lightswitch = 0;
      }
      String setString = "The white LED is ";
      String pinOutputString = setString + inputMessage;
      Serial.println(pinOutputString);
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

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("DISCONNECTED FROM WIFI!");
    esp_restart();
  }

  while (runningFlag == 0) {
    Serial.print("runningFlag: ");
    Serial.println(runningFlag);
    whiteLED(lightswitch);
    delay(100);

  }


  while (blinks == 0) {
    Serial.print("blinks: ");
    Serial.println(blinks);
    whiteLED(lightswitch);
    delay(100);


  }
  whiteLED(lightswitch);

  int buttonState;

  int j = 0;

  digitalWrite(ledPinR, HIGH);

  while (j < 30) {
    whiteLED(lightswitch);
    buttonState = readButton();
    j = j+1;
    if (buttonState == 0) {
      break;
    }
    delay(100);

  }
  
  whiteLED(lightswitch);
  digitalWrite(ledPinR, LOW);
  delay(100);

  // buttonStatus = readButton();
  buttonPressed = -1 * (buttonState - 1);
  runTime = millis();
  // char thermbuff[10];
  therm = analogRead(thermPin);
  Serial.print(buttonPressed);
  Serial.print(", ");
  Serial.println(therm);
  // dtostrf(therm, 8, 3, fbuff);


  ledPattern(buttonPressed);

  delay(100);
}




// void ledPattern(int buttonPressed1) {
//   int i = 0;
//   // int blinks = 5;
//   int onms = 1000;
//   int offms = 500;
//   int shrinkon = 250;
//   int shrinkoff = 100;
//   int blinkPin;

//   if (buttonPressed1 == 1) {
//     // delayms = 1000;
//     // shrink = 250;
//     blinkPin = ledPinG;
//   } else {
//     // delayms = 5000;
//     // shrink = 500;
//     blinkPin = ledPinR;
//   }
//   while (i < blinks) {
//     digitalWrite(blinkPin, HIGH);
//     delay(onms - (shrinkon*i));
//     digitalWrite(blinkPin, LOW);
//     delay(offms - (shrinkoff*i));
//     i++;
//   }
//   // delay(delayms + 1000);
// }

