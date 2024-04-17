#include <SoftwareSerial.h>


unsigned long currentTime;

void setup() {
  // our debugging output
  Serial.begin(9600);
 
  // sensor baud rate is 9600
  Serial1.begin(9600); 
  
}
 
struct pms5003data1 {
  uint16_t framelen1;
  uint16_t pm10_standard1, pm25_standard1, pm100_standard1;
  uint16_t pm10_env1, pm25_env1, pm100_env1;
  uint16_t particles_03um1, particles_05um1, particles_10um1, particles_25um1, particles_50um1, particles_100um1;
  uint16_t unused1;
  uint16_t checksum1;
};


 
struct pms5003data1 data1;
    
void loop() {
  if (readPMSdata1(&Serial1)) {
  currentTime = millis();
  Serial.print(currentTime); 
  Serial.print(","); 
  Serial.print(data1.particles_03um1); 
  Serial.print(","); 
  Serial.print(data1.particles_05um1); 
  Serial.print(","); 
  Serial.print(data1.particles_10um1); 
  Serial.print(",");
  Serial.println(data1.particles_25um1); 
}
}
 
boolean readPMSdata1(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
 
  /* debugging
  for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
  }
  Serial.println();
  */
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&data1, (void *)buffer_u16, 30);
 
  if (sum != data1.checksum1) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
 