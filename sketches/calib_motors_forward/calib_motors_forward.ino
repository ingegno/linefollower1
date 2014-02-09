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
    Serial.println("Moving on a straight line forward for 3 seconds");
  }
  motor_drive(max_speed, max_speed); //Sends PWM signals to the motors
  delay(3000);
  motor_drive(0, 0); //Sends PWM signals to the motors
  delay(3000);
  
  if (test){
    Serial.println("Standing still for 3 seconds");
  }
  motor_drive(min_speed, min_speed); //Sends PWM signals to the motors
  delay(3000);
  motor_drive(0, 0); //Sends PWM signals to the motors
  delay(3000);
  
  if (test){
    Serial.println("Turning Right for 3 seconds");
  }
  motor_drive(int(min_speed+(max_speed-min_speed)/2.0), max_speed);
  delay(3000);
  motor_drive(0, 0); //Sends PWM signals to the motors
  delay(3000);
  
  if (test){
    Serial.println("Turning Left for 3 seconds");
  }
  motor_drive(max_speed, int(min_speed+(max_speed-min_speed)/2.0));
  delay(3000);
  motor_drive(0, 0); //Sends PWM signals to the motors
  delay(3000);
  
}


void motor_drive(int right_speed, int left_speed){
  if (test){
    Serial.print("  Driving right and left: ");
    Serial.print(right_speed);Serial.print(" ");
    Serial.println(left_speed);
  }
  digitalWrite(motorlinksDir, HIGH); //vooruit
  digitalWrite(motorrechtsDir, HIGH); //vooruit
  analogWrite(motorrechtsPWM, 255-right_speed);
  analogWrite(motorlinksPWM, 255-left_speed);
}

