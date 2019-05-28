#include <OneWire.h>
#include <LiquidCrystal.h>
#include <Servo.h>

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
int motorpin = A3;
int potpin = A0;

int overheattemp = 30;
int hottemp = 27;
int coldtemp = 20;
int selecthottemp = 0;
int timer = 30;
int temptype = 0;
int ovhtconvertedtok = 0;
int hotconvertedtok = 0;
int coldtconvertedtok = 0;
int ovhtconvertedtof = 0;
int hotconvertedtof = 0;
int coldconvertedtof = 0;

int buttonstate = 0;
int editmode = 0;
int greenled = 0;
int yellowled = 0;
int redled = 0;

int allgood = 0;
int warning = 0;
int danger = 0;
int noswarning = 0;

Servo servo1;
int servopin = 6;
int angle = 0;

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
  pinMode(motorpin, OUTPUT);
  pinMode(servopin, OUTPUT);
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
  
  
  
  servo1.attach(6);
  
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
  
  if (temptype == 0){
  lcd.print("Temp: ");
  lcd.setCursor(14, 0);
  lcd.setCursor(6, 0);
  lcd.print(Whole);
  lcd.print("C");
  }
  if (temptype == 1){
  Whole = Whole + 235;
  lcd.print("Temp: ");
  lcd.setCursor(14, 0);
  lcd.setCursor(6, 0);
  lcd.print(Whole);
  lcd.print("K");
  }
  if (temptype == 2){
  Whole = (Whole * 9/5) + 32;
  lcd.print("Temp: ");
  lcd.setCursor(14, 0);
  lcd.setCursor(6, 0);
  lcd.print(Whole);
  lcd.print("F");
  }
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
     for(angle = 0; angle < 170; angle += 1) 	 // command to move from 0 degrees to 180 degrees 
      {       
        servo1.write(angle);              	 //command to rotate the servo to the specified angle
        delay(5);        
      }
    }
    if (warning == 1 || noswarning == 1)
    {
     lightonlyyellow();
    }
    else if (warning >= 2)
    {
    lightonlyred();
    for(angle = 0; angle < 170; angle += 1) 	 // command to move from 0 degrees to 180 degrees 
      {                                  
        servo1.write(angle);              	 //command to rotate the servo to the specified angle
        delay(5);        
      }
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
  servo1.detach(); 
  delay(1000);
  Serial.print("Button state: ");
  Serial.println(buttonstate);
  Serial.println(warning);
  noTone(buzzer);
  lcd.clear();
  warning = 0;
  noswarning = 0;
  
  editing:
  
  if(buttonstate == HIGH && editmode == 0)
  {
    editmode = 1; 
  }
  if(editmode == 1)
  {
   while (editmode > 0){
     if (editmode == 1){
       buttonstate = digitalRead(buttonpin);
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("EDIT MODE");
       lcd.setCursor(12,0);
       lcd.print(timer);
       lcd.setCursor(0,1);
       lcd.print("Entr hot tmp: ");
       lcd.setCursor(13,1);
       hottemp = analogRead(potpin);
       hottemp = map(hottemp, 0, 1023, 0, 40);
       lcd.print(hottemp);
       timer--;
       delay(1000);
       }
       if (timer == 0){
         editmode = 0;
         timer = 30;
       }
       if (timer < 28 && buttonstate == HIGH && editmode == 1)
       {
        editmode++;
        timer = 30; 
       }  
       /*if(buttonstate == HIGH && editmode == 1){
         editmode = 0;
         timer = 30;
       }*/
     if (editmode == 2){
       buttonstate = digitalRead(buttonpin);
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("EDIT MODE");
       lcd.setCursor(12,0);
       lcd.print(timer);
       lcd.setCursor(0,1);
       lcd.print("Entr ovh tmp: ");
       lcd.setCursor(13,1);
       overheattemp = analogRead(potpin);
       overheattemp = map(overheattemp, 0, 1023, 0, 40);
       lcd.print(overheattemp);
       timer--;
       delay(1000);
       }
       if (timer == 0){
         editmode = 0;
         timer = 30;
       }
       if (timer < 28 && buttonstate == HIGH && editmode == 2)
       {
        editmode++;
        timer = 30; 
       }  
     if (editmode == 3){
       buttonstate = digitalRead(buttonpin);
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("EDIT MODE");
       lcd.setCursor(12,0);
       lcd.print(timer);
       lcd.setCursor(0,1);
       lcd.print("Entr cld tmp: ");
       lcd.setCursor(13,1);
       coldtemp = analogRead(potpin);
       coldtemp = map(coldtemp, 0, 1023, 0, 40);
       lcd.print(coldtemp);
       timer--;
       delay(1000);
       }
       if (timer == 0){
         editmode = 0;
         timer = 30;
       }
       if (timer < 28 && buttonstate == HIGH && editmode == 3)
       {
        editmode++;
        timer = 30; 
       }  
     if (editmode == 4){
       buttonstate = digitalRead(buttonpin);
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("EDIT MODE");
       lcd.setCursor(12,0);
       lcd.print(timer);
       lcd.setCursor(0,1);
       lcd.print("Entr tmp mod: ");
       lcd.setCursor(13,1);
       temptype = analogRead(potpin);
       temptype = map(temptype, 0, 1023, 0, 2);
       lcd.print(temptype);
       timer--;
       delay(1000);
       
       if (timer == 0){
         editmode = 0;
         timer = 30;
         if (temptype == 1){
          overheattemp = overheattemp + 235;
          hottemp = hottemp + 235;
          coldtemp = coldtemp + 235;
          }
        if (temptype == 2){
          overheattemp = (overheattemp * 9/5) + 32;
          hottemp = (hottemp * 9/5) + 32;
          coldtemp = (coldtemp * 9/5) + 32;
          }
       }
       if (timer < 28 && buttonstate == HIGH && editmode == 4)
       {
        editmode++;
        timer = 30;
        if (temptype == 1){
          overheattemp = overheattemp + 235;
          hottemp = hottemp + 235;
          coldtemp = coldtemp + 235;
          }
        if (temptype == 2){
          overheattemp = (overheattemp * 9/5) + 32;
          hottemp = (hottemp * 9/5) + 32;
          coldtemp = (coldtemp * 9/5) + 32;
          }
       }  
       if (timer == 0){
         editmode = 0;
         timer = 30;
       }
     if (editmode == 5){
     editmode = 0;
     timer = 30;
     goto start;
     }
     }  
  }
  
  }
  }
  
  //End conversion to C
