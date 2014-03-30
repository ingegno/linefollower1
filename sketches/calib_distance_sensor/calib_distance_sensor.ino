
/************************************************************/
/*          Echo device to measure distance to object       */
/************************************************************/

// connections Arduino
#define trigPin 12
#define echoPin 13

#define motorrechtsPWM 3
#define motorrechtsDir 2
#define motorlinksPWM  5
#define motorlinksDir  4

bool test = true;   //zet op 0 om niet te debuggen,  1 in te debuggen!

void setup() {
  // zet seriele monitor via usb op om te testen (pin 0 en 1 vrij laten!)
  if (test){
    Serial.begin (9600);
  }
  // aansluitingen sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // type hier de aansluitingen van de motordriver, voor de twee motors
  pinMode(motorlinksPWM, OUTPUT);
  pinMode(motorlinksDir, OUTPUT);
  pinMode(motorrechtsPWM, OUTPUT);
  pinMode(motorrechtsDir, OUTPUT);
}

// afstand te kijken, meer dan 80 cm zal hij niet zien!
int max_zichtbare_afstand = 60; //afstand in cm die we onderzoeken, verder zien we niet!
// wanneer stoppen?
int stop_afstand = 10; // stopt aan 10 cm
// max snelheid
int snelheid_max = 170;

void loop() {
  // bepalen van afstand tot het blikje
  int duration, distance;
  // 1 milliseconde kijken wat afstand van blik is
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  // berekenen afstand in cm (test dit en pas breuk aan als nodig!
  duration = pulseIn(echoPin, HIGH);
  //Merk op: Out of range == 0 cm!
  distance = (duration/2) / 29.1;
  if (distance <= 0 || distance >= max_zichtbare_afstand){
    //out of range
    vooruit(snelheid_max); //vooruit rijden naar het blikje
    if (test) {
      Serial.println("Niets gezien");
      delay(500);
    }
  }
  else if (distance > 0 && distance <= stop_afstand){ 
    //enkel rijden wanneer blikje meer dan stop_afstand ver is
    stoppen(); //stoppen met rijden
    if (test) {
      Serial.print("Blik op "); Serial.print(distance); Serial.println(" cm"); 
      delay(500);
    }
  }
  else {
    //blik gezien, we rijden nog verder ernaar toe, wel vertragen
    vooruit(snelheid_max * (distance-stop_afstand)/(max_zichtbare_afstand-stop_afstand));
    if (test) {
      Serial.print("Blik op "); Serial.print(distance); Serial.println(" cm"); 
      delay(500);
    }
  }
  //we doen voor 50 milliseconden verder voor we opnieuw testen
  delay(50);
}


void motor_drive(int right_speed, int left_speed){
  // Drive motors according to the calculated values
  // Normally 255 - speed as we have Dir LOW, but h
  if (right_speed>=0){
    digitalWrite(motorrechtsDir, HIGH); //vooruit
    analogWrite(motorrechtsPWM, 255-right_speed);
  } else {
    digitalWrite(motorrechtsDir, LOW); //vooruit
    analogWrite(motorrechtsPWM, -right_speed);
  }
  if (left_speed>=0){
    digitalWrite(motorlinksDir, HIGH); //vooruit
    analogWrite(motorlinksPWM, 255-left_speed);
  } else {
    digitalWrite(motorlinksDir, LOW); //vooruit
    analogWrite(motorlinksPWM, -right_speed);
  }
}

// vooruit rijden
void vooruit(int snelheid){
  motor_drive(snelheid, snelheid);
}

// stoppen met rijden
void stoppen(){
  motor_drive(0, 0);
}

