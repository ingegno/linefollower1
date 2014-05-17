/************************************************************/
/*          lichtsensor callibratie voor lijnvolgen         */
/************************************************************/

#define SCHAAL_FOUT 1000

int black[2]  = {501,1000}; // hiertussen zouden de waarden voor zwart moeten zijn
//test of je groen, geel, wit kan zien
//waarschijnlijk niet ....
int green[2]  = { 75, 100};
int yellow[2] = { 80, 105};
int white[2]  = {  0, 500};

//Saya  robot
//int corrwhite[5] = {0, 0, 0, 100, 200};
//int corrblack[5] = {0, 50, 0, 20, 50};
//Gudrun robot
//int corrwhite[5] = {0, 0, 50, 0, 0};
//int corrblack[5] = {0, -10, 20, 0, 0};
//Thiemen robot
//int corrwhite[5] = {0, 0, 0, 0, 0};
//int corrblack[5] = {0, 70, 50, 30, 40};
//Jasper robot
//int corrwhite[5] = {100, 0, 0, 0, 100};
//int corrblack[5] = {0, 0, 0, 0, 0};
//Blonde stef robot
//int corrwhite[5] = {40, 0, 0, 0, 0};
//int corrblack[5] = {40, 0, 0, 0, 0};
// stef robot
//int corrwhite[5] = {0, 0, 100, 0, 0};
//int corrblack[5] = {0, 0, 0, 0, 0};
// marie-louise robot
int corrwhite[5] = {0, 0, 0, 100, 40};
int corrblack[5] = {0, 0, 0, 0, 0};


//wijzigende variabelen
float sensors_average;
int sensors_sum;
int baseline;
float sensors_average_bl; // hier houden we rekening met een baseline, dat is we trekken het minimun ervan af
int sensors_sum_bl;  // hier houden we rekening met een baseline, dat is we trekken het minimun ervan af

float mid_point = 2; //midden van de sensoren 0,1,2,3,4
float max_err = max(4-mid_point, mid_point-0) * SCHAAL_FOUT;
float min_err = 0.5 * SCHAAL_FOUT;
long error_value;

// Array om de 5 sensorlezingen in op te slaan
long sensors[] = {0, 0, 0, 0, 0};

void setup(){ 
  Serial.begin(9600); // dit start serial monitor zodat we kunnen lezen wat de sensoren terug geven als waarden
  // analog pins don't need setup!
}

void loop(){ 
  sensors_average = 0;
  sensors_sum = 0;
  sensors_average_bl = 0;
  sensors_sum_bl = 0;
  baseline = 1024;
  Serial.println("Plaats me op een kleur en noteer welke waarden ik meet tussen 0 en 1023!");
  for (int i = 0; i < 5; i++){
    sensors[i] = analogRead(i);
    if (sensors[i] < 501){ sensors[i] = sensors[i]-corrwhite[i];}
    else { sensors[i] = sensors[i]-corrblack[i];}
    if (baseline > sensors[i]){
      baseline = sensors[i];
    }
    Serial.print("Gemeten met sensor "); 
    Serial.print(i); Serial.print(": "); // deze drie dingen schrijven gewoon het volgende: "gemeten op sensor i :"  en dan met volgende commando komt de waarde 
    Serial.print(sensors[i]);Serial.print(" ");
    if (sensors[i] >= black[0] && sensors[i] <= black[1]) {
      Serial.print("... ZWART");
    }
    if (sensors[i] >= white[0] && sensors[i] <= white[1]) {
      Serial.print("...  WIT");
    }
    Serial.println(" ");
  }//achter deze for loop is de baseline gelijk aan de laagste gemetn waarde
  
  for (int i = 0; i<5; i++){ 
    int val = sensors[i];
    sensors_average += val * i;
    //Calculating the weighted mean (dit is nodig om gewogen gemiddelde van de waarden geezen door de 5 sensoren te hebben)
    sensors_sum += val; // som van de gelezen waarden
    sensors_average_bl += (val-baseline) * i; 
    //Calculating the weighted mean maar nu trekken we laagste waarde ervan af
    sensors_sum_bl += val-baseline;  //som van de gelezen waarden waarbij de laagste waarde afgetrokken wordt
  }
  
  sensors_average=sensors_average/sensors_sum;         // gewogen gemiddelde
  sensors_average_bl=sensors_average_bl/sensors_sum_bl;// gewogen gemiddelde met baseline

  Serial.print("Normaal gemeten middelpunt van lijn:");
  Serial.println(sensors_average);
  Serial.print("Baseline gemeten middelpunt van lijn::");
  Serial.println(sensors_average_bl);
  Serial.println("Als lijn in het midden stond zou bovenstaand middelpunt je midpoint moeten zijn!");
  delay(1000);
  
 
  Serial.println("De fout (afwijking middelpunt) die we gemeten hebben ");
  error_value = long((sensors_average_bl - mid_point) * SCHAAL_FOUT);
  Serial.print("Fout = "); Serial.println(error_value);
  Serial.print("abs(fout) moet liggen tussen ");
  Serial.print(min_err);Serial.print(" en ");
  Serial.print(max_err);Serial.println(" voor goede foutcorrectie.");
  Serial.println(" ");
  delay(5000);
}

