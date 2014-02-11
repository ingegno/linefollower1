/*
Line following robot
This sketch to calibrate forward movement.
*/

// typ hier de aansluitingen van de motordriver, voor de twee motors
#define motorlinksPWM 3
#define motorlinksDir 2
#define motorrechtsPWM 5
#define motorrechtsDir 4

bool test=true;

int speed_corr;
int right_speed;
int max_speed = 240;
int min_speed = 100;
int left_speed;


void setup(){
  if (test) {
    Serial.begin(9600);
  }
  pinMode(motorlinksPWM, OUTPUT);
  pinMode(motorlinksDir, OUTPUT);
  pinMode(motorrechtsPWM, OUTPUT);
  pinMode(motorrechtsDir, OUTPUT);
  
}


void loop(){
  if (test){
    Serial.println("Rotating around axis, anticlock");
  }
  motor_drive(max_speed, -max_speed);
  delay(3000);
  motor_drive(0, 0); //Sends PWM signals to the motors
  delay(3000);
  
  if (test){
    Serial.println("Rotating around axis, other way");
  }
  motor_drive(-max_speed, max_speed);
  delay(3000);
  motor_drive(0, 0); //Sends PWM signals to the motors
  delay(3000);
}

void motor_drive(int right_speed, int left_speed){
  // Drive motors according to the calculated values
  // Normally 255 - speed as we have Dir LOW, but h
  if (test){
    Serial.print("  Driving right and left: ");
    Serial.print(right_speed);Serial.print(" ");
    Serial.println(left_speed);
  }
  if (right_speed>=0){
    digitalWrite(motorrechtsDir, HIGH); //vooruit
    analogWrite(motorrechtsPWM, 255-right_speed);
  } else {
    digitalWrite(motorrechtsDir, LOW); //achteruit
    analogWrite(motorrechtsPWM, -right_speed);
  }
  if (left_speed>=0){
    digitalWrite(motorlinksDir, HIGH); //vooruit
    analogWrite(motorlinksPWM, 255-left_speed);
  } else {
    digitalWrite(motorlinksDir, LOW); //achteruit
    analogWrite(motorlinksPWM, -left_speed);
  }
}

