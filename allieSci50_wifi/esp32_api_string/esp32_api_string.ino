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

// int ledPin = 2;  //34 and 35 are read-only
// int internalLedPin = 2;
int buttonPin = 35;



// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid     = "Claremont-ETC";
const char* password = "Cl@remontI0T";

// const char* PARAM_INPUT_1 = "pickLight";
const char* PARAM_INPUT_1 = "ctl-light";
const char* PARAM_INPUT_2 = "read-button";
const char* PARAM_INPUT_3 = "ctl-ledPin";



// HTML web page to handle 3 input fields (input1, input2, input3)
// const char index_html_pick[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html><head>
//   <title>Pick an LED</title>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   </head><body>
//   <p>Please select which LED to operate:</p>
//   <p><i>Pin 2 is the internal LED</i></p>
//   <form action="/get">
//     Enter pin number: <input type="text" name="pickLight">
//     <input type="submit" value="yep">
//   </form><br>
// </body></html>)rawliteral";


// const char index_html_switch[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html><head>
//   <title>Turn LED On and Off</title>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   </head><body>
//   <form action="/get">
//     <input type="radio" id="ledON" name="lightswitch" value="1">
//     <label for="ledON">ON</label><br>
//     <input type="radio" id="ledOFF" name="lightswitch" value="0">
//     <label for="ledOFF">OFF</label><br>
//     <input type="submit" value="Let there be light, or not.">
//   </form><br>
//   <br><a href="/">Pick a different LED</a>
// </body></html>)rawliteral";

// <input type="radio" id="internal" name="pickLight" value="2">
//     <label for="internal">Internal LED</label><br>
//     <input type="radio" id="external" name="pickLight" value="32">
//     <label for="external">External LED</label><br>


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  Serial.flush();
  // pinMode(internalLedPin, OUTPUT);
  // pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
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

    // Send web page with input fields to client
  // server.on("/buttonStatus", HTTP_GET, [](AsyncWebServerRequest *request){
  // // char timeVal[16];
  // // ltoa(millis(),timeVal,10);
  // // request->send_P(200, "text/plain", timeVal);
  //   int buttonState = digitalRead(buttonPin);
  //   char buttonStateStr[16]; // Allocate a character array to store the string representation
  //   itoa(buttonState, buttonStateStr, 10); // Convert int to string
    
  //   request->send_P(200, "text/plain", buttonStateStr);
  // });




    // text/plain: Used for plain text content. This could be used for simple textual responses that are not HTML formatted.

    // application/json: Used for JSON data. If you're sending JSON data back to the client, you would use this Content-Type.

    // application/xml or text/xml: Used for XML data. If you're sending XML data back to the client, you would use one of these Content-Types.

    // image/jpeg, image/png, image/gif, etc.: Used for image data. If you're sending image files or binary data representing images, you would use one of these Content-Types.

    // application/pdf: Used for PDF documents. If you're sending PDF files, you would use this Content-Type.

    // application/octet-stream: Used for arbitrary binary data. If you're sending non-textual binary data, you might use this Content-Type.

// const char* PARAM_INPUT_1 = "ctl-light";
// const char* PARAM_INPUT_2 = "read-button";
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
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;

      int buttonState = digitalRead(buttonPin);
      char buttonStateStr[16]; // Allocate a character array to store the string representation
      itoa(buttonState, buttonStateStr, 10); // Convert int to string
      request->send_P(200, "text/plain", buttonStateStr);
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
    


    


    // else if (request->hasParam(PARAM_INPUT_2)) {
    //   inputMessage = request->getParam(PARAM_INPUT_2)->value();
    //   inputParam = PARAM_INPUT_2;
    //   inputNum = inputMessage.toInt();
    //   // Serial.print("Input Param: ");
    //   // Serial.println(inputParam);
    //   // Serial.print("Input Message: ");
    //   // Serial.println(inputMessage);
    //   // Serial.print("Input Number: ");
    //   // Serial.println(inputNum);

    //   if (inputNum == 1) {
    //     digitalWrite(ledPin, HIGH);
    //     Serial.println("Lights ON");
    //   }
    //   else if (inputNum == 0) {
    //     digitalWrite(ledPin, LOW);
    //     Serial.println("Lights OFF");
    //   }
    // }
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


