/******************************/
/* Lijnsensor uittesten
/* Sensor kan analoog zijn of digitaal
/******************************/

// alles rond lijnsensor
// pas volgende aan voor aantal sensoren die je hebt. 
// Hier voor twee sensoren, een links en een rechts
#define ANALOGSENS  false
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
        //zwart
        lijnsens[i] = 0;
      };
    } else {
      // simpele digitale sensor
      lijnsens[i] = digitalRead(lijnpins[i]);
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

