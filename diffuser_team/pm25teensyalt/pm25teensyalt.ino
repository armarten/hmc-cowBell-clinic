/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */

#include "Adafruit_PM25AQI.h"

// If your PM2.5 is UART only, for UNO and others (without hardware serial) 
// we must use software serial...
// pin #2 is IN from sensor (TX pin on sensor), leave pin #3 disconnected
// comment these two lines if using hardware serial
//#include <SoftwareSerial.h>
//SoftwareSerial pmSerial(2, 3);

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

void setup() {
  // Wait for serial monitor to open
  Serial.begin(9600);
  while (!Serial) delay(10);

  Serial.println("Adafruit PMSA003I Air Quality Sensor");

  // Wait one second for sensor to boot up!
  delay(1000);

  // If using serial, initialize it and set baudrate before starting!
  // Uncomment one of the following
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  //pmSerial.begin(9600);

  // There are 3 options for connectivity!
//  if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
  if (! aqi.begin_UART(&Serial1)) { // connect to the sensor over hardware serial
  //if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial 
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");
}

void loop() {
  PM25_AQI_Data data1;
  
  if (! aqi.read(&data1)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
 currentTime = millis();
  Serial.print(currentTime); 
  Serial.print(","); 
  Serial.print(data1.particles_03um1); 
  Serial.print(","); 
  Serial.print(data1.particles_05um1); 
  Serial.print(","); 
  Serial.print(data1.particles_10um1); 
  Serial.print(",");
  Serial.print(data1.particles_25um1); 
  Serial.print(","); 
  Serial.print(data2.particles_03um2); 
  Serial.print(","); 
  Serial.print(data2.particles_05um2); 
  Serial.print(","); 
  Serial.print(data2.particles_10um2); 
  Serial.print(",");
  Serial.print(data2.particles_25um2); 
  Serial.print(","); 
  Serial.print(data3.particles_03um3); 
  Serial.print(","); 
  Serial.print(data3.particles_05um3); 
  Serial.print(","); 
  Serial.print(data3.particles_10um3); 
  Serial.print(",");
  Serial.println(data3.particles_25um3); 
  

  delay(1000);
}