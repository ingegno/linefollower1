
/******************************/
/* Echo device om afstand te meten
/******************************/

#define trigPin 12
#define echoPin 13
//snelheid van geluid in lucht in cm/micros
#define SPEED_SOUND 0.034
//maximum afstand in cm die we willen zien
#define MAX_AFSTAND 50.
//minimum afstand die sensor kan zien in cm
#define MIN_AFSTAND 3.5
//berekenen hoe lang geluid nodig heeft in microseconds
unsigned long timeout_echo = (2* MAX_AFSTAND+1) / SPEED_SOUND;
float afstand = 0;
float afst;

bool gebruik_simple = true;

void setup() {
  // zet seriele monitor via usb op om te testen (pin 0 en 1 vrij laten!)
  Serial.begin (9600);
  // aansluitingen sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  //simpele afstandsmeting of met timeout (false)?
  gebruik_simple = true;
}

void loop() {
  if (gebruik_simple) {
    afstand = meas_afst_simpel();
  } else {
    afstand = meas_afst_timeout(MAX_AFSTAND);
  }
  if (afstand > 0) {
    Serial.print("Blik op "); Serial.print(afstand); Serial.println(" cm"); 
  } else {
    Serial.print("Niets gezien ... "); Serial.print(afstand); 
    if (afstand == 0) {Serial.print(" Echt gemeten: ");Serial.println(afst);}
  }
  delay(1000);
}

float meas_afst_simpel(){
  int duration;
  digitalWrite(trigPin, HIGH); // klaarmaken voor puls
  delayMicroseconds(1000);     // 1 ms wachten
  digitalWrite(trigPin, LOW);  // puls afgevuurd !!
  duration = pulseIn(echoPin, HIGH); //wacht op echo in millisec
  //antwoord in millisec, met snelheid geluid berekenen we afstand 
  afst =  (duration/2.) / (1000*SPEED_SOUND);
  
  if (afst < MIN_AFSTAND || afst > MAX_AFSTAND){
      return 0.;
  }
  return afst;
}
