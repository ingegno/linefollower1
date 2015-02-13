/*
Line following robot
This sketch to calibrate forward movement.
*/

// typ hier de aansluitingen van de motordriver, voor de twee motors
#define motorrechtsin3 4
#define motorrechtsin4 2
#define motorrechtsenableB 3

#define motorlinksin1  5
#define motorlinksin2  7
#define motorlinksenableA 6


bool test=false;

//callibration variables
#define calib_speed_corrR   0  //if motors deviate, correct it
#define calib_speed_corrL   0  //if motors deviate, correct it
//new batteries
#define newbat true

int calib_max_speed, calib_no_speed;

//internal variables
int right_speed;
int max_speed;
int min_speed;
int left_speed;


void setup(){
  if (newbat) {
    calib_max_speed = 180; //maximum value you want
    calib_no_speed  =  70; //lowest value that motors don't move anymore
  } else {
  //old batteries
    calib_max_speed = 255; //maximum value you want
    calib_no_speed  = 110; //lowest value that motors don't move anymore
  }
  max_speed = calib_max_speed;
  min_speed = calib_no_speed;
  if (test) {
    Serial.begin(9600);
  }
  
   pinMode( motorrechtsin3,OUTPUT);
 pinMode(motorrechtsin4, OUTPUT);
 pinMode(motorrechtsenableB,OUTPUT);

 pinMode(motorlinksin1,OUTPUT);
 pinMode( motorlinksin2, OUTPUT);
 pinMode(motorlinksenableA,OUTPUT);
  
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
  // Drive motors according to the given values
  if (test){
    Serial.print("  Driving right and left: ");
    Serial.print(right_speed);Serial.print(" ");
    Serial.println(left_speed);
  }
  //apply calibration to the speeds if needed:
  int sgvr = 1;
  if (right_speed != 0) {sgvr = abs(right_speed)/right_speed;} //teken right_speed
  int sgvl = 1;
  if (left_speed != 0) {sgvl = abs(left_speed)/left_speed;} //teken left_speed
  int vr = abs(right_speed) + calib_speed_corrR;
  int vl = abs(left_speed) + calib_speed_corrL;
  if (vr > calib_max_speed) {vr = calib_max_speed;}
  else if (vr<0) {vr = 0;}
  vr = vr * sgvr;
  if (vl > calib_max_speed) {vl = calib_max_speed;}
  else if (vl<0) {vl = 0;}
  vl = vl * sgvl;
 
  if (right_speed>=0){
    
    digitalWrite(motorrechtsin3, LOW); //vooruit
    digitalWrite(motorrechtsin4, HIGH);
    analogWrite(motorrechtsenableB, right_speed);
  } else {
     digitalWrite(motorrechtsin3, HIGH); //achteruit
    digitalWrite(motorrechtsin4, LOW);
    analogWrite(motorrechtsenableB, right_speed);
  }
  if (left_speed>=0){
    digitalWrite(motorlinksin1, LOW); //vooruit
    digitalWrite(motorlinksin2, HIGH);
    analogWrite(motorlinksenableA, right_speed);
  } else {
     digitalWrite(motorlinksin1, HIGH); //achteruit
    digitalWrite(motorlinksin2, LOW);
    analogWrite(motorlinksenableA, right_speed);
  }
  
}
