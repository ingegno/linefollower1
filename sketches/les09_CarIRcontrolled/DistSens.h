 /*
Bibliotheek om met afstandsensoren te werken
Library to work with distance sensors


  A distance sensor is activated by creating an instance of the DistSens class 
  and passing the desired trigger and echo pin to the attach() method.
  
  You can now request a distance measurement in several ways.
  The methods are: 
    attach(pintrig, pinecho) - what pins to listen on
    stop()                   - deactivate, all distance measurements will return 0.
    start()                  - activate again after a deactivate
    setMinMax(min, max)      - set minimum and maximum distance to measure. Default: 3.5cm and 50cm
    setResolution(res)       - set lowest interval to allow remeasure of distance in microsec
                               default = 250000UL = 250 millisec = 1/4 sec
                               A measurement before this time has past will return the old distance
    distSimple()             - distance measurement that will block the sketch for an unknown amount
                               of time
    distTimeout()            - distance measurement that will block the sketch maximally for the
                               time needed for sound to go to maximum distance set by setMinMax + 1ms
                               This time is 2*max/(speed of sound) + 1
    distNoblock()            - distance measurement that will return in less than 1ms a value.
                               the last measurement is returned as long as a new measurement was not
                               performed. This method will only return correct results if called in a
                               loop() function that uses no delay and calls this function every time. 
                               Use this function if you have several sensors to act upon, and don't 
                               want the distance sensor blocking updates needed for the other sensors
*/

#ifndef DISTSENS_H
#define DISTSENS_H

#include <Arduino.h> 


//snelheid van geluid in lucht in cm/micros
#define SPEED_SOUND 0.034

class DistSens
{
public:
  DistSens(){
    //set defaults
    minafstand          =  50;
    maxafstand          =  3.5;
    //hoe vaak afstand meten in microseconds
    measure_resolution  = 250000UL;  //= 250 ms = 1/4s
    afstand             = 0;
    laatste_afst_meas   = 0UL;
    timeout_echo        = (2* maxafstand+1) / SPEED_SOUND;
    dotrig              = false;
    inecholoc           = false;
  };
  void attach(int pintrig, int pinecho){
    pinMode(pintrig, OUTPUT); trigpin=pintrig;
    pinMode(pinecho, INPUT);  echopin=pinecho;
    start();
  };
  void stop(){active=false;afstand=0.;};
  void start(){active=true;afstand=0.;};
  void setMinMax(float minafst, float maxafst){
    minafstand = minafst;
    maxafstand = maxafst;
    timeout_echo = (2* maxafstand+1) / SPEED_SOUND;
  };
  void setResolution(unsigned long res) {
    measure_resolution = res;
  };

public:
  /*** Meten van afstand op twee pinnen, waarden tussen min/max teruggeven
       Deze functie blokkeert je sketch.
       Geen controle op de duurtijd van de afstandsmeting!
    **/
  float distSimple(){
    if (this->active) {
      timemicro = micros();
      if (timemicro-laatste_afst_meas < measure_resolution){
        //nog geen nieuwe meting doen! Oude afstand teruggeven
        return afstand;
      }
      int duration;
      digitalWrite(trigpin, HIGH); // klaarmaken voor puls
      delayMicroseconds(1000);     // 1 ms wachten
      digitalWrite(trigpin, LOW);  // puls afgevuurd !!
      duration = pulseIn(echopin, HIGH); //wacht op echo in millisec
      //antwoord in millisec, met snelheid geluid berekenen we afstand 
      afstand =  (duration/2.) / (1000*SPEED_SOUND);
      laatste_afst_meas = micros();
      if (afstand < minafstand || afstand > maxafstand){
          afstand = 0.;
      }
      return afstand;
    } else return 0.;
  };
  /*** Meten van afstand op twee pinnen, waarden tussen min/max teruggeven
       Deze functie blokkeert je sketch.
       Duurtijd is maximaal de duur die geluid nodig heeft om maxafst te meten + 1ms
    **/
  float distTimeout(){
    if (this->active) {
      timemicro = micros();
      if (timemicro-laatste_afst_meas < measure_resolution){
        //nog geen nieuwe meting doen! Oude afstand teruggeven
        return afstand;
      }
      digitalWrite(trigpin, HIGH); // klaarmaken voor puls
      delayMicroseconds(1000);     // 1 ms wachten
      digitalWrite(trigpin, LOW);  // puls commando gegeven!!
      unsigned long echodurationstart = micros();
      bool contwait = true;
      inecholoc = false;
      unsigned char echoval;
      
      while (contwait) {
        //tijd voor echo begint te lopen als echoPin HIGH is
        echoval = digitalRead(echopin);
        if (echoval == HIGH) {
          //// puls afgevuurd !! start timing
          echodurationstart = micros();
          inecholoc = true;
          contwait = false;
        } else if (micros() - echodurationstart > 800UL) {
            //iets fout, dit duurt te lang! jump out of loop
          contwait = false;
          inecholoc = false;
        }
      }
      unsigned long echoduration = 0UL;
      while (inecholoc) {
        //we zijn aan het wachten op echo
        echoval = digitalRead(echopin);
        //Als pin LOW wordt is echo aangekomen
        if (echoval == LOW) {
          //echo ontvangen, bereken de duur
          echoduration = micros()-echodurationstart;
          inecholoc = false;
        } else if (micros()-echodurationstart > timeout_echo) {
          //lang genoeg gewacht! Stop.
          echoduration = 0UL;
          inecholoc = false;
        }
      }
      laatste_afst_meas = micros();
      afstand = (echoduration/2.) * SPEED_SOUND;
      if (afstand < minafstand || afstand > maxafstand){
          afstand = 0.;
      }
      return afstand;
    } else return 0.;
  };
  
  /*** Meten van afstand op twee pinnen, waarden tussen min/max teruggeven
       Deze functie blokkeert je sketch niet, je hebt een antwoord binnen 1ms.
       Zolang geen nieuwe afstand berekend is, krijg je de waarde van de oude afstand
       terug.
    **/
  float distNoblock(){
    if (this->active) {
      timemicro=micros();
      bool updated_dist = false;
      // meet afstand eens elke xx seconds
      // 5 mogelijkheden:
      //  1. nog niet nodig een nieuwe meting te doen
      //  2. maak klaar om meting te doen
      //  3. stuur commando om puls uit te sturen
      //  4. controleer of puls uitgestuurd is
      //  5. puls gestuurd, wacht op echo
      if (timemicro-laatste_afst_meas < measure_resolution){
        // nog niet lang genoeg sinds vorige meting, niets doen
        dotrig = true;
        inecholoc = false;
      } else if (!inecholoc && dotrig == true){
        dotrig = false;
        // Klaarmaken om puls uit te sturen
        digitalWrite(trigpin, HIGH);
      } else if (!inecholoc && timemicro-laatste_afst_meas > measure_resolution + 100UL){
        //lang genoeg gewacht, bepaal nieuwe afstand, emit sound (8x40kHz pulses):
        echodurationstart = micros();
        digitalWrite(trigpin, LOW);
        //catch echo, determine afstand
        bool contwait = true;
        while (contwait) {
          //timing starts running when pin reads HIGH
          if (digitalRead(echopin) == HIGH) {
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
        //if we read LOW, timing ends
        if (digitalRead(echopin) == LOW) {
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
        afstand = (echoduration/2.) * SPEED_SOUND;
        if (afstand < minafstand || afstand > maxafstand){
          afstand = 0.;
        }
        //set time of this dist meas, so it does not happen again too fast
        laatste_afst_meas = micros();
      }
      return afstand;
    } else return 0.;
  }
private:
  int           trigpin;
  int           echopin;
  bool          active;
  float         minafstand;
  float         maxafstand;
  unsigned long timeout_echo;
  unsigned long measure_resolution;
  unsigned long timemicro;
  bool          dotrig;
  bool          inecholoc;
  unsigned long echodurationstart;
  unsigned long echoduration;
  
  float         afstand;
  unsigned long laatste_afst_meas;
};



#endif

