/************************************************************/
/*          Look around: push object away on WHITEFIELD     */
/************************************************************/

// typ hier de aansluitingen van de motordriver, voor de twee motors
#define motorrechtsPWM 3
#define motorrechtsDir 2
#define motorlinksPWM  5
#define motorlinksDir  4

//callibration variables
#define calib_speed_corrR   0  //if motors deviate, correct it
#define calib_speed_corrL   0  //if motors deviate, correct it

//robots
#define SAYA    0
#define GUDRUN  1
#define THIEMEN 2
#define JASPER  3
#define BLSTEF  4
#define STEF    5
#define MLOUISE 6
int ROBOT = MLOUISE;

//new batteries
#define newbat true

int calib_max_speed, calib_no_speed, SLOW_SPEED, search_turn_speed;

// afstandsmeting
#define trigPin 12
#define echoPin 13

bool test=false;

int speed_corr;
int right_speed;
int left_speed;
int turn_correction;

//afstand in cm die we onderzoeken voor baasje, verder zien we niet!
int max_zichtbare_afstand = 40; 
// wanneer stoppen?
int stop_afstand = 5; // stopt aan 5 cm
float distance_object;

bool onsearchfield = false;
bool finished = false;

#define NO_OBJECT      0
#define OBJECT_FAR     1
#define OBJECT_NEAR    2
#define OBJECT_COLLIDE 3


void setup(){
  if (newbat) {
    calib_max_speed = 180; //maximum value you want
    calib_no_speed  =  100; //lowest value that motors don't move anymore
    SLOW_SPEED      = 160;         //a slow speed good for searching
    turn_correction =  30;
    search_turn_speed = 130;
    if (ROBOT==THIEMEN)
      {calib_max_speed = 160;
      } else if (ROBOT=BLSTEF)
      {calib_max_speed = 140;
       calib_no_speed = 75;
      }
  } else {
  //old batteries
    calib_max_speed = 255; //maximum value you want
    calib_no_speed  = 110; //lowest value that motors don't move anymore
    SLOW_SPEED      = 200;         //a slow speed good for searching
    turn_correction =   0;
    search_turn_speed = 220;
  }
  if (test) {
    Serial.begin(9600);
  }
  // aansluitingen sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(motorlinksPWM, OUTPUT);
  pinMode(motorlinksDir, OUTPUT);
  pinMode(motorrechtsPWM, OUTPUT);
  pinMode(motorrechtsDir, OUTPUT);
}

void loop(){ 
  onsearchfield = true;
      if (!finished && onsearchfield){
        search_object();
        move_to_object();
        push_object();
      } else {
        delay(2000);
      }
  if (finished){
    motor_drive(0, 0);
  }
}

void search_object(){
  //we look for the object. First rotate left 180 degrees
  motor_drive(0,0);
      if (test) {
        Serial.println("Searching object!");
      }
  //delay(1000);
  //now rotate and scan
  motor_drive(-search_turn_speed,search_turn_speed);
  bool cont = true;
  while (cont) {
    switch (measure_distance(distance_object)) {
      case OBJECT_FAR:
      case OBJECT_NEAR:
      case OBJECT_COLLIDE:
        motor_drive(0,0);
        cont = false;
        break;
    }
  }
}

int measure_distance(float &distance){
  //determine how far an obstacle is
  // bepalen van afstand tot het blikje
  int duration;
  // 1 milliseconde kijken wat afstand van blik is
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  // berekenen afstand in cm (test dit en pas breuk aan als nodig!
  duration = pulseIn(echoPin, HIGH);
  //Merk op: Out of range == 0 cm!
  distance = (duration/2) / 29.1;
  if (test) {
        Serial.print("Dist ");Serial.print(distance);Serial.println(" ");
    }
  if (distance <= 0 || distance >= max_zichtbare_afstand){
    distance = max_zichtbare_afstand;
    return NO_OBJECT;
  } else if (distance > max_zichtbare_afstand/2.) {
    return OBJECT_FAR;
  } else if (distance > stop_afstand) {
    return OBJECT_NEAR;
  } else {
    return OBJECT_COLLIDE;
  }
}

void move_to_object(){
  //we go to object as long as we are on WHITEFIELD
  int speedval;
      if (test) {
        Serial.println("Moving to object");
      }
  motor_drive(0,0);
  int pos_obj;
  //pos_obj = measure_distance(distance_object);
  bool cont=true;
  while (cont){
    pos_obj = measure_distance(distance_object);
    switch (pos_obj){
      case NO_OBJECT:
        //problem, research the object
        search_object();
        break;
      case OBJECT_FAR:
        motor_drive(calib_max_speed, calib_max_speed);
      case OBJECT_NEAR:
        speedval = min(calib_max_speed, calib_max_speed * max(SLOW_SPEED/(calib_max_speed*1.0), (distance_object)/(max_zichtbare_afstand)));
        if (test){
          Serial.print("Moving with speed ");Serial.println(speedval);
        }
        motor_drive(speedval, speedval);
        delay(500);
        //motor_drive(0, 0);
        break;
      case OBJECT_COLLIDE:
        motor_drive(0, 0);
        cont=false;
        break;
    }
  }
}

void push_object(){
  // we now are or in front of object, 
  //we push it a bit, if ok, continue, otherwise finished.
  if (test) {
    Serial.println("Pushing object");
  }
  int pos_obj = measure_distance(distance_object);
  if (pos_obj == OBJECT_COLLIDE) {
    //we drive forward for 2 sec
    motor_drive(calib_max_speed, calib_max_speed);
    delay(1500);
    //motor_drive(-calib_max_speed, calib_max_speed);
    //delay(500);
    motor_drive(0,0);
    if (measure_distance(distance_object) == OBJECT_COLLIDE)
    {//we stop, stil object blocking us
      finished = true;
    }
  }
}

void motor_drive(int right_speed, int left_speed){
  // Drive motors according to the given values
  int sgvr = 1;
  if (right_speed != 0) {sgvr = abs(right_speed)/right_speed;}
  int sgvl = 1;
  if (left_speed != 0) {sgvl = abs(left_speed)/left_speed;}
  int vr = abs(right_speed) + calib_speed_corrR;
  int vl = abs(left_speed) + calib_speed_corrL;
  if (vr > calib_max_speed) {vr = calib_max_speed;}
  else if (vr<0) {vr = 0;}
  vr = vr * sgvr;
  if (vl > calib_max_speed) {vl = calib_max_speed;}
  else if (vl<0) {vl = 0;}
  vl = vl * sgvl;
  
  if (right_speed>=0){
    digitalWrite(motorrechtsDir, LOW); //vooruit
    analogWrite(motorrechtsPWM, right_speed);
  } else {
    digitalWrite(motorrechtsDir, HIGH); //achteruit
    analogWrite(motorrechtsPWM, 255+right_speed);
  }
  if (left_speed>=0){
    digitalWrite(motorlinksDir, LOW); //vooruit
    analogWrite(motorlinksPWM, left_speed);
  } else {
    digitalWrite(motorlinksDir, HIGH); //achteruit
    analogWrite(motorlinksPWM, 255+left_speed);
  }
}
