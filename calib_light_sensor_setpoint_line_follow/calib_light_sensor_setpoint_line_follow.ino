
#define Kp 1
#define Kd 0
#define Ki 0

#define motorlinksPWM 3
#define motorlinksDir 2
#define motorrechtsPWM 5
#define motorrechtsDir 4

long sensors_average;
int sensors_sum;
float pos;
int baseline;
long sensors_average_bl;
int sensors_sum_bl;

// Array used to store 5 readings for 5 sensors
long sensors[] = {0, 0, 0, 0, 0};

void setup(){ 
  Serial.begin(9600);
  // analog pins don't need setup!
}

void loop(){ 
  sensors_average = 0;
  sensors_sum = 0;
  sensors_average_bl = 0;
  sensors_sum_bl = 0;
  baseline = 1024;
  Serial.println("Place me on color and note what values I measure between 0 and 1023!");
  for (int i = 0; i < 5; i++){
    sensors[i] = analogRead(i);
    if (baseline > sensors[i]){
      baseline = sensors[i];
    }
    Serial.print("Measured at "); 
    Serial.print(i); Serial.print(":");
    Serial.println(sensors[i]);  
  }
  for (int i = 0; i<5; i++){ 
    sensors_average += sensors[i] * i * 1000;
    //Calculating the weighted mean
    sensors_sum += int(sensors[i]); 
    sensors_average_bl += (sensors[i]-baseline) * i * 1000;
    //Calculating the weighted mean
    sensors_sum_bl += int(sensors[i]-baseline);
  }
  //Calculating sum of sensor readings
  pos = int(sensors_average / sensors_sum)/1000.0;
  Serial.print("Normal sensor pos line:");
  Serial.println(pos);
  Serial.print("baseline sensor pos line:");
  Serial.println(int(sensors_average_bl / sensors_sum_bl)/1000.);
  Serial.println("If line was centered, then pos line is your calibration mid_point!");
  delay(5000);
}

