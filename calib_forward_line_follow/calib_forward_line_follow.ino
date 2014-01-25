/*
Line following robot
*/

// typ hier de aansluitingen van de motordriver, voor de twee motors
#define motorlinksPWM 3
#define motorlinksDir 2
#define motorrechtsPWM 5
#define motorrechtsDir 4

#define Kp 1
#define Kd 0
#define Ki 0

bool test=false;

long sensors_average;
int sensors_sum;
int pos;
int baseline;
int topline;
long sensors_average_bl;
int sensors_sum_bl;
// how much difference betwen biggest and lowest to say we see a line
int accuracy=100;
bool lineseen;

int mid_point = 2000;
int max_err = max(4000-mid_point, mid_point-0);
int min_err = 500;

long proportional;
long integral;
long derivative;
long last_proportional;
long error_value;

int speed_corr;
int right_speed;
//play with speed interval !!!
//depends on battery also !!
int max_speed = 100;
int min_speed = 0;
int left_speed;

// Array used to store 5 readings for 5 sensors.
long sensors[] = {0, 0, 0, 0, 0};

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
  lineseen = sensors_read();
  if (lineseen){
    //Calculates how much away from the mid point of the line
    pid_calc();
    calc_turn();
    //Computes the error to be corrected
    motor_drive(right_speed, left_speed); //Sends PWM signals to the motors
  } else {
    if (test) {
      Serial.println("No line seen !!!");
      motor_drive(0, 0);
    }
    // Here we should do a test if we did not miss the line, 
    // and if not search for the can.  
  }
  // Optional
  delay(1);
}

bool sensors_read(){
  /*
  Method to read the sensors. Return true if a line is seen, and does
  necessary computations to know where the line is.
  */
  sensors_average = 0;
  sensors_sum = 0;
  sensors_average_bl = 0;
  sensors_sum_bl = 0;
  baseline = 1024;
  topline = 0;
  for (int i = 0; i < 5; i++){
    sensors[i] = analogRead(i);
    if (baseline > sensors[i]){
      baseline = sensors[i];
    }
    if (topline < sensors[i]){
      topline = sensors[i];
    }
  }
  for (int i = 0; i<5; i++){ 
    //Calculating the weighted mean
    sensors_average += sensors[i] * i * 1000;
    sensors_sum += int(sensors[i]); 
    sensors_average_bl += (sensors[i]-baseline) * i * 1000;
    sensors_sum_bl += int(sensors[i]-baseline);
  }
  if (topline-baseline < accuracy){
    // no line seen
    return false;
  } else {
    return true;
  }
}


void pid_calc(){
  // position based on baseline
  pos = int(sensors_average_bl / sensors_sum_bl);
  proportional = pos - mid_point;
  integral = integral + proportional;
  derivative = proportional - last_proportional;
  last_proportional = proportional;
  error_value = int(proportional * Kp + integral * Ki + derivative * Kd);
  if (test){
    Serial.print("position - error: ");
    Serial.print(pos);Serial.print(" ");
    Serial.print(proportional);Serial.print(" ");
    Serial.print(error_value);Serial.print(" ");
  }
}


void calc_turn(){
  
  //Restricting the error value between -254 and +254.
  if (abs(error_value) > max_err){
    // not possible, some error, don't correct
    error_value = 0;
  }
  if (abs(error_value) < min_err){
    // don't correct yet
    error_value = 0;
  }
  //convert to correct number
  speed_corr = float(error_value)/float(max_err) * (max_speed-min_speed);
  if (test){
    Serial.print(" Speed corr: "); Serial.print(speed_corr);Serial.print(" ");
  }
  if (speed_corr < 0){
    //line at sensor 0 to 2, move towards 2 (to left), by reducing speed left
    right_speed = max_speed + speed_corr;
    left_speed = max_speed;// + speed_corr;
  } else {
    //line at sensor 2 to 4, move towards 2, by reducing speed right
    right_speed = max_speed; // - speed_corr;
    left_speed = max_speed - speed_corr;
  }
}

void motor_drive(int right_speed, int left_speed){
  if (test){
    Serial.print("driving right and left: ");
    Serial.print(right_speed);Serial.print(" ");
    Serial.println(left_speed);
  }
  // Drive motors according to the calculated values
  // Normally 255 - speed as we have Dir LOW, but h
  digitalWrite(motorlinksDir, HIGH); //vooruit
  digitalWrite(motorrechtsDir, HIGH); //vooruit
  analogWrite(motorrechtsPWM, 255-right_speed);
  analogWrite(motorlinksPWM, 255-left_speed);
}

