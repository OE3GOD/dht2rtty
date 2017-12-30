// RTTY Testcode OE3GOD
// Dezember 2017
// Sende DHT21 Temperatur und Luftfeuchte per RTTY

#include <stdint.h>
#include <Tone.h>
#include "DHT.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define DHTPIN 3     // Sensor-Data (gelb)
#define Audiopin 11  // 1kOhm Widerstand in Serie mit dem 8 Ohm Lautsprecher
#define DHTTYPE DHT21   // DHT 21 (AM2301)
// kein 10k Ohm Widerstand von +5Volt zur Datenleitung notwendig, hat der DHT21 bereits eingebaut

LiquidCrystal_I2C lcd(0x3f, 16, 2);

DHT dht(DHTPIN, DHTTYPE);

Tone tone1;
 
#define LETTERS_SHIFT 31
#define NUMBERS_SHIFT 27
#define LF 2
#define CR  8
#define bell 11
#define mark 2295
#define space 2125
 
#define is_lowercase(ch)    ((c) >= 'a' && (c) <= 'z' || (c) == ' ')
#define is_uppercase(ch)    ((c) >= 'A' && (c) <= 'Z' || (c) == ' ')
 
char letters_arr[32] = "xEnA SIUrDRJNFCKTZLWHYPQOBGxMXV";
char figures_arr[32] = "x3n- '87rw4b,x:(5+)2x6019?xx./=";
char outstr[20]; 

char current_mode;
 
void setup(){
  
  dht.begin();
  
  tone1.begin(Audiopin);

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("DHT21 goes RTTY");
  lcd.setCursor(0, 1);
  lcd.print("OE3GOD-Dez. 2017");  
  
  delay(2500);
}
 
void loop(){
    
  // Lesen der Sensordaten
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    lcd.clear();
    lcd.print("Read-Error DHT21 !");
  } else {
    // Starte Aussendung
    tone1.play(mark); 
    delay(1000); 
    
    // Auf Buchstaben umstellen, sonst erscheint 46 statt RY !
    rtty_txbyte(LETTERS_SHIFT);
    current_mode = 'L';
    delay(1000); 
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("H: "); 
    lcd.print(h);
    lcd.print(" %");
    lcd.setCursor(0, 1);
    lcd.print("T: "); 
    lcd.print(t);
    lcd.print(" *C");
    
    rtty_txstring("RYRYRYrn");
    rtty_txstring("ARDUINO SENSOR2RTTY DE OE3GODrn");
    dtostrf(t,7,1,outstr);
    rtty_txstring("TEMPERATURE: ");
    rtty_txstring(outstr);
    rtty_txstring(" DEGREES (C)");
    rtty_txstring("rn");
 
    dtostrf(h,7,1,outstr);
    rtty_txstring("HUMIDITY: ");
    rtty_txstring(outstr);
    rtty_txstring(" PERCENT");
    rtty_txstring("rn");
 
    delay(1000);
    tone1.stop();    
    
   }  
   
}

 
 void rtty_txstring(String txstr)
{

  char c;
  uint8_t b;
  int l,i;

  rtty_txbyte(LETTERS_SHIFT);
  current_mode = 'L';
  
  l = txstr.length();
  
   for (i = 0; l; i++)
    {
     char c = txstr.charAt(i);

    if (c == 'n')
    {
      rtty_txbyte(LF);
    }
    else if (c == 'r')
    {
      rtty_txbyte(CR);
    }
    else if (is_lowercase(txstr) || is_uppercase(txstr))
    {
       // if (is_lowercase(txstr))
       // {
        //  c -= 32;
       // }
 
      if (current_mode != 'L')
      {
        rtty_txbyte(LETTERS_SHIFT);
        current_mode = 'L';
      }
 
      rtty_txbyte(char_to_baudot(c, letters_arr));
    }
    else
    {
      b = char_to_baudot(c, figures_arr);
 
      if (b != 0 && current_mode != 'N')
      {
        rtty_txbyte(NUMBERS_SHIFT);
        current_mode == 'N';
      }
 
      rtty_txbyte(b);
    }
    l--;
//    txstr++;
  }
}

 
 
 
uint8_t char_to_baudot(char c, char *array)
{
  int i;
  //Serial.println(c);
  for (i = 1; i < 32; i++)
  {
    if (array[i] == c) 
      return i;
  }
 
  return 0;
}
 
void rtty_txbyte(uint8_t b)
{
  int8_t i;
  
  //Startbit 
  rtty_txbit(0);
 
  /* TODO: I don't know if baudot is MSB first or LSB first */
  /* for (i = 4; i >= 0; i--) */
  // for (i = 0; i < 5; i++)
  for (i = 0; i < 5; i++)
  {
    if (b & (1 << i))
      rtty_txbit(1);
    else
      rtty_txbit(0);
  }
  
 //2 Stopp/Bits 
  rtty_txbit(1);
  rtty_txbit(1);
}
 

 
 
 
// Transmit a bit as a mark or space
void rtty_txbit (int bit) {
  if (bit) {
    // High - mark
    tone1.play(mark);    
  }
  else {
    // Low - space
    tone1.play(space);
  }
  // 1/0,022 = 45,45 baud.
  // 1/0,020 = 50 baud.
  delay(22);
  }
