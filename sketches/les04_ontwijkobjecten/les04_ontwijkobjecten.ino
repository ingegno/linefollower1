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

//andere variabelen
int munt=0;

void setup() {
  distsens.attach(trigPin, echoPin);
  distsens.setMinMax(MIN_AFSTAND, MAX_AFSTAND);
  servo1.attach(servoLpin);
  servo2.attach(servoRpin);
  set_servos(servL, servR); //startsnelheid
  dotime();                 //tijd bijhouden
  randomSeed(analogRead(0));
}

void loop() {
  dotime();
  // gebruik hier een van de afstandsmeting methoden
  //afstand = distsens.distSimple();
  afstand = distsens.distTimeout();
  //afstand = distsens.distNoblock(); //NEE. We hebben een delay!
  if (afstand == 0) {
    //niets te zien, rij voorruit
    vooruit();
    munt = random(2);
  } else if (afstand < 18) {
    //iets voor onze neus, hard draaien
    if (munt == 0) { linkshard();
    } else {         rechtshard();
    }
    //minstens 2 seconde
    delay(1500);
  } else {
    //vertraag tot aan 10cm
    set_servos(afstand/MAX_AFSTAND*100, afstand/MAX_AFSTAND*100);
  }
}

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

/* handige korte functies voor typische beweging
 */
 void vooruit()    {set_servos( 100,  100); }
 void achteruit()  {set_servos(-100, -100); }
 void links()      {set_servos(   0,  100); }
 void rechts()     {set_servos( 100,    0); }
 void stopauto()   {set_servos(   0,    0); }
 void linkshard()  {set_servos(-100,  100); }
 void rechtshard() {set_servos( 100, -100); }
 

