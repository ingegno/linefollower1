/******************************/
/* Echo device om afstand te meten
/*  Geavanceerd: zonder de code te blokkeren, we kunnen dus
/*  verder rijden en draaien terwijl we afstand meten
/*  Hier rijden we en ontwijken we alle objecten.
/******************************/

#include <Servo.h>
#include "DistSens.h"
 
Servo servo1;
Servo servo2;
DistSens distsens;

// positie servo's. Start: stilstaan 
// waarde in percenten -100 .. 100
int servL = 0;
int servR = 0;
#define servoLpin 9
#define servoRpin 10
int correctieL = 0;
int correctieR = 5;

// bijhouden tijd
unsigned long timesec,   timesecbegin  =0UL;
unsigned long timemilli, timemillibegin=0UL;
unsigned long timemicro, timemicrobegin=0UL;

// alles rond afstand:
#define trigPin 12
#define echoPin 13
//maximum afstand in cm die we willen zien
#define MAX_AFSTAND 50.
//minimum afstand die sensor kan zien in cm
#define MIN_AFSTAND 3.5
//variabelen om mee te rekenen
float afstand=0;

// alles rond lijnsensor
// pas volgende aan voor aantal sensoren die je hebt. 
// Hier voor twee sensoren, een links en een rechts
#define ANALOGSENS true
#define POLULU false
#define lijnLpin  2  // digital pin normaal, analog A2 als ANALOGSENS==true
#define lijnRpin  3  // digital pin normaal, analog A3 als ANALOGSENS==true
#define nrlijnpin 2
int lijnpins[]  = {lijnLpin, lijnRpin};
#define ZWART 0
#define WIT   1
int lijnsens[] = {ZWART, ZWART};  // waarde per sensor
int statuslijn;
// Wat kunnen we zien met de 2 lijnsensoren
#define LS_LIJNMIDDEN 0
#define LS_LIJNLINKS  1
#define LS_LIJNRECHTS 2
#define LS_ZWARTVELD  3

//algemene zaken autorobot
int statusrobot;
#define ROBOT_LIJNVOLG 0
#define ROBOT_REDDER   1
#define ROBOT_ONTWIJK  2
int lijnstategie;
#define LS_WAGGEL  0
#define LS_TRAAG   1
#define LS_SNEL    2

//andere variabelen
int munt=0;

void setup() {
  distsens.attach(trigPin, echoPin);
  distsens.setMinMax(MIN_AFSTAND, MAX_AFSTAND);
  servo1.attach(servoLpin);
  servo2.attach(servoRpin);
  set_servos(servL, servR); //startsnelheid
  //pins lijnsensor hebben geen setup nodig!
  dotime();                 //tijd bijhouden
  randomSeed(analogRead(0));
  
  //we beginnen in modus lijnvolg
  statusrobot = ROBOT_LIJNVOLG;
  //lijn volgen door traag te gaan
  lijnstategie = LS_TRAAG;
  //geen afstand meten in lijnvolg
  distsens.stop();
  
}

void loop() {
  dotime();
  // gebruik hier een van de afstandsmeting methoden
  //afstand = distsens.distSimple();
  //afstand = distsens.distTimeout();
  afstand = distsens.distNoblock();
  leeslijn();
  
  if (statusrobot == ROBOT_ONTWIJK) {
    if (afstand == 0) {
      //niets te zien, rij voorruit
      vooruit();
      munt = random(2);
    } else if (afstand < 18) {
      //iets voor onze neus, hard draaien
      if (munt == 0) { linkshard();
      } else {         rechtshard();
      }
    } else {
      //vertraag tot aan 10cm
      set_servos(afstand/MAX_AFSTAND*100, afstand/MAX_AFSTAND*100);
    }
  }
  
  if (statusrobot == ROBOT_LIJNVOLG) {
    if (lijnstategie == LS_TRAAG) {
      traaglijnvolgen();
    } else {
      //nog schrijven!!
      stopauto();
    }
  }
}

void leeslijn()
{
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
  verwerklijn();
}

void verwerklijn()
{
  if      (lijnsens[0] == WIT   && lijnsens[nrlijnpin-1] == WIT) 
    statuslijn = LS_LIJNMIDDEN;
  else if (lijnsens[0] == ZWART && lijnsens[nrlijnpin-1] == WIT)
    statuslijn = LS_LIJNLINKS;
  else if (lijnsens[0] == WIT   && lijnsens[nrlijnpin-1] == ZWART)
    statuslijn = LS_LIJNRECHTS;
  else
    //beide lezen zwart.
    statuslijn = LS_ZWARTVELD;
}

void traaglijnvolgen(){
    if (statuslijn == LS_LIJNMIDDEN) {
      vooruit(20);
    } else if (statuslijn == LS_LIJNLINKS) {
      linkshard(10);
      //niet overshooten
    } else if (statuslijn == LS_LIJNRECHTS) {
      rechtshard(10);
      //niet overshooten
    } else if (statuslijn == LS_ZWARTVELD) {
      //oeps. probeer te herstellen 
      rechtshard(10);
      //niet overshooten
    }
}

void snellijnvolgen(){
    if (statuslijn == LS_LIJNMIDDEN) {
      vooruit();
    } else if (statuslijn == LS_LIJNLINKS) {
      //niet overshooten
      achteruit(10);
      delay(500);
      //nu actie
      links();
    } else if (statuslijn == LS_LIJNRECHTS) {
      //niet overshooten
      achteruit(10);
      delay(500);
      //nu actie
      rechts();
    } else if (statuslijn == LS_ZWARTVELD) {
      //oeps. probeer te herstellen 
      //niet overshooten
      achteruit(10);
      delay(500);
      //nu actie
      rechtshard();
    }
}

/* handige korte functies voor typische beweging
 */
 void vooruit()         {set_servos( 100,  100); }
 void vooruit(int v)    {set_servos(   v+correctieL,    v+correctieR); }
 void achteruit()       {set_servos(-100, -100); }
 void achteruit(int v)  {set_servos(  -(v+correctieL),  -(v+correctieR)); }
 void links()           {set_servos(   0,  100); }
 void links(int v)      {set_servos(   0,    (v+correctieR)); }
 void rechts()          {set_servos( 100,    0); }
 void rechts(int v)     {set_servos(   (v+correctieL),    0); }
 void stopauto()        {set_servos(   0,    0); }
 void linkshard()       {set_servos(-100,  100); }
 void linkshard(int v)  {set_servos(  -(v+correctieL),    (v+correctieR)); }
 void rechtshard()      {set_servos( 100, -100); }
 void rechtshard(int v) {set_servos(   (v+correctieL),   -(v+correctieR)); }
 
/* Opslaan van de tijd
 */
void dotime()
{
  timemilli = millis()-timemillibegin;
  timemicro = micros()-timemicrobegin;
  timesec = timemilli/1000;
}
void resettime()
{
    timemillibegin = timemilli;
    timemicrobegin = timemicro;
    timemilli = 0UL; timemicro=0UL; timesec=0UL;
}

/* De servo's snelheden geven in percent van de
   maximale snelheid.
 */ 
void set_servos(int links, int rechts) 
{
  //testen van de input
  if (links > 100) links = 100;
  if (links < -100) links = -100;
  if (rechts > 100) rechts = 100;
  if (rechts < -100) rechts = -100;
  //waarde sturen naar de servo als verschillend van huidige
  if (links != servL) {
    servo1.write(90-links*90/100.);
    servL = links;
  }
  if (rechts != servR) {
    //opgelet, rechts staat tegengesteld, dus maal -1 !!
    servo2.write(90+rechts*90/100.);
    servR = rechts;
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

