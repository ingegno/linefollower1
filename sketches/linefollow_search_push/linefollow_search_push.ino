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
int ROBOT = SAYA;

//new batteries
#define newbat true

int calib_max_speed, calib_no_speed, SLOW_SPEED, search_turn_speed;
//based on sensors an extra slowdown can be set, it will be substracted from the speed!
int extra_slowdown=0;
unsigned long slowdown_start = 0UL;

// afstandsmeting
#define trigPin 12
#define echoPin 13

bool test=false;

//geen PID control http://en.wikipedia.org/wiki/PID_controller
// enkel proportioneel de afwijking van middellijn in rekening brengen

#define SCHAAL_FOUT 1000

int black[2]  = {600,1100}; // hiertussen zouden de waarden voor zwart moeten zijn
int white[2]  = { 0, 500};
// correction callibration
int corrwhite[5] = {0,0,0,0,0};
int corrblack[5] = {0,0,0,0,0};

// Wat kunnen we zien met 5 sensoren van lijnsensor?
#define LS_UNKNOWN    0 // we kunnen niet bepalen wat we zien
#define LS_BLACKLINE  1 // een zwarte lijn
#define LS_BLACKFIELD 2 // een zwart veld
#define LS_WHITEFIELD 3 // een wit veld
#define LS_BLACKLEFT  4 // zwart afbuigend naar links
#define LS_BLACKRIGHT 5 // zwart afbuigend naar rechts
#define LS_BLACKSPLIT 6 // zwart links en rechts, niet midden
#define LS_BLACKEXTREMELEFT  7 // zwart uiterst links
#define LS_BLACKEXTREMERIGHT 8 // zwart uiterst rechts
#define LS_BLACKBANKLEFT     9 // zwart uiterst rechts
#define LS_BLACKBANKRIGHT   10 // zwart uiterst rechts

//wijzigende variabelen
float sensors_average;
float sensors_average_bl; // hier houden we rekening met een baseline, dat is we trekken het minimun ervan af

float mid_point = 2; //midden van de sensoren 0,1,2,3,4
float max_err = max(4-mid_point, mid_point-0) * SCHAAL_FOUT;
float min_err = 0.5 * SCHAAL_FOUT;
long error_value;

// Array om de 5 sensorlezingen in op te slaan
long sensors[5] = {0, 0, 0, 0, 0};

//enkele extra variabelen
// hoeveel verschil moet er zijn tussen grootste en kleinste waarden 
// om te zeggen dat er een lijn gezien is
#define ACCURACY 100

int speed_corr;
int right_speed;
int left_speed;
int turn_correction;

int max_zichtbare_afstand = 55; //afstand in cm die we onderzoeken, verder zien we niet!
// wanneer stoppen?
int stop_afstand = 5; // stopt aan 10 cm
float distance_object;

bool onsearchfield = false;
unsigned long prevtimewhitefield = 0UL;
bool finished = false;
#define NO_OBJECT      0
#define OBJECT_FAR     1
#define OBJECT_NEAR    2
#define OBJECT_COLLIDE 3


void setup(){
  if (newbat) {
    calib_max_speed = 180; //maximum value you want
    calib_no_speed  =  110; //lowest value that motors don't move anymore
    SLOW_SPEED      = 120;         //a slow speed good for searching
    turn_correction =  40;
    search_turn_speed = 100;
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
  if (ROBOT == THIEMEN){
    black[0] = 700;
  } else if (ROBOT == JASPER){
    black[0] = 700;
    white[1] = 600;
  }
  if (ROBOT == SAYA){
    corrwhite[0]=0;corrwhite[1]=0;corrwhite[2]=0;corrwhite[3]=100;corrwhite[4]=200;
    corrblack[0]=0;corrblack[1]=50;corrblack[2]=0;corrblack[3]=20;corrblack[4]=50;
  } else if (ROBOT == GUDRUN){
    corrwhite[0]=0;corrwhite[1]=0;corrwhite[2]=50;corrwhite[3]=0;corrwhite[4]=0;
    corrblack[0]=0;corrblack[1]=-10;corrblack[2]=20;corrblack[3]=0;corrblack[4]=0;
  } else if (ROBOT == THIEMEN){
    corrwhite[0]=0;corrwhite[1]=0;corrwhite[2]=0;corrwhite[3]=0;corrwhite[4]=0;
    corrblack[0]=0;corrblack[1]=70;corrblack[2]=50;corrblack[3]=30;corrblack[4]=40;
  } else if (ROBOT == JASPER){
    corrwhite[0]=100;corrwhite[1]=0;corrwhite[2]=0;corrwhite[3]=0;corrwhite[4]=100;
    corrblack[0]=0;corrblack[1]=0;corrblack[2]=0;corrblack[3]=0;corrblack[4]=0;
  } else if (ROBOT == BLSTEF){
    corrwhite[0]=40;corrwhite[1]=0;corrwhite[2]=0;corrwhite[3]=0;corrwhite[4]=0;
    corrblack[0]=40;corrblack[1]=0;corrblack[2]=0;corrblack[3]=0;corrblack[4]=0;
  } else if (ROBOT == STEF){
    corrwhite[0]=0;corrwhite[1]=0;corrwhite[2]=100;corrwhite[3]=0;corrwhite[4]=0;
    corrblack[0]=0;corrblack[1]=0;corrblack[2]=0;corrblack[3]=0;corrblack[4]=0;
  } else if (ROBOT == MLOUISE){
    corrwhite[0]=0;corrwhite[1]=100;corrwhite[2]=0;corrwhite[3]=0;corrwhite[4]=40;
    corrblack[0]=0;corrblack[1]=0;corrblack[2]=0;corrblack[3]=0;corrblack[4]=0;
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
  //unset slowdown if longer than 4 sec
  if (extra_slowdown > 0 && (millis()-slowdown_start > 4000UL)) {
    extra_slowdown = 0;
  }
  //Reads sensor values and computes sensor sum and weighted average
  int nrseen = 0;
  int lineseen = sensors_read();
  onsearchfield = false;
  switch (lineseen) {
    case LS_BLACKLINE:
    case LS_BLACKBANKLEFT:
    case LS_BLACKLEFT:
    case LS_BLACKBANKRIGHT:
    case LS_BLACKRIGHT:
      finished = false;
      onsearchfield = false;
      calc_turn();
      //Computes the error to be corrected
      motor_drive(right_speed, left_speed); //Sends PWM signals to the motors
      break;
    case LS_UNKNOWN:
      //can't decide what we have seen. stop and read again
      motor_drive(0, 0);
      if (test) {
        Serial.println("No line seen !!!");
      }
      break;
    case LS_WHITEFIELD:
      motor_drive(0,0);
      onsearchfield = false;
      if (test) {
        Serial.println("white line !!!");
      }
      if (millis() - prevtimewhitefield < 1500UL) {
        //really on white field
        motor_drive(0,0);
        onsearchfield = true;
      } else {
        //back up a bit to find the line again
        prevtimewhitefield = millis();
        motor_drive(0,0);
        delay(100);
        motor_drive(-calib_max_speed+turn_correction, -calib_max_speed+turn_correction);
        delay(500);
        motor_drive(0,0);
        delay(100);
        // slow down before continuing
        extra_slowdown = SLOW_SPEED + (calib_max_speed-SLOW_SPEED)/2;
      }
//      //we do three repeats
//      for (int seeagain=0; seeagain<3; seeagain++) {
//        if (LS_WHITEFIELD == sensors_read()){
//          nrseen += 1;
//          delay(200);
//        }
//      }
//      if (nrseen == 3) {
//        onsearchfield = true;
//      } else {
//        onsearchfield = false;
//      }      
      if (!finished && onsearchfield){
        search_object();
        move_to_object();
        push_object();
      } // else: make him follow line again next loop
      break;
    case LS_BLACKFIELD:
      //a black field. here we should stop. wait a sec, try again, if still problem, go backward a bit, try again?
      motor_drive(0, 0);
      break;
    case LS_BLACKEXTREMELEFT:
      // we assume a sharp turn to left
      motor_drive(calib_max_speed-turn_correction, -calib_max_speed+turn_correction);
      delay(100);
      motor_drive(0,0);
      break;
//    case LS_BLACKBANKLEFT:
//      // we assume a sharp turn to left
//      motor_drive(calib_max_speed-turn_correction, 0);
//      delay(100);
//      motor_drive(0,0);
//      break;
    case LS_BLACKEXTREMERIGHT:
      // we assume a sharp turn to right
      motor_drive(-calib_max_speed+turn_correction , calib_max_speed-turn_correction);
      delay(100);
      motor_drive(0,0);
      break;
//    case LS_BLACKBANKRIGHT:
//      // we assume a sharp turn to right
//      motor_drive(0 , calib_max_speed-turn_correction);
//      delay(100);
//      motor_drive(0,0);
//      break;
    default:
      motor_drive(0, 0);
      break;
  }
  if (finished){
    motor_drive(0, 0);
  }
}

boolean sensors_read(){
  /*
  Method to read the sensors. Return type of black seen, and does
  necessary computations to know where the line is.
  */
  int sensors_sum = 0;
  int sensors_sum_bl = 0;
  int baseline = 1024;
  int topline = 0;
  sensors_average = 0;
  sensors_average_bl = 0;
  for (int i = 0; i < 5; i++){
    sensors[i] = analogRead(i);
    if (sensors[i] < 501){ sensors[i] = sensors[i]-corrwhite[i];}
    else { sensors[i] = sensors[i]-corrblack[i];}
    if (baseline > sensors[i]){
      baseline = sensors[i];
    }
    if (topline < sensors[i]){
      topline = sensors[i];
    }
  }
  for (int i = 0; i<5; i++){
    int val = sensors[i];
    sensors_average += val * i;
    //Calculating the weighted mean 
    sensors_sum += val;
    sensors_average_bl += (val-baseline) * i; 
    //Calculating the weighted mean maar nu trekken we laagste waarde ervan af
    sensors_sum_bl += val-baseline;
  }
  sensors_average=sensors_average/sensors_sum;         // gewogen gemiddelde
  sensors_average_bl=sensors_average_bl/sensors_sum_bl;// gewogen gemiddelde met baseline
  
  //de foutwaarde
  error_value = long((sensors_average_bl - mid_point) * SCHAAL_FOUT);

  int seen = LS_UNKNOWN;
  if (baseline > black[0]){
    seen = LS_BLACKFIELD;
  } else if (topline < white[1]){
    seen = LS_WHITEFIELD;
  } else if (topline-baseline < ACCURACY){
    // no line seen but not black or white field
    seen = LS_UNKNOWN;
  } else if (sensors[2] < white[1] && sensors[0] > black[0] && sensors[4] > black[0]) {
    seen = LS_BLACKSPLIT;
  } else if (sensors[0] < white[1] && sensors[3] > black[0] && sensors[2] < black[0]) {
    seen = LS_BLACKLEFT;
  } else if (sensors[0] < white[1] && sensors[3] > black[0] && sensors[4] > black[0]) {
    seen = LS_BLACKBANKLEFT;
  } else if (sensors[0] < white[1] && sensors[4] > black[0]) {
    seen = LS_BLACKEXTREMELEFT;
  } else if (sensors[4] < white[1] && sensors[1] > black[0] && sensors[2] < black[0]) {
    seen = LS_BLACKRIGHT;
  } else if (sensors[4] < white[1] && sensors[0] > black[0] && sensors[1] > black[0]) {
    seen = LS_BLACKBANKRIGHT;
  } else if (sensors[4] < white[1] && sensors[0] > black[0]) {
    seen = LS_BLACKEXTREMERIGHT;
  } else {
    seen = LS_BLACKLINE;
  }
  if (test) {
    motor_drive(0,0);
    Serial.print("prev speed: "); Serial.print(right_speed); Serial.print(" "); Serial.println(left_speed);
    Serial.print("I see:"); Serial.println(seen);
    Serial.print("Corrected measured: "); 
    for (int i = 0; i<5; i++){
      Serial.print(sensors[i]);Serial.print(" ");
    }
    Serial.println(" ");
    delay(2000);
  }
  return seen;
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
      case OBJECT_NEAR:
        speedval = min(calib_max_speed, calib_max_speed * max(SLOW_SPEED/(calib_max_speed*1.0), (distance_object)/(max_zichtbare_afstand)));
        Serial.print("Moving with speed ");Serial.println(speedval);
        motor_drive(speedval, speedval);
        delay(500);
        //motor_drive(0, 0);
        break;
      case OBJECT_COLLIDE:
        motor_drive(0, 0);
        cont=false;
        break;
    }
    // test if still WHITEFIELD !
    int nrwhite = 0;
    int lineseen;
    for (int ind=0; ind<5; ind++) {
      lineseen = sensors_read();
      if (lineseen == LS_WHITEFIELD) {nrwhite++;}
    }
    if (nrwhite < 4) {
      //problem, at edge of field!, stop and search next cycle.
      cont = false;
      motor_drive(-180,180);
      delay(500);
      motor_drive(0,0);
    }
  }
}

void push_object(){
  // we now are or in front of object, or on edge search field. In first case, push it out.
      if (test) {
        Serial.println("Pushing object");
      }
  int pos_obj = measure_distance(distance_object);
  if (pos_obj == OBJECT_COLLIDE) {
    //we drive forward slowly for 7 sec?
    motor_drive(calib_max_speed, calib_max_speed);
    delay(8000);
    motor_drive(calib_max_speed,-calib_max_speed);
    delay(2000);
    //finished = true;
    motor_drive(0,0);
  }
}

void calc_turn(){
  
  //Restricting the error value between -max_err, -min_err and min_err, max_err.
  if (error_value > max_err){
    // not possible ...
    error_value = max_err;
  } else if (error_value < -max_err) {
    // not possible ...
    error_value = -max_err;
  }
  if (abs(error_value) < min_err){
    // don't correct yet
    error_value = 0;
  }
  //convert to correct number
  speed_corr = float(error_value)/float(max_err) * (calib_max_speed-calib_no_speed);
  if (error_value < -1000) {
    //line at sensor 0 move line slowly towards 2 (to left), by a right turn, so reducing speed right
    right_speed = 0; 
    left_speed = SLOW_SPEED;
    if (ROBOT == THIEMEN) {
      right_speed = -SLOW_SPEED;
      left_speed  = SLOW_SPEED;
    }
  } else if (error_value < 0){
    //line at sensor 0 to 2, move line towards 2 (to left), by reducing speed right
    right_speed = calib_max_speed + speed_corr;
    left_speed = calib_max_speed;
    if (ROBOT == THIEMEN) {
      left_speed  = calib_max_speed + 2*speed_corr;
    }
  } else if (error_value > 1000) {
    //line at sensor 4 move line slowly towards 3 (to right), by a left turn, so reducing speed left
    right_speed = SLOW_SPEED;
    left_speed = 0;
    if (ROBOT == THIEMEN) {
      right_speed = SLOW_SPEED;
      left_speed  = -SLOW_SPEED;
    }
  } else {
    //line at sensor 2 to 4, move line towards 2, by reducing speed left
    right_speed = calib_max_speed;
    left_speed = calib_max_speed - speed_corr;
    if (ROBOT == THIEMEN) {
      left_speed  = calib_max_speed - 2*speed_corr;
    }
  }
}

void motor_drive(int right_speed, int left_speed){
  // Drive motors according to the given values
  int sgvr = 1;
  if (right_speed != 0) {sgvr = abs(right_speed)/right_speed;}
  int sgvl = 1;
  if (left_speed != 0) {sgvl = abs(left_speed)/left_speed;}
  int vr = abs(right_speed) - extra_slowdown + calib_speed_corrR;
  int vl = abs(left_speed) - extra_slowdown + calib_speed_corrL;
  if (vr > calib_max_speed) {vr = calib_max_speed;}
  else if (vr<0) {vr = 0;}
  vr = vr * sgvr;
  if (vl > calib_max_speed) {vl = calib_max_speed;}
  else if (vl<0) {vl = 0;}
  vl = vl * sgvl;
  
  if (right_speed>=0){
    digitalWrite(motorrechtsDir, LOW); //vooruit
    analogWrite(motorrechtsPWM, vr);
  } else {
    digitalWrite(motorrechtsDir, HIGH); //achteruit
    analogWrite(motorrechtsPWM, 255+vr);
  }
  if (left_speed>=0){
    digitalWrite(motorlinksDir, LOW); //vooruit
    analogWrite(motorlinksPWM, vl);
  } else {
    digitalWrite(motorlinksDir, HIGH); //achteruit
    analogWrite(motorlinksPWM, 255+vl);
  }
}
