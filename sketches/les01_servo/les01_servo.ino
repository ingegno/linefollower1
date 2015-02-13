/***************************************/
/*         Gebruik de Arduino om de 
/*         Servo's te controleren 
/**************************************/

#include <Servo.h> 
 
Servo servo1;
Servo servo2;
 
void setup() 
{ 
  servo1.attach( 9);  // connecteer servo op pin  9
  servo2.attach(10);  // connecteer servo op pin 10 
}
 
void loop() 
{ 
  servo1.write(90);
  servo2.write(90);
  delay(1000);
  servo1.write(180);
  servo2.write(0);
  delay(5000);
  servo1.write(0);
  servo2.write(90);
  delay(2000);
  servo1.write(45);
  delay(10000);
  servo1.write(90);
  servo2.write(180);
  delay(2000);
  servo2.write(90);
  delay(10);
} 
