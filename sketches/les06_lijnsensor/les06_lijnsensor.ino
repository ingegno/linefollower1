/******************************/
/* Lijnsensor uittesten
/* Sensor kan analoog zijn of digitaal
/******************************/

// alles rond lijnsensor
// pas volgende aan voor aantal sensoren die je hebt. 
// Hier voor twee sensoren, een links en een rechts
#define ANALOGSENS false
#define lijnLpin  2
#define lijnRpin  3
#define nrlijnpin 2
int lijnpins[]  = {lijnLpin, lijnRpin};
int lijnsens[] = {0, 0};  // waarde per sensor



void setup() {
  Serial.begin(9600); // dit start serial monitor voor de test
  //pins lijnsensor hebben geen setup nodig!
}

void loop() {
  //lees analoog de lijn sensoren
  for (int i=0;i<nrlijnpin;i++){
    if (ANALOGSENS) {
      if (analogRead(lijnpins[i]) < 500) {
        //wit
        lijnsens[i] = 1;
      } else {
        //wit
        lijnsens[i] = 0;
      };
    } else {
      //digitaal zwart of niets
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

