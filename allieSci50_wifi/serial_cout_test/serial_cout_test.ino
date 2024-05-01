#include <iostream> 

int i = 1;
float n = -1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.flush();
  Serial.println("Code Start");
  

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(i);
  std::cout << n << std::endl;
  i = i + 1;
  n = n - 1;
  delay(500);

}
