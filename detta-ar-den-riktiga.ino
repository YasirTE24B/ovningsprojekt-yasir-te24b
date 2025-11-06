/*
* Name: övningsprojekt
* Author: Victor Huke
* Date: 2025-10-8
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display, 
* Further, it measures temprature and displays a mapped value to a 9g-servo-motor
*/

// Include Libraries
#include <Wire.h>
#include <RTClib.h>
#include "U8glib.h"
#include <Servo.h>
#include "LedControl.h"  // MAX7219 library enligt instruktioner

// MAX7219 pins: DIN, CLK, CS
LedControl lc = LedControl(12, 11, 10, 1); // pinnarna, 1 för 1 enhet

Servo myservo;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

// Init constants

// Init global variables

// Construct objects
RTC_DS3231 rtc;

const int ldrPin = A0;

void setup() {
  // init communication
  Serial.begin(9600);
  Wire.begin();

  myservo.attach(9);

  rtc.begin();

  // Settings
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Initialize MAX7219  från https://www.instructables.com/MAX7219-7-Segment-Using-Arduino/
  lc.shutdown(0, false);   // Wake up display
  lc.setIntensity(0, 1);   // Medium brightness
  lc.clearDisplay(0);      // Clear all digits
}


/*
Tar värdet från ldr 0-1023 och mappar det från 0-100% men skrivet baklänges så att 100% betyder ljust och 0% betyder mörkt
*Parameters: none
*Returns: integer
*/
int getLDRProcent() {
  int rawValue = analogRead(ldrPin);
  int percent = map(rawValue, 0, 1023, 100, 0);
  return percent;
}


/*
*This function reads time from an ds3231 module and package the time as a String
*Parameters: Void
*Returns: time in hh:mm:ss as String
*/
String getTime() {
  DateTime now = rtc.now();
  return String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
}

/*
*This function reads time from an ds3231 module and package the date as a String
*Parameters: Void
*Returns: time in yyyy/mm/dd as String
*/
String getDate() {
  DateTime now = rtc.now();
  return String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
}

/*
* This function takes temprature from ds3231 and returns as a float
*Parameters: Void
*Returns: temprature as float
*/
float getTemp() {
  return rtc.getTemperature();
}

/*
* This function takes a string and draws it to an oled display
*Parameters: - text: Time string to write to display   text2: Date string to write to display
*Returns: void
*/
void oledWrite(String text, String text2) {
  // en tydlig font
  u8g.setFont(u8g_font_helvB10);

  // hämtar procent map från LDR värde från funktionen
  int ldrPercent = getLDRProcent();

  // måste omvandlas till sträng
  int temp = getTemp();
  String temp2 = String(temp) + " C";

  String procentText = String(ldrPercent) + "%";

  // målar på oled min text (gettime) text2 (date) procentText (värde från LDR som omvandlas till 0-100% mha map) och temp2 (temperatur från sträng)
  u8g.firstPage();
  do {
    u8g.drawStr(34, 32, text.c_str());
    u8g.drawStr(26, 48, text2.c_str());
    u8g.drawStr(86, 16, procentText.c_str());
    u8g.drawStr(32, 16, temp2.c_str());
  } while (u8g.nextPage());
}

/*
* takes a temprature value and maps it to corresppnding degree on a servo
*Parameters: - value: temprature
*Returns: void
*/
void servoWrite(float value) {
  myservo.write(map(value, 20, 30, 0, 179));
  delay(100);
}

// === MAX7219 displaydel ===
// Display HH MM SS on 8-digit display (taget från https://www.instructables.com/MAX7219-7-Segment-Using-Arduino/)
void displayMAX7219(DateTime now) {
  int hh = now.hour();
  int mm = now.minute();
  int ss = now.second();

  // Digits: 7=leftmost, 0=rightmost
  // VI GÖR / 10 FÖR ATT FÅ TIOTALET OCH % 10 FÖR ATT FÅ ENTALET. (13:30) ,/10 = 1 ,%10 = 3 ,/10 = 3 ,% 10 = 0
  lc.setDigit(0, 7, hh / 10, false);
  lc.setDigit(0, 6, hh % 10, false);
  lc.setDigit(0, 5, mm / 10, false);
  lc.setDigit(0, 4, mm % 10, false);
  lc.setDigit(0, 3, ss / 10, false);
  lc.setDigit(0, 2, ss % 10, false);

  // sätta dom två sista numren till falsk så att dom inte är på
  lc.setChar(0, 1, ' ', false);
  lc.setChar(0, 0, ' ', false);
}


// ===== LOOP ===== 
void loop() {
  DateTime now = rtc.now();

  // OLED & Servo
  oledWrite(getTime(), getDate());
  servoWrite(getTemp());

  // skickar rtc.now till display funktionen
  displayMAX7219(now);

  // Debug i serial monitor så att jag vet att jag inte är galen
  Serial.println(getTime());
  Serial.println(getDate());
  Serial.print(rtc.getTemperature(), 2);
  Serial.print("  LDR: ");
  Serial.print(getLDRProcent());
  Serial.println("%");

  delay(500);
}