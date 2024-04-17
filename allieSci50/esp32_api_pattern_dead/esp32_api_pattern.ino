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
int thermPin = 34;
int buttonPin = 25; // internal pullup, ADC_2, 

int ledPin;

// Forward string declaration
std::string getPatternFlowRate();
std::vector<std::vector<float>> stringToArray(const std::string& input);
float currentDesiredFlowRate(std::vector<std::vector<float>> flowPattern, int current_time_ms);
std::vector<std::vector<float>> desiredFlowPattern;
std::string desiredFlowPatternString;
float time_start_ms;
int firstLoopFlag;

// Desired flow rate
float desiredFlowRate = 0; // Change, units SLPM
int Stop_Flag = 0; // Flag for big valve only going once 

float failFlowRate = 10987654321; // Dummy variable to check if flow rate check has failed
float endFlowRate = 12345678910; // Dummy variable to check if flow rate check has ended


// Web server???
AsyncWebServer server(80);


// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid     = "Claremont-ETC";
const char* password = "Cl@remontI0T";
char paramPrint[100]; // Assuming a maximum length for the concatenated string

// const char* PARAM_INPUT_1 = "pickLight";
const char* PARAM_INPUT_1 = "ctl-light";
const char* PARAM_INPUT_2 = "read-data";
const char* PARAM_INPUT_3 = "ctl-ledPin";


bool scriptRunning = false; // Flag to indicate whether the script is running

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
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


  // Combine the parameters into one variable

  strcpy(paramPrint, PARAM_INPUT_1);
  strcat(paramPrint, PARAM_INPUT_2);
  strcat(paramPrint, PARAM_INPUT_3);

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send_P(200, "text/plain", paramPrint);
  });

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
      digitalWrite(internalLedPin, HIGH);
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;

      // serializeJson(doc, output);

      // char timebuff[10];
      float runTime = millis();
      // dtostrf(runTime, 8, 3, timebuff);

      int buttonState = digitalRead(buttonPin);
      Serial.print(buttonState);
      // char buttonStateStr[16]; // Allocate a character array to store the string representation
      // itoa(buttonState, buttonStateStr, 10); // Convert int to string

      // char thermbuff[10];
      float therm = analogRead(thermPin);
      Serial.println(therm);
      // dtostrf(therm, 8, 3, fbuff);

      // char* output;
      // size_t outputCapacity;


      // JsonDocument doc;


      // Allocate the JSON document
      //
      // Inside the brackets, 200 is the RAM allocated to this document.
      // Don't forget to change this value to match your requirement.
      // Use arduinojson.org/v6/assistant to compute the capacity.
      StaticJsonDocument<200> doc;

      doc["time"] = runTime;
      doc["button status"] = buttonState;
      doc["thermistor"] = therm;

      // doc.shrinkToFit();  // optional
      serializeJsonPretty(doc, Serial);

      // Print a minified JSON document to the serial port
      String json_text;
      serializeJson(doc, json_text);
      // serializeJson(doc, WifiClient);
      Serial.print("textTest:");
      Serial.println(json_text);
      Serial.print("jsonSerial:");
      serializeJson(doc, Serial);

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



 
  server.onNotFound(notFound);
  server.begin();

  firstLoopFlag = 0;

}




void loop() {




  if (firstLoopFlag == 0) {
     desiredFlowPatternString = getPatternFlowRate(); // Input string of flow rates and durations
    //  Serial.println("desiredFlowPatternString");
    //  Serial.println(desiredFlowPatternString.c_str());
     desiredFlowPattern = stringToArray(desiredFlowPatternString); // Convert string to actual array
     Serial.println("data_begin");
     Serial.println("time_ms, set_point, flow_3300, flow_3400, total, control_effort, new_goal_position");
     time_start_ms = millis();
  }


  // Check flow pattern and get current flow rate
  float time_check_ms = millis() - time_start_ms;
  
  // Serial.println("desiredFlowPattern:");
  float oldDesiredFlowRate = desiredFlowRate;

  float desiredFlowRate = currentDesiredFlowRate(desiredFlowPattern, time_check_ms);

  if (oldDesiredFlowRate != desiredFlowRate) {Stop_Flag = 0;} // So the big valve re-checks if it should move every time flow rate changes

  if (desiredFlowRate == endFlowRate) {
    std::cout << "FLOW PATTERN ENDED AT " << time_check_ms << " ms, CLOSING VALVES" << std::endl;
    restartRun();  // Infinite loop to halt the program
  }
  else if (desiredFlowRate == failFlowRate) {
    std::cout << "FAILURE TO MATCH TIME AT " << time_check_ms << " ms, CLOSING VALVES" << std::endl;

    restartRun();  // Infinite loop to halt the program
  }


  
}


std::string getPatternFlowRate() {
  Serial.println("");
  Serial.println("Input your flow pattern.");
  Serial.println("");
  Serial.println("Flow rates in SLPM, duration in minutes.");
  Serial.println("");
  Serial.println("Flow patterns should be of the form [flow rate 1 , duration 1 ; flow rate 2 , duration 2 ; .... ; flow rate n , duration n].");
  Serial.println("");
  Serial.println("All parameters must be valid numbers. Spaces around , and ; are optional.");
  Serial.println("");
  while (!scriptRunning && !Serial.available()); // Wait for input
  // string input = Serial.readStringUntil('\n'); // Read input
  // std::string inputString = Serial.readStringUntil('\n');
  // std::string desiredFlowPattern = inputString;
  String inputString = Serial.readStringUntil('\n'); // Read input
  std::string desiredFlowPattern = inputString.c_str();
  if (desiredFlowPattern[0] == '[') {
    Serial.print("You inputted the following flow pattern: ");
    Serial.println(desiredFlowPattern.c_str()); // Convert to const char* for println    scriptRunning = true;
    // Add your script start code here
  } else {
    Serial.println("Your input was not accepted. Please enter a string in the provided form.");
    getPatternFlowRate(); // Call the function recursively until valid input is received
  }
  return desiredFlowPattern;
}



void printArray(const std::vector<std::vector<float>>& array) {
    for (const auto& row : array) {
        Serial.print("{");
        for (size_t i = 0; i < row.size(); ++i) {
            Serial.print(row[i]);
            if (i < row.size() - 1) {
                Serial.print(", ");
            }
        }
        Serial.print("}");
        if (&row != &array.back()) {
            Serial.print(", ");
        }
    }
    Serial.println();
}


void ledOn(float onTime) {
  digitalWrite(ledPin, HIGH);
  int onTime_ms = onTime*1000;
  delay(onTime_ms);
  digitalWrite(ledPin, LOW);
}
