/*******************************************/
/* Robocup Junior basis code
/*  Volg een parcours
/*  Na bepaalde tijd: zoek geel veld, en zoek object om te redden
/*  
/*******************************************/

#include <Servo.h>
#include "DistSens.h"

/********************************************/
/* Gegevens rond de bouw van je auto. 
/********************************************/
// pinnen voor de servo motor
#define servoLpin 9
#define servoRpin 10
// correctiewaarde indien bij rechtdoor auto niet rechtdoor rijd
// Voor de wedstrijd: laat de auto licht naar links afwijken gezien parcours
// voornamlijk naar rechts draaien is.
int correctieL = 0;
int correctieR = 0;

// alles rond afstand:
#define trigPin 12
#define echoPin 13
//maximum afstand in cm die we willen zien (bv bij redden)
#define MAX_AFSTAND 50.
//minimum afstand die sensor kan zien in cm (afhankelijk van waar je sensor staat)
#define MIN_AFSTAND 3.5

//voor lijnvolgred, vanaf wanneer redden mogelijk maken in ms
unsigned long time_red_na = 30000UL;
unsigned long time_vertraag_vanaf = 20000UL;

// alles rond lijnsensor
#define ANALOGSENS false
#define POLULU false
#define lijnLpin  2  // digital pin normaal, analog A2 als ANALOGSENS==true
#define lijnRpin  3  // digital pin normaal, analog A3 als ANALOGSENS==true

//snelheden van 0 (stilstaan) tot 100 (vol vooruit)
int lijn_snelheid = 15;  //niet te vlug of je schiet over de zwarte lijn!
#define lijn_snelheid_traag 10 //vanaf traag
#define lijn_draaihard 20  //als zwart onder sensor, draaihard om te corrigeren

/********************************************/
/* Code
/********************************************/
Servo servo1;
Servo servo2;
DistSens distsens;

// positie servo's. Start: stilstaan 
// waarde in percenten -100 .. 100
int servL = 0;
int servR = 0;

// bijhouden tijd
unsigned long timesec,   timesecbegin  =0UL;
unsigned long timemilli, timemillibegin=0UL;
unsigned long timemicro, timemicrobegin=0UL;

//variabelen om mee te rekenen
float afstand=0;

// alles rond lijnsensor
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
#define ROBOT_LIJNVOLG    0
#define ROBOT_REDDER      1
#define ROBOT_ONTWIJK     2
#define ROBOT_IR          3
#define ROBOT_LIJNVOLGRED 4
//robocup is lijnvolgen en redden:
int statusrobot = ROBOT_LIJNVOLGRED; 

#define LS_WAGGEL  0
#define LS_TRAAG   1
#define LS_SNEL    2
//lijn volgen door traag te gaan
//wijzig de strategie van lijn volgen naar wens!
int lijnstrategie = LS_TRAAG;

//bijhouden wanneer laatst zwarte lijn gezien
unsigned long time_blackline = timemillibegin;

//andere variabelen
int munt=0;
bool test=false;

void setup() {  
  distsens.attach(trigPin, echoPin);
  distsens.setMinMax(MIN_AFSTAND, MAX_AFSTAND);
  servo1.attach(servoLpin);
  servo2.attach(servoRpin);
  set_servos(servL, servR); //startsnelheid
  //pins lijnsensor hebben geen setup nodig!
  dotime();                 //tijd bijhouden
  randomSeed(analogRead(0));
  
  //geen afstand meten bij start
  distsens.stop();
}

void loop() {
  dotime();
  if (timemilli > time_vertraag_vanaf) lijn_snelheid = lijn_snelheid_traag;
  
  if (statusrobot == ROBOT_LIJNVOLGRED) {
    //lijn op grond lezen
    leeslijn();
    if (lijnstrategie == LS_TRAAG) {
      traaglijnvolgen();
    } else {
      //nog schrijven!!
      stopauto();
    }
    if (timemilli > time_red_na) {
      // controleer dat we lijn zien elke 1.5 sec
      if (timemilli - time_blackline > 1500UL) {
        //zoek de zwarte lijn
        bool zwart_gevonden = zoek_zwart();
        if (zwart_gevonden==false) {
          //zwart niet gevonden, schakel over op redden!
          statusrobot = ROBOT_REDDER;
          //start afstandmeting
          distsens.start();
          stopauto();
        }
      }
    }
  }
  
  if (statusrobot == ROBOT_REDDER) {
    // gebruik hier een van de afstandsmeting methoden
    //afstand = distsens.distSimple();
    //afstand = distsens.distTimeout();
    afstand = distsens.distNoblock();
    if (afstand == 0) {
      //niets te zien, draai traag rond
      //we doen dit door elke 100ms van een seconde te draaien, anders niet
      // timemilli % 500 heeft de rest terug bij deling door 500!
      if (timemilli % 350 < 50) {
        rechtshard(100);
      } else {
        stopauto();
      }
    } else {
      //traag vooruit
      if (afstand > 12) {
        //niet te vlug ernaartoe
        vooruit(60);
      } else {
        //dichtbij, duw het!
        vooruit(100);
        //doe dit 6 seconden lang
        delay(6000);
      }
    }
  }
}

void traaglijnvolgen(){
    if (statuslijn == LS_LIJNMIDDEN) {
      vooruit(lijn_snelheid);
    } else if (statuslijn == LS_LIJNLINKS) {
      linkshard(lijn_draaihard);
      //niet overshooten
    } else if (statuslijn == LS_LIJNRECHTS) {
      rechtshard(lijn_draaihard);
      //niet overshooten
    } else if (statuslijn == LS_ZWARTVELD) {
      //oeps. probeer te herstellen 
      rechtshard(lijn_draaihard);
      //niet overshooten
    }
}

void snellijnvolgen(){
  //werkt NIET!! Aanpassen naar eigen idee
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
  else if (lijnsens[0] == ZWART && lijnsens[nrlijnpin-1] == WIT) {
    statuslijn = LS_LIJNLINKS;
    time_blackline = timemilli;    
  } else if (lijnsens[0] == WIT   && lijnsens[nrlijnpin-1] == ZWART) {
    statuslijn = LS_LIJNRECHTS;
    time_blackline = timemilli; 
  } else {
    //beide lezen zwart.
    statuslijn = LS_ZWARTVELD;
    time_blackline = timemilli; 
  }
}

//blokeer alles en zoek zwarte lijn door links te draaien.
//na 2.5 sec moeten we zwart zien
//Geeft true terug als zwart gevonden
bool zoek_zwart(){
  unsigned long start_zoek_zwart = timemilli;
  while ((millis()-timemillibegin) - start_zoek_zwart < 2500UL && statuslijn == LS_LIJNMIDDEN) {
    linkshard(20);
    leeslijn();
  }
  stopauto();
  //einde zoek_zwart. Indien nog LS_LIJNMIDDEN, niet gevonden!
  if (statuslijn == LS_LIJNMIDDEN) {
    return false;
  }
  return true;
}

/* handige korte functies voor typische beweging
 */
 void vooruit()         {set_servos( 100,  100); }
 void vooruit(int v)    {set_servos(   v+correctieL,    v+correctieR); }
 void achteruit()       {set_servos(-100, -100); }
 void achteruit(int v)  {set_servos(-(v+correctieL),  -(v+correctieR)); }
 void links()           {set_servos(   0,  100); }
 void links(int v)      {set_servos(   0,    (v+correctieR)); }
 void linksop()         {set_servos(  50,  100); }
 void linksop(int v)    {set_servos( (v+correctieR)/2, (v+correctieR)); }
 void linksopmin()      {set_servos( -50, -100); }
 void linksopmin(int v) {set_servos(-(v+correctieR)/2,-(v+correctieR)); }
 void linksmin()        {set_servos(   0, -100); }
 void linksmin(int v)   {set_servos(   0,-(v+correctieR)); }
 void rechts()          {set_servos( 100,    0); }
 void rechts(int v)     {set_servos( (v+correctieL),    0); }
 void rechtsop()        {set_servos( 100,    50); }
 void rechtsop(int v)   {set_servos( (v+correctieL),   (v+correctieR)/2); }
 void rechtsopmin()     {set_servos(-100,   -50); }
 void rechtsopmin(int v){set_servos(-(v+correctieL),  -(v+correctieR)/2); }
 void rechtsmin()       {set_servos(-100,     0); }
 void rechtsmin(int v)  {set_servos(-(v+correctieL),    0); }
 void stopauto()        {set_servos(   0,    0); }
 void linkshard()       {set_servos(-100,  100); }
 void linkshard(int v)  {set_servos(-(v+correctieL),   (v+correctieR)); }
 void rechtshard()      {set_servos( 100, -100); }
 void rechtshard(int v) {set_servos( (v+correctieL),  -(v+correctieR)); }
 

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

