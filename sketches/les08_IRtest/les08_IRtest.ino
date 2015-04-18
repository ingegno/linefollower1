/*
 * IR remote test - demonstreer IR codes ontvangen met 
 * IRremote bibliotheek
 * Copyright 2015 Ingegno.be - Public Domain
 * Zie ook https://arduino-info.wikispaces.com/IR-RemoteControl
 */

#include "IRremote.h"

int RECV_PIN = 6;        // pin waarop je luistert naar IR ontvanger.

IRrecv irrecv(RECV_PIN); //Initialiseer bibliotheek

decode_results results;  // Variabele waar je resultaat opslaat

void setup()
{
  Serial.begin(9600);   // Seriele monitor gebruiken voor deze test
  irrecv.enableIRIn();  // Start de IR ontvanger
  Serial.println(results.value, HEX);
}

void loop() {
  // Controleer of we een IR waarde 
  if (irrecv.decode(&results)) {
    Serial.print("Ontvangen waarde=");
    Serial.print(results.value, HEX);
    Serial.print("  - Vertaling: ");
    Serial.println(vertaalIR(results.value) );
    
    irrecv.resume(); // Ontvang volgende waarde value
  } else {
    //nothing
    Serial.println("Niets geziens");
  }
  delay(1000);  //elke seconde luisteren.
}

/* Een hex waarde veranderen in tekst
*/
char* vertaalIR(unsigned long waarde) 
{
  //voeg hier de waarden toe van je afstandsbediening
  //onderstaande voor Carmp3 afstandsbediening Arduino
  switch(waarde) {
    case 0xFFA25D:  
      return "CH-"; 
      break;
    case 0xFF629D:  
      return "CH"; 
      break;
    case 0xFFE21D:  
      return "CH+";
      break;
    case 0xFF22DD:  
      return "PREV";
      break;
    case 0xFF02FD:  
      return "NEXT";
      break;
    case 0xFFC23D:  
      return "PLAY/PAUSE";
      break;
    case 0xFFE01F:  
      return "VOL-";
      break;
    case 0xFFA857:  
      return "VOL+";
      break;
    case 0xFF906F:  
      return "EQ";
      break;
    case 0xFF6897:  
      return "0"; 
      break;
    case 0xFF9867:  
      return "100+"; 
      break;
    case 0xFFB04F:  
      return "200+"; 
      break;
    case 0xFF30CF:  
      return "1"; 
      break;
    case 0xFF18E7:  
      return "2"; 
      break;
    case 0xFF7A85:  
      return "3"; 
      break;
    case 0xFF10EF:  
      return "4"; 
      break;
    case 0xFF38C7:  
      return "5"; 
      break;
    case 0xFF5AA5:  
      return "6"; 
      break;
    case 0xFF42BD:  
      return "7"; 
      break;
    case 0xFF4AB5:  
      return "8"; 
      break;
    case 0xFF52AD:  
      return "9"; 
      break;
    default: 
      return "Ongekende knop - Voeg toe!";
    }

  delay(500);


} //END translateIR


