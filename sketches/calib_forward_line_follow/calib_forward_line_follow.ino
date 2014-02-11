/************************************************************/
/*          lijn volgen basisversie                         */
/************************************************************/

// typ hier de aansluitingen van de motordriver, voor de twee motors
#define motorlinksPWM 3
#define motorlinksDir 2
#define motorrechtsPWM 5
#define motorrechtsDir 4

bool test=false;

//geen PID control http://en.wikipedia.org/wiki/PID_controller
// enkel proportioneel de afwijking van middellijn in rekening brengen


#define SCHAAL_FOUT 1000

int black[2]  = {600,1000}; // hiertussen zouden de waarden voor zwart moeten zijn
//test of je groen, geel, wit kan zien
//waarschijnlijk niet ....
int green[2]  = { 75, 100};
int yellow[2] = { 80, 105};
int white[2]  = { 40, 150};

int corrwhite[5] = {0, 0, 0, 10, 30};
int corrblack[5] = {0, 50, 0, 20, 50};

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
//play with speed interval !!!
//depends on battery also !!
#define MAX_SPEED  180 //max 255!
#define MIN_SPEED  0

int speed_corr;
int right_speed;
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
  //Reads sensor values and computes sensor sum and weighted average
  boolean lineseen = sensors_read();
  if (lineseen){
    calc_turn();
    //Computes the error to be corrected
    motor_drive(right_speed, left_speed); //Sends PWM signals to the motors
  } else {
    if (test) {
      Serial.println("No line seen !!!");
    }
    motor_drive(0, 0);
    // Here we should do a test if we did not miss the line, 
    // and if not search for the can.  
  }
}

boolean sensors_read(){
  /*
  Method to read the sensors. Return true if a line is seen, and does
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

  if (topline-baseline < ACCURACY){
    // no line seen
    return false;
  } else {
    return true;
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
  speed_corr = float(error_value)/float(max_err) * (MAX_SPEED-MIN_SPEED);
  if (test){
    Serial.print(" Speed corr: "); Serial.print(speed_corr);Serial.print(" ");
  }
  if (speed_corr < -1000) {
    //line at sensor 0 move line slowly towards 2 (to left), by a right turn, so reducing speed right
    right_speed = 0; 
    left_speed = MAX_SPEED/2;
    
  } else if (speed_corr < 0){
    //line at sensor 0 to 2, move line towards 2 (to left), by reducing speed right
    right_speed = MAX_SPEED + speed_corr;
    left_speed = MAX_SPEED;
  } else if (speed_corr > 1000) {
    //line at sensor 4 move line slowly towards 3 (to right), by a left turn, so reducing speed left
    right_speed = MAX_SPEED/2;
    left_speed = 0;
  } else {
    //line at sensor 2 to 4, move line towards 2, by reducing speed left
    right_speed = MAX_SPEED;
    left_speed = MAX_SPEED - speed_corr;
  }
}

void motor_drive(int right_speed, int left_speed){
  // Drive motors according to the calculated values
  // Normally 255 - speed as we have Dir LOW, but h
  if (right_speed>=0){
    digitalWrite(motorrechtsDir, HIGH); //vooruit
    analogWrite(motorrechtsPWM, 255-right_speed);
  } else {
    digitalWrite(motorrechtsDir, LOW); //achteruituit
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

