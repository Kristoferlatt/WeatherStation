#include <OneWire.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int photoresistorPin = A2; // выбор вх.пина для фоторезистора
int photoresistorValue = 0; // переменная для значения принятого от датчика

int greenpin = 8;
int yellowpin = 9;
int redpin = 13;
int buttonpin = 7;
int buzzer = A1;
int potpin = A0;

int overheattemp = 30;
int hottemp = 27;
int coldtemp = 20;
int selecthottemp = 0;
int timer = 30;

int buttonstate = 0;
int editmode = 0;
int greenled = 0;
int yellowled = 0;
int redled = 0;

int allgood = 0;
int warning = 0;
int danger = 0;
int noswarning = 0;

void lightgreen(){
  digitalWrite(greenpin, HIGH);
}

void lightyellow(){
  digitalWrite(yellowpin, HIGH);
}

void lightred(){
  digitalWrite(redpin, HIGH);
}

void lightonlygreen(){
  digitalWrite(greenpin, HIGH);
  digitalWrite(yellowpin, LOW);
  digitalWrite(redpin, LOW);
}

void lightonlyyellow(){
  digitalWrite(yellowpin, HIGH);
  digitalWrite(greenpin, LOW);
  digitalWrite(redpin, LOW);
}

void lightonlyred(){
  digitalWrite(redpin, HIGH);
  digitalWrite(greenpin, LOW);
  digitalWrite(yellowpin, LOW);
}

void nolight(){
  digitalWrite(redpin, LOW);
  digitalWrite(greenpin, LOW);
  digitalWrite(yellowpin, LOW);
}
 
// DS18S20 Temperature chip i/o
OneWire ds(10);  // on pin 10
 
void setup(void) {
  // initialize inputs/outputs
  // start serial port
  
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(greenpin, OUTPUT);
  pinMode(yellowpin, OUTPUT);
  pinMode(redpin, OUTPUT);
  pinMode(buttonpin, INPUT);
  pinMode(buzzer, OUTPUT);
  // Print a message to the LCD.
}
 
void loop(void) {
 
  //For conversion of raw data to C
  start:
  int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
 
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  
  
  
  
  photoresistorValue = analogRead(photoresistorPin);
  Serial.println(photoresistorValue, DEC);
  
  buttonstate = digitalRead(buttonpin);
  
  
  if ( !ds.search(addr)) {
      //Serial.print("No more addresses.\n");
      ds.reset_search();
      return;
  }
 
  //Serial.print("R=");
  for( i = 0; i < 8; i++) {
    //Serial.print(addr[i], HEX);
    //Serial.print(" ");
  }
 
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      //Serial.print("CRC is not valid!\n");
      return;
  }
 
  if ( addr[0] == 0x10) {
      //Serial.print("Device is a DS18S20 family device.\n");
  }
  else if ( addr[0] == 0x28) {
      //Serial.print("Device is a DS18B20 family device.\n");
  }
  else {
      //Serial.print("Device family is not recognized: 0x");
      //Serial.println(addr[0],HEX);
      return;
  }
 
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
 
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
 
 // Serial.print("P=");
  //Serial.print(present,HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
 // Serial.print(" CRC=");
  //Serial.print( OneWire::crc8( data, 8), HEX);
 // Serial.println();
 
  //Conversion of raw data to C
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
 
  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
 
 
  if (SignBit) // If its negative
  {
     Serial.print("-");
  }
  Serial.print(Whole);
  //Serial.print(".");
  if (Fract < 10)
  {
     Serial.print("0");
  }
  //Serial.print(Fract);
 
  Serial.print("\n");
  
  lcd.print("Temp: ");
  lcd.setCursor(14, 0);
  lcd.setCursor(6, 0);
  lcd.print(Whole);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Light: ");
  lcd.setCursor(7, 1);
  lcd.print(photoresistorValue);
  if (photoresistorValue < 100 && photoresistorValue >= 50 )
  {
    lcd.setCursor(11, 1);
    lcd.print("Light");
  }
  else if (photoresistorValue < 49)
  {
    lcd.setCursor(11, 1);
    lcd.print("Bright");
    noswarning++;
  }
  else if (photoresistorValue > 101 && photoresistorValue < 299)
  {
    lcd.setCursor(11, 1);
    lcd.print("Norm");
  }
  else if (photoresistorValue > 300 && photoresistorValue < 449)
  {
    lcd.setCursor(11, 1);
    lcd.print("Dark");
    noswarning++;
  }
  else if (photoresistorValue > 450)
  {
    lcd.setCursor(11, 1);
    lcd.print("Black");
    noswarning++;
  }
  if (Whole >= hottemp && Whole < overheattemp)
    {
     warning++;
     lcd.setCursor(10, 0);
     lcd.print("Hot"); 
    }
  else if (Whole <= coldtemp)
  {
    lcd.setCursor(10, 0);
    lcd.print("Cold"); 
  }  
  else if (Whole >= overheattemp)
  {
   lcd.setCursor(10, 0);
   lcd.print("Ovrht");
   warning++;
   warning++; 
  }
  else
  {
   lcd.setCursor(10, 0);
   lcd.print("Norm");
  }
    if (warning == 1)
    {
     lightonlyyellow();
    }
    if (warning == 1 || noswarning == 1)
    {
     lightonlyyellow();
    }
    else if (warning >= 2)
    {
    lightonlyred();	 
    tone(buzzer, 1000);
    }
    else if (warning == 0 && noswarning == 0)
    {
     lightonlygreen(); 
    }
  
  else
  {
    nolight();
  }
 
  delay(1000);
  Serial.print("Button state: ");
  Serial.println(buttonstate);
  Serial.println(warning);
  noTone(buzzer);
  lcd.clear();
  warning = 0;
  noswarning = 0;
  }
  
  //End conversion to C
