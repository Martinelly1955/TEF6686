/*
 * original sketch from https://github.com/makserge/tef6686_radio
 * adapted for i2c display and phical buttons by Nicu FLORICA (niq_ro)
 * http://www.arduinotehniq.com/
 * add RadioText near PI, PS, etc info for RDS
 * v.5: add. facility to work with STM32F103, must use Wire library from https://github.com/stm32duino/Arduino_Core_STM32/tree/master/libraries/Wire
 * v.6beta - added band changes (FM1.FM2, FM3, LW, MW. SW) - I'm not satisfiet yef for AM receiver part 
 */

#include "TEF6686.h"

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>  // https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int frequency;
int volume;
int volume2;
byte plus = 0;
int doarRT0 = 0;
byte doarRT = 0;
int nivel = 0;
int banda = 0;
int banda0 = 0;
byte stereo = 0;

unsigned long somn = 200;
unsigned long trezire;

uint8_t isRDSReady;  

char programTypePrevious[17];
char programServicePrevious[9];
char radioTextPrevious[65];
char radioTextRezumat[20];
char programIdPrevious[4];

boolean isFmSeekMode;
boolean isFmSeekUp;

TEF6686 radio;
RdsInfo rdsInfo;

/*
// for Arduino Mega
#define volumplus 4
#define volumminus 5
#define cautareplus 6
#define cautareminus 7
#define inainte 8
#define inapoi 9
#define scbanda 10
#define pinRT 10
*/
// for STM32F103
#define volumplus PA0
#define volumminus PA1
#define cautareplus PA2
#define cautareminus PA3
#define inainte PA4
#define inapoi PA5 //
#define scbanda PA6

//#define pinRT 10

/*
byte antena[] = {   // https://maxpromer.github.io/LCD-Character-Creator/
  B10001,
  B10101,
  B10101,
  B01110,
  B00100,
  B00100,
  B00100,
  B00100
};
*/

byte antena[] = {
  B11111,
  B10101,
  B10101,
  B01110,
  B01110,
  B00100,
  B00100,
  B00100
};


byte difuzor[] = {
  B00001,
  B00011,
  B11101,
  B10101,
  B10101,
  B11101,
  B00011,
  B00001
};

byte difuzor2[] = {
  B10000,
  B11000,
  B10111,
  B10101,
  B10101,
  B10111,
  B11000,
  B10000
};




void setup() {
  pinMode(volumminus, INPUT);
  pinMode(volumplus, INPUT);
  pinMode(cautareminus, INPUT);
  pinMode(cautareplus, INPUT);
  pinMode(inainte, INPUT);
  pinMode(inapoi, INPUT); 
  digitalWrite(volumminus, HIGH);
  digitalWrite(volumplus, HIGH);
  digitalWrite(cautareminus, HIGH);
  digitalWrite(cautareplus, HIGH);  
  digitalWrite(inainte, HIGH);
  digitalWrite(inapoi, HIGH); 
  
   
  lcd.begin(); // initialize the LCD
  lcd.createChar(0, antena);
  lcd.createChar(1, difuzor);
  lcd.createChar(2, difuzor2);
  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear(); 
  // Print a logo message to the LCD.
  lcd.setCursor(0,0);
  lcd.print(" TEF6686 radio v.6.0");
  lcd.setCursor(0,1);
  lcd.print("github.com/makserge ");
  lcd.setCursor(0,2);
  lcd.print("  updated by niq_ro ");
  lcd.setCursor(0,3);
  lcd.print(" github.com/tehniq3 ");
  delay (3000);
  lcd.clear();

  Serial.begin(115200);
  while (!Serial);
  Serial.println("TEF6686 radio");
  radio.init();
  radio.powerOn();
  radio.setFrequency(9200);
  frequency = radio.getFrequency();
  banda = Radio_GetCurrentBand();
  banda0 = banda;
  radio.clearRDS();
  guma();
  displayInfo();    
}

void loop() {
  
   if (millis() - trezire > somn) 
   {
   if (banda < 3) // for FM
   {
     readRds();
     //trezire = millis();
   }
   displayInfo();
   trezire = millis();
   }
//  readRds();
//  showFmSeek();
//  displayInfo();
//  radioGui(0);
//  displayInfo();

   if (digitalRead(volumplus) == LOW)
    {
      volume += 4;
      if (volume >= 24) volume = 24;
      radio.setVolume(volume);
      displayInfo();
      delay(250);
    } 
    if (digitalRead(volumminus) == LOW)
    {
      volume -= 4;
      if (volume < -60) volume = -60;
      radio.setVolume(volume);
      displayInfo();
      delay(250);
    } 
    if (digitalRead(cautareplus) == LOW)
    {
      //lcd.clear();
      frequency = radio.seekUp();
     // banda = Radio_GetCurrentBand();
      delay(200);
      guma();
      displayInfo();
      //
    }
    if (digitalRead(cautareminus) == LOW)
    {
      frequency = radio.seekDown();
      banda = Radio_GetCurrentBand();
      guma();
      //displayInfo();
      //delay(250);
    }

    if (digitalRead(inainte) == LOW)
    {
      frequency = radio.tuneUp();
      guma();
      //displayInfo();
      //delay(250);
    }
   if (digitalRead(inapoi) == LOW)
    {
      frequency = radio.tuneDown();
      guma();
      //displayInfo();
      //delay(250);
    }
    if (digitalRead(scbanda) == LOW)
    {
      lcd.clear();
      Radio_NextBand();
    //  banda0 = Radio_GetCurrentBand();
    //  banda = banda + 1;
    //  if (banda > 5) banda = 0;
      guma();
      //readRds();
      displayInfo();
      delay(250);
    }

//banda = Radio_GetCurrentBand();




 /*
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'm') {
      radio.setMute();
      displayInfo();
    }
    else if (ch == 'n') {
      radio.setUnMute();
      displayInfo();
    }
    else if (ch == 'p') {
      radio.powerOn();
      displayInfo();
    } 
    else if (ch == 'o') {
      radio.powerOff();
      displayInfo();
    }
    else if (ch == 'u') {
      frequency = radio.seekUp();
      lcd.clear();
      displayInfo();
    } 
    else if (ch == 'd') {
      frequency = radio.seekDown();
      lcd.clear();
      displayInfo();
    }
    else if (ch == 'c') {
      Serial.println("Seeking up");
      isFmSeekMode = true;
      isFmSeekUp = true;
    } 
    else if (ch == 'e') {
      Serial.println("Seeking down");
      isFmSeekMode = true;
      isFmSeekUp = false;
    }
    else if (ch == '+')
    {
      volume += 4;
      if (volume >= 24) volume = 24;
      radio.setVolume(volume);
//      lcd.clear();
      displayInfo();
    } 
    else if (ch == '-')
    {
      volume -= 4;
      if (volume < -60) volume = -60;
      radio.setVolume(volume);
      displayInfo();
    } 
    else if (ch == '8')
    {
      frequency = radio.tuneUp();
      displayInfo();
    }
    else if (ch == '2')
    {
      frequency = radio.tuneDown();
      displayInfo();
    }
  }
*/
delay(10);
doarRT0 = doarRT0 + 1;
if (doarRT0 % 100 == 0) doarRT = doarRT + 1;
} // end main loop

void readRds() {
 // radio.clearRDS();
  isRDSReady = radio.readRDS(); 
  radio.getRDS(&rdsInfo);
  
  showPTY();
  showPS();
  showRadioText(); 
  displayInfo();
}

void showPTY() {
  if ((isRDSReady == 1) && !strcmp(rdsInfo.programType, programTypePrevious, 16)) { 
    Serial.print(rdsInfo.programType);
    strcpy(programTypePrevious, rdsInfo.programType);
    Serial.println();

  //  lcd.print(strcpy(programTypePrevious, rdsInfo.programType));
  }  
  
}

void showPS() {
  if ((isRDSReady == 1) && (strlen(rdsInfo.programService) == 8) && !strcmp(rdsInfo.programService, programServicePrevious, 8)) {
    Serial.print("-=[ ");
    Serial.print(rdsInfo.programService);
    Serial.print(" ]=-");
    strcpy(programServicePrevious, rdsInfo.programService);
    Serial.println();
  }
}


void showRadioText() {
  if ((isRDSReady == 1) && !strcmp(rdsInfo.radioText, radioTextPrevious, 65)){
//  if ((isRDSReady == 1) && !strcmp(rdsInfo.radioText, radioTextPrevious, 20)){
    Serial.println(rdsInfo.radioText);
    strcpy(radioTextPrevious, rdsInfo.radioText);
   
     Serial.println();
     Serial.print("--> ");
 //   Serial.print(radioTextRezumat);
     Serial.println();   
  }
}

bool strcmp(char* str1, char* str2, int length) {
  for (int i = 0; i < length; i++) {
    if (str1[i] != str2[i]) {
      return false;
    }    
  }  
  return true;
}

void displayInfo() {
   delay(10);
   frequency = radio.getFrequency();
   nivel = radio.getLevel();
   stereo = radio.getStereoStatus();
   banda = Radio_GetCurrentBand();
   Serial.print("Frequency:"); 
   Serial.print(frequency); 
   Serial.print(" Volume:"); 
   Serial.println(volume);
   Serial.print("Level:"); 
   Serial.println(nivel);
   Serial.print("Stereo:"); 
   Serial.println(stereo);
   Serial.print("Band:"); 
   Serial.println(banda);  

    Serial.println();
    Serial.print(F("PI:  "));
    showPI();
    Serial.print(F("      ( "));

  // volume
    lcd.setCursor(1,1);
    lcd.write(1);  // speaker sign
    if (volume > 0) 
    {
      plus = 1;
      volume2 = volume;
    }
    else
    {
      plus = 0;
      volume2 = -volume;
    }
    if (volume2/10 == 0) 
    lcd.print(" "); 
    if (volume2 == 0) lcd.print(" ");
    else
    if (plus == 0)
    {
    lcd.print("-"); 
    }
    else
    {
    lcd.print("+"); 
    }
    lcd.print(volume2);
    lcd.print("dB ");

    // antenna level
  //  nivel = radio.getLevel();
    if (nivel > 100) nivel = 0;
    lcd.setCursor(15,0);
    if (nivel < 100)
    lcd.print(" "); 
    if (nivel < 10)
    lcd.print(" ");      
    lcd.print(nivel);
    lcd.print("%");
    lcd.write(0);  // antenna sign

banda = Radio_GetCurrentBand();
    // frequency
    lcd.setCursor(0, 0);
 if ((banda >=0) and (banda < 3))
 {
    if (frequency < 10000) lcd.print(" ");
    lcd.print(frequency/100); 
    lcd.print(",");
    lcd.print(frequency%100/10); // thanks to carkiller08
  //  lcd.print(frequency%10);
  //  lcd.print("MHz");
 }
 else
 if ((banda == 3) or (banda == 4))  // LW or MW
 {
    if (frequency < 1000) lcd.print(" ");
    lcd.print(frequency); 
    lcd.print("kHz ");
 }
 else 
if (banda == 5)  // SW
 {
  if (frequency < 10000) lcd.print(" ");
  //  lcd.print(frequency); 
    lcd.print(frequency/1000); 
    lcd.print(",");
    lcd.print(frequency%1000); // thanks to carkiller08 
    lcd.print("MHz");
 }
  
  
    // band indicator
    lcd.setCursor(10,0);
    if (banda == 3) lcd.print("MW   ");
    else
    if (banda == 4) lcd.print("LW   ");
    else
    if (banda == 5) lcd.print("SW   ");
    else
    if (banda == 0) 
    {
      lcd.setCursor(5,0);
      lcd.print("FM1 ");
    }
    else
    if (banda == 1)
    {
      lcd.setCursor(5,0);
      lcd.print("FM2 ");
    }
    else
    if (banda == 2)
    {
      lcd.setCursor(5,0);
      lcd.print("FM3 ");
    }
  

if ((banda >=0) and (banda < 3)) // FM
{

    // programID (PI)
    lcd.setCursor(10,0);
    //lcd.print("Pi: ");
    lcd.print(programIdPrevious); 
    lcd.setCursor(14,0);
    lcd.print(" ");
    
  // stereo
    lcd.setCursor(0,1);
    if (stereo == 1)
    {
    lcd.write(2);  // speaker2 sign
    }
    else
    lcd.print(" ");   
    
  if (doarRT % 2 == 0)
  {
      //PTY
    /*
    lcd.setCursor(0,2);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("                    ");
    */
    Serial.print(programTypePrevious);
    Serial.println();
     lcd.setCursor(0,3);
 //   lcd.print(programTypePrevious); 
     lcd.print("PTY:");
 //   lcd.print(programTypePrevious); 
       for (int i = 0; i < 15; i++) 
     {
      Serial.print(programTypePrevious[i]);
      lcd.setCursor(i+4,3);
      lcd.print(programTypePrevious[i]);   
     }

    for (int i = 0; i < 20; i++) 
     {
      Serial.print(radioTextPrevious[i]);
      lcd.setCursor(i,2);
      lcd.print(radioTextPrevious[i]);   
     }
  }
  else
   {
    // program service
    lcd.setCursor(10,1);
    lcd.print(programServicePrevious);
    lcd.print(" ");

       // RadioText
/*     
     lcd.setCursor(0,2);
     lcd.print("                    ");
     lcd.setCursor(0,3);
     lcd.print("                    ");
 */    
     for (int i = 0; i < 20; i++) 
     {
      Serial.print(radioTextPrevious[i]);
      lcd.setCursor(i,2);
      lcd.print(radioTextPrevious[i]);   
     }
     for (int i = 20; i < 40; i++) 
     {
      Serial.print(radioTextPrevious[i]);
      lcd.setCursor(i-20,3);
      lcd.print(radioTextPrevious[i]);   
     }
   }

}

}

void showFmSeek() {
  if (isFmSeekMode) {
    if (radio.seekSync(isFmSeekUp)) {
      isFmSeekMode = false;
      Serial.println("Seek stopped");
   //   lcd.setCursor(3, 1);
   //   lcd.print("Seek stopped");    
      frequency = radio.getFrequency();
      displayInfo();
    }
  }
}


void showPI() {
  if (isNewPi()){
    strcpy(programIdPrevious, rdsInfo.programId);
  }
  Serial.print(rdsInfo.programId);
     // programID (PI)
   // lcd.setCursor(10,0);
    //lcd.print("Pi: ");
   // lcd.print(rdsInfo.programId); 
}

bool isNewPi(){
  return (strlen(rdsInfo.programId) == 4) && !strcmp(rdsInfo.programId, programIdPrevious, 4);
}


void guma()
{
  lcd.clear();
  radio.clearRDS();
//programTypePrevious = "                ";
//programServicePrevious =  "        ";
//radioTextRezumat[20];
//programIdPrevious = "    ";
  strcpy(programTypePrevious, "     No PTY     ");
  strcpy(programIdPrevious, "    ");
  strcpy(programServicePrevious, "        ");
//  strcpy(rdsProgramServiceUnsafe, "        ");
  strcpy(radioTextPrevious, "                                                 ");
 // banda = 6;
}


void displayInfo1(byte banda1) {
   delay(10);
 //  banda = Radio_GetCurrentBand();
   frequency = radio.getFrequency();
   nivel = radio.getLevel();
   stereo = radio.getStereoStatus();
   Serial.print("Frequency:"); 
   Serial.print(frequency); 
   Serial.print(" Volume:"); 
   Serial.println(volume);
   Serial.print("Level:"); 
   Serial.println(nivel);
   Serial.print("Stereo:"); 
   Serial.println(stereo);
   Serial.print("Band:"); 
   Serial.println(banda1);  

    Serial.println();
    Serial.print(F("PI:  "));
    showPI();
    Serial.print(F("      ( "));

  // volume
    lcd.setCursor(1,1);
    lcd.write(1);  // speaker sign
    if (volume > 0) 
    {
      plus = 1;
      volume2 = volume;
    }
    else
    {
      plus = 0;
      volume2 = -volume;
    }
    if (volume2/10 == 0) 
    lcd.print(" "); 
    if (volume2 == 0) lcd.print(" ");
    else
    if (plus == 0)
    {
    lcd.print("-"); 
    }
    else
    {
    lcd.print("+"); 
    }
    lcd.print(volume2);
    lcd.print("dB ");

    // antenna level
  //  nivel = radio.getLevel();
    if (nivel > 100) nivel = 0;
    lcd.setCursor(15,0);
    if (nivel < 100)
    lcd.print(" "); 
    if (nivel < 10)
    lcd.print(" ");      
    lcd.print(nivel);
    lcd.print("%");
    lcd.write(0);  // antenna sign


    // frequency
    lcd.setCursor(0, 0);
 if ((banda1 >=0) and (banda1 < 3))
 {
    if (frequency < 10000) lcd.print(" ");
    lcd.print(frequency/100); 
    lcd.print(",");
    lcd.print(frequency%100/10); // thanks to carkiller08
  //  lcd.print(frequency%10);
  //  lcd.print("MHz");
 }
 else
 if ((banda1 == 3) or (banda1 == 4))  // LW or MW
 {
    if (frequency < 1000) lcd.print(" ");
    lcd.print(frequency); 
    lcd.print("kHz ");
 }
 else 
if (banda1 == 5)  // SW
 {
  if (frequency < 10000) lcd.print(" ");
  //  lcd.print(frequency); 
    lcd.print(frequency/1000); 
    lcd.print(",");
    lcd.print(frequency%1000); // thanks to carkiller08 
    lcd.print("MHz");
 }
  
  
    // band indicator
    lcd.setCursor(10,0);
    if (banda1 == 3) lcd.print("LW   ");
    else
    if (banda1 == 4) lcd.print("MW   ");
    else
    if (banda1 == 5) lcd.print("SW   ");
    else
    if (banda1 == 0) 
    {
      lcd.setCursor(5,0);
      lcd.print("FM1 ");
    }
    else
    if (banda1 == 1)
    {
      lcd.setCursor(5,0);
      lcd.print("FM2 ");
    }
    else
    if (banda1 == 2)
    {
      lcd.setCursor(5,0);
      lcd.print("FM3 ");
    }
  
if ((banda >=0) and (banda < 3)) // FM
{

    // programID (PI)
    lcd.setCursor(10,0);
    //lcd.print("Pi: ");
    lcd.print(programIdPrevious); 
    lcd.setCursor(14,0);
    lcd.print(" ");
    
  // stereo
    lcd.setCursor(0,1);
    if (stereo == 1)
    {
    lcd.write(2);  // speaker2 sign
    }
    else
    lcd.print(" ");   
    
  if (doarRT % 2 == 0)
  {
      //PTY
    Serial.print(programTypePrevious);
    Serial.println();
     lcd.setCursor(0,3);
 //   lcd.print(programTypePrevious); 
     lcd.print("PTY:");
 //   lcd.print(programTypePrevious); 
       for (int i = 0; i < 15; i++) 
     {
      Serial.print(programTypePrevious[i]);
      lcd.setCursor(i+4,3);
      lcd.print(programTypePrevious[i]);   
     }

    for (int i = 0; i < 20; i++) 
     {
      Serial.print(radioTextPrevious[i]);
      lcd.setCursor(i,2);
      lcd.print(radioTextPrevious[i]);   
     }
  }
  else
   {
    // program service
    lcd.setCursor(10,1);
    lcd.print(programServicePrevious);
    lcd.print(" ");

       // RadioText

     for (int i = 0; i < 20; i++) 
     {
      Serial.print(radioTextPrevious[i]);
      lcd.setCursor(i,2);
      lcd.print(radioTextPrevious[i]);   
     }
     for (int i = 20; i < 40; i++) 
     {
      Serial.print(radioTextPrevious[i]);
      lcd.setCursor(i-20,3);
      lcd.print(radioTextPrevious[i]);   
     }
   }

}

}
