#include <SoftwareSerial.h>

unsigned long currentTime;

void setup() {
  // our debugging output
  Serial.begin(9600);
 
  // sensor baud rate is 9600
  Serial1.begin(9600); 
  Serial2.begin(9600);
  Serial3.begin(9600);
  
}
 
struct pms5003data1 {
  uint16_t framelen1;
  uint16_t pm10_standard1, pm25_standard1, pm100_standard1;
  uint16_t pm10_env1, pm25_env1, pm100_env1;
  uint16_t particles_03um1, particles_05um1, particles_10um1, particles_25um1, particles_50um1, particles_100um1;
  uint16_t unused1;
  uint16_t checksum1;
};

struct pms5003data2 {
  uint16_t framelen2;
  uint16_t pm10_standard2, pm25_standard2, pm100_standard2;
  uint16_t pm10_env2, pm25_env2, pm100_env2;
  uint16_t particles_03um2, particles_05um2, particles_10um2, particles_25um2, particles_50um2, particles_100um2;
  uint16_t unused2;
  uint16_t checksum2;
};

struct pms5003data3 {
  uint16_t framelen3;
  uint16_t pm10_standard3, pm25_standard3, pm100_standard3;
  uint16_t pm10_env3, pm25_env3, pm100_env3;
  uint16_t particles_03um3, particles_05um3, particles_10um3, particles_25um3, particles_50um3, particles_100um3;
  uint16_t unused3;
  uint16_t checksum3;
};


 
struct pms5003data1 data1;
struct pms5003data2 data2;
struct pms5003data3 data3;
    
void loop() {
  if (readPMSdata1(&Serial1)) {
    // reading data was successful!
  readPMSdata2(&Serial2);
  readPMSdata3(&Serial3);
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
  delay(500);
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

boolean readPMSdata2(Stream *s) {
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
  memcpy((void *)&data2, (void *)buffer_u16, 30);
 
  if (sum != data2.checksum2) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}

boolean readPMSdata3(Stream *s) {
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
  memcpy((void *)&data3, (void *)buffer_u16, 30);
 
  if (sum != data3.checksum3) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}