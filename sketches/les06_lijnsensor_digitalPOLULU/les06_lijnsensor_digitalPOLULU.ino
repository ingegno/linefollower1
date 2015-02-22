/******************************/
/* Lijnsensor uittesten
/* Sensor kan analoog zijn of digitaal
/* Deze sketch bevat speciale code voor de digitale POLULU lijnsensor
/******************************/

// alles rond lijnsensor
// pas volgende aan voor aantal sensoren die je hebt. 
// Hier voor twee sensoren, een links en een rechts
#define ANALOGSENS true
#define POLULU false
#define lijnLpin  2  // digital pin normaal, analog A2 als ANALOGSENS==true
#define lijnRpin  3  // digital pin normaal, analog A3 als ANALOGSENS==true
#define nrlijnpin 2
int lijnpins[]  = {lijnLpin, lijnRpin};
int lijnsens[] = {0, 0};  // waarde per sensor

void setup() {
  Serial.begin(9600); // dit start serial monitor voor de test
  //pins lijnsensor hebben geen setup nodig!
}

void loop() {
  for (int i=0;i<nrlijnpin;i++){
    if (ANALOGSENS) {
      //lees analoog de lijn sensoren
      // Calibreer grenswaarde van 700 !
      int val = analogRead(lijnpins[i]);
      Serial.print(" Reading analog ... ");Serial.println(val);
      if ( val < 700) {
        //wit
        lijnsens[i] = 1;
      } else {
        //wit
        lijnsens[i] = 0;
      };
    } else {
      //digitaal zwart of niets
      if (POLULU) {
        lijnsens[i] = readQLDigital(lijnpins[i]);
      } else {
        lijnsens[i] = digitalRead(lijnpins[i]);
      }
    }
  }
  delay(1000);
  for (int i=0;i<nrlijnpin;i++){
    Serial.print("Gemeten op pin "); 
    Serial.print(lijnpins[i]); Serial.print(": ");
    if (lijnsens[i] == 0) {
      Serial.println(" ZWART ");
    } else {
      Serial.println(" WIT ");
    }
  }
}

int readQLDigital(int pin){
  //Returns value from the QRE1113 Digital Polulu
  //Lower numbers mean more refleacive
  //More than 3000 means nothing was reflected.
  pinMode( pin, OUTPUT );
  digitalWrite( pin, HIGH );  
  delayMicroseconds(10);
  pinMode( pin, INPUT );

  long time = micros();

  //time how long the input is HIGH, but quit after 3ms as nothing happens after that
  while (digitalRead(pin) == HIGH && micros() - time < 3000); 
  int diff = micros() - time;

  if (diff < 1000) {
    return 1;
  } else {
    return 0;
  }
  //return diff;
}

