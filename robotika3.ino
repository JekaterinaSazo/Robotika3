#include <EEPROM.h>

const int motorPin = 6;	    // Variklio pinas		

const int tempPin = A5;		// Temperaturos jutiklio pinas

const int btn1Pin = 2;		// Mygtuku pinai
const int btn2Pin = 3;

bool btnPressed[] = {false,false}; // Masyvas, saugantis ar mygtukai buvo paspausti
unsigned long debounceTimes[] = {0,0}; // Paskutinio paspaudimo laikai
const unsigned long debounceMinTime = 200; // Min. laiko tarpas tarp paspaudimu (200 ms)

int fanSpeeds[] = {0, 950, 990, 1023}; // Ventiliatoriaus 4 greicio lygiai [0 - isjungtas,1/3 galios,2/3 galios,1 - pilnas greitis]

byte currSpeed;

const int redPin = 10;		// RGB LED pinai
const int bluePin = 9;    // rodo temp. rezima pagal spalva

int tempMin = 18;			  // Leidziamos temp. reziai
int tempMax = 23;
byte currTempMode;			// Minimali temp. motoro pajungimui
float currTemp;				  // Dabartine temp., gauta is jutiklio

int redVal;					    // Stiprumas RGB Led raudonos ir melynos spalvu 
int blueVal;

bool checkTemp = true;		// timerio interuptas pasako kada laikas matuoti temp.(kas 10 ms)
int analogTemp;				    // temperaturos matuoklio (gryna) reiksme
int print = 0;				    // debug'o spausdinimo praretinimas

void setup()
{
  // Pirmiausiai sutvarkom timerio pertraukima po to darom likusi setup
  cli();                        // Sustabdom pertraukimus
  TCCR1A = 0;                   // Kadangi naudosim timer1 tai nustatom TCCR1A i 0
  TCCR1B = 0;                   // Tas pats su TCCR1B
  TCNT1  = 0;                   // Duodam pradine nuline reiksme skaiciuokliui (counteriui)
  
  // Nustatom palyginimo registra del 100Hz pertraukimu (kas 10ms)
  OCR1A = 19999;                // = 16000000 / (8 * 100) - 1
  // Ijungiam timerio nunulinima kai registras pasiekia nurodyta reiksme
  TCCR1B |= (1 << WGM12);
  // Nustatom CS12 CS11 ir CS10 bitus del 8 padidintojo
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
  // Ijungiam timerio palyginimo pertraukima
  TIMSK1 |= (1 << OCIE1A);
  sei();                        // Leidziam pertraukimus
  
  Serial.begin(9600);           // Spausdinimui
  pinMode(motorPin, OUTPUT);    // Nustatom motoro ir lempuciu pinus kaip isvesti
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(btn1Pin), btn1Interrupt, RISING); // Pajungiam pertraukimus mygtukams
  attachInterrupt(digitalPinToInterrupt(btn2Pin), btn2Interrupt, RISING);
  
  // Pirmiausiai patikrinam eeprom crc, jei blogas
  // Tada nustatom tempMode i 18 (zemiausia reiksme)
  // ir dabartini greiti i 0; (isjungtas)
  // tada issaugom Tempmode ir currSpeed kaip 18 ir 1
  // kad jei fenui reiktu isijungt jis turetu greiti
  // kuri gali pasiimti, kai issijungia fenas issaugom greiti
  // ir nustatom ji i 0, ir kai pasikeicia tempMode issaugom dabartine reiksme
  
  
  readEEPROM();
  setLight();
}

ISR(TIMER1_COMPA_vect){
   // Vyksta kas 10 ms, tai 100Hz
   // Paskaityti: https://www.instructables.com/Arduino-Timer-Interrupts/
   // Skaiciuotuvas: https://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
  checkTemp = true; //ijungia flag'a
}
void debug(){       // Spausdinam kintamuju reiksmes del klaidu taisymo ir stebejimo
  if(print ==  0){
    Serial.print("checkTemp: ");
    Serial.print(checkTemp);
    Serial.print("\t\tcurrSpeed: ");
    Serial.print(currSpeed);
    Serial.print("\t\tcurrTempMode: ");
    Serial.print(currTempMode);
    Serial.print("\t\tanalogTemp: ");
    Serial.print(analogTemp);
    Serial.print("\t\tcurrTemp: ");
    Serial.print(currTemp);
    Serial.print("\t\tredVal: ");
    Serial.print(redVal);
    Serial.print("\t\tblueVal: ");
    Serial.println(blueVal);
  }
  print++;
  if(print > 400) print = 0;
}
void loop()
{
  // Jei timeris liepe, pasiemam temperatura, jei buvo mygtuko pertraukimas pakeiciam minimumus, tada galiausiai
  // Patikrinam ar reikia ijungti fena, ir ijungiam jei reikia!
  debug();
  if(checkTemp){		// Kas 10 ms matuojama temp.
  	analogTemp = analogRead(tempPin);
    float voltage = analogTemp *  4.88; // Paverciame signala atgal i voltus
    currTemp = voltage/10;              // Voltus konvertuojame i temperatura (10mV/1 laipsnis C)
    checkTemp = false;
  }
  if(btnPressed[0]){ // Jei 1 mygt. paspaustas, keicia nustatyta min. temp. ir LED spalva
    changeTemp();
    btnPressed[0] = false;
  }
  if(btnPressed[1]){ // Jei 2 mygtukas paspaustas, keicia ventiliatoriaus greiti
    if(currSpeed == 0){
      readSpeed();
      changeSpeed();
      currSpeed = 0;
    } else{
      changeSpeed();
    }
    btnPressed[1] = false;
  }
  if(currTemp >= currTempMode){		// Jei temp. pakilo virs norimos temp.
    readSpeed();
  } else if(currSpeed != 0){		// Jei vel nukrito, isjungia ventiliatoriu
  	writeSpeed();
    currSpeed = 0;
  }
  clockwise();						// Suka ventiliatoriu teisinga kryptimi
  
}

void changeTemp(){					// Pakeicia min temp. ir LED spalva
  currTempMode++;
  if(currTempMode > tempMax){
  	currTempMode = tempMin;
  }
  writeTemp();
  setLight();
}
void setLight(){					//Keicia  LED spalvu santyki, ryskuma, spalva (18C - max. melyna, 23C - max. raudona)
  redVal = map(currTempMode, 18, 23, 1023, 0);
  blueVal = map(currTempMode, 23, 18, 1023, 0);
  analogWrite(redPin, redVal);
  analogWrite(bluePin, blueVal);
}
void changeSpeed(){					//Ventiliatoriaus greicio keitimas
  currSpeed++;
  if(currSpeed > 3){
    currSpeed = 1;
  }
  writeSpeed();
}
uint_fast16_t calculateCrc(){     // Apskaiciuojam CRC issaugotu reiksmiu tarp arduino
  const uint_fast16_t crc_table[16] = {
    0x0000, 0xcc01, 0xd801, 0x1400, 0xf001, 0x3c00, 0x2800, 0xe401,
    0xa001, 0x6c00, 0x7800, 0xb401, 0x5000, 0x9c01, 0x8801, 0x4400
  };
  uint_fast16_t crc = 0;
  int length = 2*sizeof(byte);  // 2 baitai, pirmas del temperaturos, antras del greicio
  for(int index = 0; index < length;index++){
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
  }
  return crc & 0xffff;
}
bool checkCrc(){    // Patikrinam ar issaugotas crc atitinka apskaiciuotam crc
  uint_fast16_t crc;
  EEPROM.get(2, crc);
  return crc == calculateCrc();
}
void writeCrc(){	// Iraso nauja crc, jei pasikeite duomenys
  	uint_fast16_t crc;
  	EEPROM.get(2, crc);
  	uint_fast16_t newCrc = calculateCrc();
  	if(crc != newCrc){
		  EEPROM.write(2, calculateCrc());
    }
}
void readTemp(){  // Perskaito isaugota temp. is EEPROM
	EEPROM.get(0, currTempMode);
  	if(currTempMode < 18 || currTempMode > 23){
      currTempMode = 18;
    }
}
void readSpeed(){ // Perskaito issaugota feno rezima is EEPROM
  	EEPROM.get(1, currSpeed);
  	if(currSpeed < 1 || currSpeed > 3){
      currSpeed = 1;
    }
}
void writeTemp(){ // Issaugo pasirinkta temp.i EEPROM
  	EEPROM.update(0, currTempMode);
  	writeCrc();
}
void writeSpeed(){ // Issaugo pasirinkta feno rezima i EEPROM
	EEPROM.update(1, currSpeed);
  	writeCrc();
}
void writeEEPROM(){ // Issaugo ir temp, ir feno rez. i EEPROM
  	EEPROM.update(0, currTempMode);
  	EEPROM.update(1, currSpeed);
  	writeCrc();
}
void readEEPROM(){ //  Bando perksiatyt is EEPROM, jei blogi duomenys nustato min. reiksme
  if(checkCrc()){
  	EEPROM.get(0, currTempMode);
    currSpeed = 0;
  } else{
  	currSpeed = 1;
    currTempMode = 18;
    writeEEPROM();
    currSpeed = 0;
  }
  if(currTempMode == 0){
  	currTempMode = 18;
    writeEEPROM();
  }
}

void clockwise(){  // Suka motora
  analogWrite(motorPin, fanSpeeds[currSpeed]);
}

void btn1Interrupt(){ 	// Mygtuku pertraukimai. Tikrina, ar nuo paskutinio paspaudimo 
  						          //preejo daugiau kaip 200 ms kad isvengtu dvigubu signalu
  unsigned long currIntTime = millis();
  if (currIntTime - debounceTimes[0] > debounceMinTime) {
    btnPressed[0] = true;
  }
  debounceTimes[0] = currIntTime;
}

void btn2Interrupt(){
  unsigned long currIntTime = millis();
  if (currIntTime - debounceTimes[1] > debounceMinTime) {
    btnPressed[1] = true;
  }
  debounceTimes[1] = currIntTime;
}