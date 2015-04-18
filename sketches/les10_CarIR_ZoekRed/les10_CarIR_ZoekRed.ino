/******************************/
/* Echo device om afstand te meten
/*  Geavanceerd: zonder de code te blokkeren, we kunnen dus
/*  verder rijden en draaien terwijl we afstand meten
/*  Hier rijden we en ontwijken we alle objecten.
/******************************/

#include <Servo.h>
#include "DistSens.h"
#include "IRremote.h"
 
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

// infrarood
#define MET_IR true
#define IRpin 6
IRrecv irrecv(IRpin);

decode_results results;

int IR_lezing;
#define IR_UNKNOWN 100
#define IR_CHMIN   1
#define IR_CH      2
#define IR_CHPLUS  3
#define IR_PREV    4
#define IR_NEXT    5
#define IR_PLAYPAUSE 6
#define IR_VOLMIN  7
#define IR_VOLPLUS 8
#define IR_EQ      9
#define IR_0      10
#define IR_100PLUS   11
#define IR_200PLUS   12
#define IR_1         21
#define IR_2         22 
#define IR_3         23
#define IR_4         24
#define IR_5         25
#define IR_6         26
#define IR_7         27
#define IR_8         28
#define IR_9         29
const int IR_VOORUIT=0,IR_LINKS=1,IR_RECHTS=2,IR_ACHTERUIT=3,IR_STOP=4;
const int IR_VOORLIN=5,IR_VOORREC=6,IR_ACHTERLIN=7,IR_ACHTERREC=8;
const int IR_GELIJK=0,IR_SNELLER=1,IR_TRAGER=2;
int IR_command = IR_VOORUIT;
int IR_action = IR_GELIJK;
unsigned long timeIR=0UL;
int speedIR = 60;

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
#define ANALOGSENS false
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
#define ROBOT_LIJNVOLG    0
#define ROBOT_REDDER      1
#define ROBOT_ONTWIJK     2
#define ROBOT_IR          3
#define ROBOT_LIJNVOLGRED 4
int lijnstrategie;
#define LS_WAGGEL  0
#define LS_TRAAG   1
#define LS_SNEL    2
//voor lijnvolgred, pas red mogelijk na tijd in ms
unsigned long time_red_na = 1000UL;  //10000UL;
//bijhouden wanneer laatst zwarte lijn gezien
unsigned long time_blackline = timemillibegin;

//andere variabelen
int munt=0;
bool test=false;

void setup() {
  if (MET_IR)  irrecv.enableIRIn();  // Start de IR ontvanger
  
  distsens.attach(trigPin, echoPin);
  distsens.setMinMax(MIN_AFSTAND, MAX_AFSTAND);
  servo1.attach(servoLpin);
  servo2.attach(servoRpin);
  set_servos(servL, servR); //startsnelheid
  //pins lijnsensor hebben geen setup nodig!
  dotime();                 //tijd bijhouden
  randomSeed(analogRead(0));
  
  //we beginnen in modus lijnvolg
  statusrobot = ROBOT_LIJNVOLGRED; //ROBOT_IR;  ROBOT_REDDER; 
  //lijn volgen door traag te gaan
  lijnstrategie = LS_TRAAG;
  //geen afstand meten in lijnvolg
  if (statusrobot == ROBOT_REDDER || statusrobot == ROBOT_ONTWIJK) 
    distsens.start();
  else
    distsens.stop();
  if (test) {
    Serial.begin(9600);   // Seriele monitor gebruiken voor deze test
  }
}

void loop() {
  dotime();
  if (MET_IR) {
    //lees IR signaal en bepaal statusrobot en wijzig waardes
    verwerkIR();
  }
  if (statusrobot == ROBOT_ONTWIJK) {
    // gebruik hier een van de afstandsmeting methoden
    //afstand = distsens.distSimple();
    //afstand = distsens.distTimeout();
    afstand = distsens.distNoblock();
    if (afstand == 0) {
      //niets te zien, rij voorruit
      vooruit();
      munt = random(2);
    } else if (afstand < 18) {
      //iets voor onze neus, hard draaien
      if (munt == 0) { linkshard(50);
      } else {         rechtshard(50);
      }
    } else {
      //vertraag tot aan 10cm
      set_servos(afstand/MAX_AFSTAND*100, afstand/MAX_AFSTAND*100);
    }
  }
  
  if (statusrobot == ROBOT_LIJNVOLG || statusrobot == ROBOT_LIJNVOLGRED) {
    //lijn op grond lezen
    leeslijn();
    if (lijnstrategie == LS_TRAAG) {
      traaglijnvolgen();
    } else {
      //nog schrijven!!
      stopauto();
    }
    if (statusrobot == ROBOT_LIJNVOLGRED) {
      if (timemilli > time_red_na) {
        // controleer dat we lijn zien elke 2 sec
        if (timemilli - time_blackline > 2000UL) {
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
  }
  
  if (statusrobot == ROBOT_REDDER) {
    // gebruik hier een van de afstandsmeting methoden
    //afstand = distsens.distSimple();
    //afstand = distsens.distTimeout();
    //stopauto();
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
  
  if (statusrobot == ROBOT_IR) {
    //beweeg commando uitvoeren
    switch(IR_command) {
      case IR_VOORUIT:
        vooruit(speedIR);
        break;
      case IR_ACHTERUIT:
        achteruit(speedIR);
        break;
      case IR_LINKS:
        linkshard(speedIR);
        break;
      case IR_RECHTS:
        rechtshard(speedIR);
        break;
      case IR_STOP:
        stopauto();
        break;
      case IR_VOORLIN:
        links(speedIR);
        break;
      case IR_VOORREC:
        rechts(speedIR);
        break;
      case IR_ACHTERLIN:
        linksmin(speedIR);
        break;
      case IR_ACHTERREC:
        rechtsmin(speedIR);
        break;
    }
  }
  // IR acties uitvoeren indien ontvangen
  if (MET_IR)
  {
    //opdracht uitvoeren
    switch(IR_action) {
      case IR_SNELLER:
        speedIR += 10;
        if (speedIR > 100) speedIR=100;
        IR_action = IR_GELIJK;
        break;
      case IR_TRAGER:
        speedIR -= 10;
        if (speedIR < 0) speedIR=0;
        IR_action = IR_GELIJK;
        break;
    }
  }
}

void verwerkIR()
{
  //verwerken van IR signaal
  if (timemilli-timeIR > 500) {
    timeIR=timemilli;
    if (irrecv.decode(&results)) {
      //een IR signaal ontvangen, we verwerken het
      IR_lezing = vertaalIR(results.value);
      //wijzig IR_command als iets gelezen
      switch(IR_lezing) {
        case IR_2:  
          IR_command = IR_VOORUIT; 
          break;
        case IR_1:  
          IR_command = IR_VOORLIN; 
          break;
        case IR_3:  
          IR_command = IR_VOORREC; 
          break;
        case IR_4:  
          IR_command = IR_LINKS; 
          break;
        case IR_6:  
          IR_command = IR_RECHTS; 
          break;
        case IR_8:  
          IR_command = IR_ACHTERUIT; 
          break;
        case IR_7:  
          IR_command = IR_ACHTERLIN; 
          break;
        case IR_9:  
          IR_command = IR_ACHTERREC; 
          break;
        case IR_5:  
          IR_command = IR_STOP; 
          break;
        case IR_VOLMIN:  
          IR_action = IR_TRAGER; 
          break;
        case IR_VOLPLUS:  
          IR_action = IR_SNELLER; 
          break;
        case IR_CHMIN:  
          statusrobot = ROBOT_IR; 
          distsens.stop();
          stopauto();
          IR_command = IR_STOP;
          break;
        case IR_CH:  
          statusrobot = ROBOT_LIJNVOLG;
          distsens.stop();
          stopauto();
          break;
        case IR_CHPLUS:  
          statusrobot = ROBOT_ONTWIJK;
          distsens.start();
          stopauto();
          break;
        case IR_PREV:  
          statusrobot = ROBOT_REDDER;
          distsens.start();
          stopauto();
          break;
        default:
          //niets
          statusrobot = statusrobot;
      }
      if (test) {
        Serial.print("Ontvangen waarde=");
        Serial.print(results.value, HEX);
        Serial.print("  - Vertaling: ");
        Serial.println(IR_lezing);
      }
      irrecv.resume(); //klaarmaken om volgende IR waarde te lezen.
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
 
 
/* Een hex waarde veranderen in tekst
*/
int vertaalIR(unsigned long waarde) 
{
  //voeg hier de waarden toe van je afstandsbediening
  //onderstaande voor Carmp3 afstandsbediening Arduino
  switch(waarde) {
    case 0xFFA25D:  
      return IR_CHMIN; 
      break;
    case 0xFF629D:  
      return IR_CH; 
      break;
    case 0xFFE21D:  
      return IR_CHPLUS;
      break;
    case 0xFF22DD:  
      return IR_PREV;
      break;
    case 0xFF02FD:  
      return IR_NEXT;
      break;
    case 0xFFC23D:  
      return IR_PLAYPAUSE;
      break;
    case 0xFFE01F:  
      return IR_VOLMIN;
      break;
    case 0xFFA857:  
      return IR_VOLPLUS;
      break;
    case 0xFF906F:  
      return IR_EQ;
      break;
    case 0xFF6897:  
      return IR_0; 
      break;
    case 0xFF9867:  
      return IR_100PLUS; 
      break;
    case 0xFFB04F:  
      return IR_200PLUS; 
      break;
    case 0xFF30CF:  
      return IR_1; 
      break;
    case 0xFF18E7:  
      return IR_2; 
      break;
    case 0xFF7A85:  
      return IR_3; 
      break;
    case 0xFF10EF:  
      return IR_4; 
      break;
    case 0xFF38C7:  
      return IR_5; 
      break;
    case 0xFF5AA5:  
      return IR_6; 
      break;
    case 0xFF42BD:  
      return IR_7; 
      break;
    case 0xFF4AB5:  
      return IR_8; 
      break;
    case 0xFF52AD:  
      return IR_9; 
      break;
    default: 
      return IR_UNKNOWN;
    }
} //END translateIR


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

