/************************************************************/
/*          Look around: push object away on WHITEFIELD     */
/************************************************************/

// typ hier de aansluitingen van de motordriver, voor de twee motors
#define motorlinksPWM 3
#define motorlinksDir 2
#define motorrechtsPWM 5
#define motorrechtsDir 4

//callibration variables
#define calib_speed_corrR   0  //if motors deviate, correct it
#define calib_speed_corrL   0  //if motors deviate, correct it
#define calib_max_speed    255 //maximum value you want
#define calib_no_speed     100 //lowest value that motors don't move anymore
#define SLOW_SPEED 120         //a slow speed good for searching

// afstandsmeting
#define trigPin 12
#define echoPin 13

bool test=false;

//geen PID control http://en.wikipedia.org/wiki/PID_controller
// enkel proportioneel de afwijking van middellijn in rekening brengen

#define SCHAAL_FOUT 1000

int black[2]  = {700,1000}; // hiertussen zouden de waarden voor zwart moeten zijn
//test of je groen, geel, wit kan zien
//waarschijnlijk niet ....
int green[2]  = { 75, 100};
int yellow[2] = { 80, 105};
int white[2]  = { 0, 500};

int corrwhite[5] = {0, 0, 0, 100, 200};
int corrblack[5] = {0, 50, 0, 120, 50};

// Wat kunnen we zien met 5 sensoren van lijnsensor?
#define LS_UNKNOWN    0 // we kunnen niet bepalen wat we zien
#define LS_BLACKLINE  1 // een zwarte lijn
#define LS_BLACKFIELD 2 // een zwart veld
#define LS_WHITEFIELD 3 // een wit veld
#define LS_BLACKLEFT  4 // zwart afbuigend naar rechts
#define LS_BLACKRIGHT 5 // zwart afbuigend naar links
#define LS_BLACKSPLIT 6 // zwart links en rechts, niet midden

//wijzigende variabelen
float sensors_average;
float sensors_average_bl; // hier houden we rekening met een baseline, dat is we trekken het minimun ervan af

float mid_point = 2; //midden van de sensoren 0,1,2,3,4
float max_err = max(4-mid_point, mid_point-0) * SCHAAL_FOUT;
float min_err = 0.5 * SCHAAL_FOUT;
long error_value;

// Array om de 5 sensorlezingen in op te slaan
long sensors[] = {0, 0, 0, 0, 0};

//enkele extra variabelen
// hoeveel verschil moet er zijn tussen grootste en kleinste waarden 
// om te zeggen dat er een lijn gezien is
#define ACCURACY 100

int speed_corr;
int right_speed;
int left_speed;


int max_zichtbare_afstand = 80; //afstand in cm die we onderzoeken, verder zien we niet!
// wanneer stoppen?
int stop_afstand = 5; // stopt aan 10 cm
float distance_object;

bool onsearchfield = false;
bool finished = false;
#define NO_OBJECT      0
#define OBJECT_FAR     1
#define OBJECT_NEAR    2
#define OBJECT_COLLIDE 3


void setup(){
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
  } else if (sensors[0] < white[1] && sensors[3] > black[0] && sensors[4] > black[0]) {
    seen = LS_BLACKLEFT;
  } else if (sensors[4] < white[1] && sensors[1] > black[0] && sensors[0] > black[0]) {
    seen = LS_BLACKRIGHT;
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

void loop(){ 
  //Reads sensor values and computes sensor sum and weighted average
  int lineseen = sensors_read();
  switch (lineseen) {
    case LS_BLACKLINE:
      finished = false;
      onsearchfield = false;
      motor_drive(0, 0);
      if (test) {
        Serial.println("black line !!!");
      }
      break;
    case LS_UNKNOWN:
      //can't decide what we have seen. stop and read again
      motor_drive(0, 0);
      if (test) {
        Serial.println("No line seen !!!");
      }
      break;
    case LS_WHITEFIELD:
      //a white field. here we should stop, go backward a bit, verify, go forward, search for can
      if (!finished && !onsearchfield){
        motor_drive(-160,-160);
        delay(500);
        motor_drive(0,0);
        lineseen = sensors_read();
        if (lineseen == LS_BLACKLINE){
          // good, move forward again
          onsearchfield = true;
          motor_drive(160,160);
          delay(500);
        }
      }
      if (!finished && onsearchfield){
        motor_drive(0,0);
        search_object();
        move_to_object();
        push_object();
      } // else: make him follow line again next loop
      break;
    case LS_BLACKFIELD:
      //a black field. here we should stop. wait a sec, try again, if still problem, go backward a bit, try again?
      motor_drive(0, 0);
      break;
    case LS_BLACKLEFT:
      // we assume a sharp turn to left
      motor_drive(0, 0);
      break;
    case LS_BLACKRIGHT:
      // we assume a sharp turn to right
      motor_drive(0, 0);
      break;
    default:
      motor_drive(0,0);
      break;
  }
  if (finished){
    motor_drive(0,0);
  }
}


void search_object(){
  //we look for the object. First rotate left 180 degrees
  motor_drive(0,0);
  delay(1000);
  //now rotate and scan
  motor_drive(-180,180);
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
  motor_drive(0,0);
  int pos_obj;
  pos_obj = measure_distance(distance_object);
  bool cont=true;
  while (cont){
    switch (pos_obj){
      case NO_OBJECT:
        //problem, research the object
        search_object();
        break;
      case OBJECT_FAR:
      case OBJECT_NEAR:
        speedval = calib_max_speed * max(SLOW_SPEED, (distance_object)/(max_zichtbare_afstand));
        motor_drive(speedval, speedval);
        delay(1500);
        motor_drive(0, 0);
        break;
      case OBJECT_COLLIDE:
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
      delay(1500);
      motor_drive(0,0);
    }
  }
}

void push_object(){
  // we now are or in front of object, or on edge search field. In first case, push it out.
  int pos_obj = measure_distance(distance_object);
  if (pos_obj == OBJECT_COLLIDE) {
    //we drive forward slowly for 7 sec?
    motor_drive(SLOW_SPEED, SLOW_SPEED);
    delay(7000);
    motor_drive(calib_max_speed,-calib_max_speed);
    delay(2000);
    finished = true;
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
  if (speed_corr < -1000) {
    //line at sensor 0 move line slowly towards 2 (to left), by a right turn, so reducing speed right
    right_speed = 0; 
    left_speed = calib_max_speed/2;
    
  } else if (speed_corr < 0){
    //line at sensor 0 to 2, move line towards 2 (to left), by reducing speed right
    right_speed = calib_max_speed + speed_corr;
    left_speed = calib_max_speed;
  } else if (speed_corr > 1000) {
    //line at sensor 4 move line slowly towards 3 (to right), by a left turn, so reducing speed left
    right_speed = calib_max_speed/2;
    left_speed = 0;
  } else {
    //line at sensor 2 to 4, move line towards 2, by reducing speed left
    right_speed = calib_max_speed;
    left_speed = calib_max_speed - speed_corr;
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
