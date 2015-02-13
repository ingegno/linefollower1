/******************************/
/* Echo device om afstand te meten
/*  Geavanceerd: zonder de code te blokkeren, we kunnen dus
/*  verder rijden en draaien terwijl we afstand meten
/*  Hier rijden we en ontwijken we alle objecten.
/******************************/

#include <Servo.h> 
 
Servo servo1;
Servo servo2;

// positie servo's. Start: stilstaan 
// waarde in percenten -100 .. 100
int servL = 0;
int servR = 0;

// bijhouden tijd
unsigned long timesec,   timesecbegin  =0UL;
unsigned long timemilli, timemillibegin=0UL;
unsigned long timemicro, timemicrobegin=0UL;

// alles rond afstand:
#define trigPin 12
#define echoPin 13
//activeer afstand sensor of niet
boolean GEBRUIK_AFSTAND = true;
//snelheid van geluid in lucht in cm/micros
#define SPEED_SOUND 0.034
//maximum afstand in cm die we willen zien
#define MAX_AFSTAND 50.
//minimum afstand die sensor kan zien in cm
#define MIN_AFSTAND 3.5
//hoe vaak afstand meten in microseconds
#define AFSTAND_MEAS_RESO 250000UL  //= 250 ms = 1/4s
//berekenen hoe lang geluid nodig heeft in microseconds
unsigned long timeout_echo = (2* MAX_AFSTAND+1) / SPEED_SOUND;
//variabelen om mee te rekenen
float afstand=0, oude_afstand=0;
float afst;
unsigned long laatste_afst_meas = 0UL;
bool dotrig = false;
bool inecholoc = false;
bool pulseoutwait = false;
unsigned long echodurationstart,echoduration;
//unsigned char echoval;

//andere variabelen
int munt=0;

void setup() {
  // zet seriele monitor via usb op om te testen (pin 0 en 1 vrij laten!)
  //Serial.begin (9600);
  setup_afstand_sensor();
  setup_servos(); 
  dotime();
  randomSeed(analogRead(0));
}

void setup_afstand_sensor() {
  if (GEBRUIK_AFSTAND) {
    // aansluitingen sensor
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
  }
  afstand=0; laatste_afst_meas=0UL;
}

void setup_servos() {
  servo1.attach( 9);  // connecteer servo op pin  9
  servo2.attach(10);  // connecteer servo op pin 10 
  set_servos(servL, servR);
}
  
void loop() {
  dotime();
  afstand = meas_afst_noblock();
  if (afstand == 0) {
    vooruit();
    munt = random(2);
  } else if (afstand < 10) {
    if (munt == 0) { linkshard();
    } else {         rechtshard();
    }
  } else {
    //vertraag tot aan 10cm
    set_servos(afstand/MAX_AFSTAND*100, afstand/MAX_AFSTAND*100);
  }
  /*
  if (timemilli % 3000 == 0) {
     Serial.print("Afstand gemeten ... "); 
     Serial.println(afstand); 
  } */
}

/****  Functies hieronder  ****/
float meas_afst_noblock(){
  if (!GEBRUIK_AFSTAND) { return 0;}
  bool updated_dist = false;
  // measure afstand once every xx seconds
  // 5 mogelijkheden:
  //  1. nog niet nodig een nieuwe meting te doen
  //  2. maak klaar om meting te doen
  //  3. stuur commando om puls uit te sturen
  //  4. controleer of puls uitgestuurd is
  //  5. puls gestuurd, wacht op echo
  if (timemicro-laatste_afst_meas < AFSTAND_MEAS_RESO){
    // nog niet lang genoeg sinds vorige meting, niets doen
    dotrig = true;
    inecholoc = false;
  } else if (!inecholoc && dotrig == true){
    dotrig = false;
    // read the afstand. Prepare to emit sound
    digitalWrite(trigPin, HIGH);
  } else if (!inecholoc && timemicro-laatste_afst_meas > AFSTAND_MEAS_RESO + 100UL){
    //we waited long enough, determine new afstand, emit sound (8x40kHz pulses):
    echodurationstart = micros();
    digitalWrite(trigPin, LOW);
    //catch echo, determine afstand
    bool contwait = true;
    while (contwait) {
      //timing starts running when pin reads HIGH
      //echoval = digitalRead(echoPin);
      if (digitalRead(echoPin) == HIGH) {
        //start timing
        echodurationstart = micros();
        contwait = false;
        inecholoc = true;
      } else if (micros() - echodurationstart > 800UL) {
        //timeout, jump out of loop
        contwait = false;
        inecholoc = false;
        echoduration = 0UL;
        updated_dist = true;
      }
    }
  } else if (inecholoc) {
    //we are waiting for the echo, we test the echo pin
    //echoval = digitalRead(echoPin);
    //if we read LOW, timing ends
    if (digitalRead(echoPin) == LOW) {
      //echo received, new echoduration value
      echoduration = timemicro-echodurationstart;
      inecholoc = false;
      updated_dist = true;
    } else if (timemicro-echodurationstart > timeout_echo) {
      echoduration = 0UL;
      inecholoc = false;
      updated_dist = true;
    }
  }
  if (updated_dist) {
    oude_afstand = afstand;
    afstand = (echoduration/2.) * SPEED_SOUND;
    if (afstand < MIN_AFSTAND || afstand > MAX_AFSTAND){
      afstand = 0.;
    }
    oude_afstand = afstand;
    //set time of this dist meas, so it does not happen again too fast
    laatste_afst_meas = micros();
  }
  return afstand;
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
 

