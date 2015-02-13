/***************************************/
/*  Gebruik de Arduino om de Servo's te controleren
/**************************************/

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

void setup() 
{ 
  servo1.attach( 9);  // connecteer servo op pin  9
  servo2.attach(10);  // connecteer servo op pin 10 
  
  set_servos(servL, servR);
  
  dotime();
}


void loop() 
{ // we tonen hoe zonder blokkeren via delay we vlot 
  // kunnen rijden
  // dit is nodig als we terwijl we rijden we willen kunnen
  // zoeken naar objecten of een lijn volgen
  
  dotime();
  
  //voorgedefinieerde weg afleggen in seconden
  if      (timesec < 5) vooruit();
  else if (timesec < 10) achteruit();
  else if (timesec < 15) links();
  else if (timesec < 20) rechts();
  else if (timesec < 25) stopauto();
  else {
    //herbegin
    resettime();
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
    servo1.write(90- 90 * links/100.);
    servL = links;
  }
  if (rechts != servR) {
    //opgelet, rechts staat tegengesteld, dus maal -1 !!
    servo2.write(90 + 90 * rechts/100.);
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
 
