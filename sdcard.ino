#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <stdio.h>

// Akizuki LCD
// 1:VDD
// 2:VSS
// 3:VO contrast
// 4:RS
// 5:R/W
// 6:E
// 7-14:DB0-DB17

// LiquidCrystal(rs, enable, d4, d5, d6, d7)
// LiquidCrystal(rs, rw, enable, d4, d5, d6, d7)
// LiquidCrystal(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7)
// LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7) 
LiquidCrystal lcd(7,6,5,4,3,2);

int year = 15;
int month = 12;
int day = 15;
int hour = 12;
int minute = 00;
const int chipSelect = 10;

//=====================================
// initialize
//=====================================
void setup() {
  byte tm[7];
  char date[20];

  //pinMode(8, INPUT);
  //pinMode(9, INPUT);
  
  // initialize LCD
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  
  // initialize serial
  Serial.begin(9600);
  while (!Serial);

  // initialize RTC
  rtc();
  getRTC(tm);
  getCTime(date, tm);
  Serial.println(date);
  lcd.print(date);
  lcd.setCursor(0,1);

  // initialize SD card
  Serial.print(F("Initializing SD card..."));
  lcd.print(F("SD card..."));
  pinMode(10, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or no present"));
    lcd.print(F("failed!"));
    while (1) {
      lcd.noDisplay();
      delay(500);
      lcd.display();
      delay(500);
    }
  }
  SdFile::dateTimeCallback( &dateTime );
  Serial.println(F("ok."));
  lcd.print(F("ok."));
}

//============================================
// main loop
//============================================
void loop() {
  byte tm[7];
  char date[20];
  char date2[20];
  char filename[24];

  while (1) {
    if (digitalRead(8) == 0) break;
    getRTC(tm);
    getCTime(date, tm);
    lcd.home();
    lcd.print(date);
    delay(1000);
  }
  lcd.setCursor(0,1);
  lcd.print(F("Logging...      "));
  while (1);
  
  for (int i = 0; i < 6; i++) {
    getRTC(tm);
    getCTime(date, tm);
    getCTime2(date2, tm);
    sprintf(filename, "%s.txt", date2);
    Serial.print(date);
    Serial.print(':');
    Serial.println(filename);
    getData(10, filename);     
  }
  Serial.println("done!");
  while (1);
}

//============================================
// setTime()
//============================================

void setTime()
{
  char date[32];
  int dt = 500;
  int cnt = 5;

  lcd.cursor();
  sprintf(date, "%04d-%02d-%02d %02d:%02d", year+2000, month, day, hour, minute);
  lcd.home();
  lcd.print(date);

  // year
  dt = 500;
  cnt = 5;
  while (1) {
    lcd.setCursor(3,0);
    if (digitalRead(8) == 0) break;
    if (digitalRead(9) == 0) {
      year++;
      lcd.setCursor(0,0);
      lcd.print(year+2000);
      lcd.setCursor(3,0);
      delay(dt);
      cnt--;
      if (cnt == 0) dt = 100;
    }
    else {
      cnt = 5;
      dt = 500;
    }
  }
  delay(150);
  while (digitalRead(8) == 0);

  // month
  cnt = 5;
  dt = 500;
  while (1) {
    lcd.setCursor(6,0);
    if (digitalRead(8) == 0) break;
    if (digitalRead(9) == 0) {
      month++;
      if (month == 13) month = 1;
      lcd.setCursor(5,0);
      if (month < 10) {
        lcd.print(0);
      }
      lcd.print(month);
      lcd.setCursor(6,0);
      delay(dt);
      cnt--;
      if (cnt == 0) dt = 100;
    }
    else {
      cnt = 5;
      dt = 500;
    }
  }
  delay(150);
  while (digitalRead(8) == 0);

  // day
  cnt = 5;
  dt = 500;
  while (1) {
    lcd.setCursor(9,0);
    if (digitalRead(8) == 0) break;
    if (digitalRead(9) == 0) {
      day++;
      if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day == 31) day = 1;
      }
      else if (month == 2) {
        int y = year+2000;
        if ((y % 4) == 0 && (y % 100) != 0 && (y % 400) == 0) {
          if (day == 30) day = 1; 
        }
        else {
          if (day == 29) day = 1;
        }
      }
      else {
        if (day == 32) day = 1;
      }
      
      lcd.setCursor(8,0);
      if (day < 10) {
        lcd.print(0);
      }
      lcd.print(day);
      lcd.setCursor(9,0);
      delay(dt);
      cnt--;
      if (cnt == 0) dt = 100;
    }
    else {
      cnt = 5;
      dt = 500;
    }
  }
  delay(150);
  while (digitalRead(8) == 0);

  // hour
  cnt = 5;
  dt = 500;
  while (1) {
    lcd.setCursor(12,0);
    if (digitalRead(8) == 0) break;
    if (digitalRead(9) == 0) {
      hour++;
      if (hour == 24) hour = 0;
      lcd.setCursor(11,0);
      if (hour < 10) {
        lcd.print(0);
      }
      lcd.print(hour);
      lcd.setCursor(12,0);
      delay(dt);
      cnt--;
      if (cnt == 0) dt = 100;
    }
    else {
      cnt = 5;
      dt = 500;
    }
  }
  delay(150);
  while (digitalRead(8) == 0);

  // minute
  cnt = 5;
  dt = 500;
  while (1) {
    lcd.setCursor(15,0);
    if (digitalRead(8) == 0) break;
    if (digitalRead(9) == 0) {
      minute++;
      if (minute == 60) minute = 0;
      lcd.setCursor(14,0);
      if (minute < 10) {
        lcd.print(0);
      }
      lcd.print(minute);
      lcd.setCursor(15,0);
      delay(dt);
      cnt--;
      if (cnt == 0) dt = 100;
    }
    else {
      cnt = 5;
      dt = 500;
    }
  }
}

//============================================
// getCTime()
//============================================
void getCTime(char c[20], byte tm[7])
{
  char t[7];
  for (int i = 0; i < 7; i++) {
    t[i] = bcd2bin(tm[i]);
  }
  sprintf(c, "%04d-%02d-%02dT%02d:%02d:%02d", t[6]+2000, t[5], t[3], t[2], t[1], t[0]);  
}

//============================================
// getCTime2()
//============================================
void getCTime2(char c[20], byte tm[7])
{
  char t[7];
  for (int i = 0; i < 7; i++) {
    t[i] = bcd2bin(tm[i]);
  }
  sprintf(c, "%02d%02d%02d%02d", t[3], t[2], t[1], t[0]);  
}

//============================================
// getData()
//============================================
void getData(int n, char *filename)
{
  long num = 0;
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    for (int j = 0; j < n; j++) {
      // about 1 sec loop
      for (int i = 0; i < 100 ; i++) {
        int value1 = analogRead(0);
        int value2 = analogRead(1);
        dataFile.print(num);
        num++;
        dataFile.print(",");
        dataFile.print(value1);
        dataFile.print(",");
        dataFile.println(value2);
        delay(9);
      }
    }
    dataFile.close();
  }
  else {
    Serial.print(F("error opening "));
    Serial.println(filename);
  }
}

unsigned int bcd2bin(byte dt)
{
  return ((dt >> 4) * 10) + (dt & 0x0f);
}

unsigned int bin2bcd(unsigned int num)
{
  return ((num / 100) << 8) | (((num % 100) / 10) << 4) | (num % 10); 
}


//===============================================
// RTC()
//===============================================
#define RTC_ADRS 0B1010001
void rtc()
{
  byte reg1, reg2;
  
  Wire.begin();
  delay(1000);
  Wire.beginTransmission(RTC_ADRS);
  Wire.write(0x01); // register address 01h
  
  int ans = Wire.endTransmission();
  if (ans == 0) {
    ans = Wire.requestFrom(RTC_ADRS, 2);
    if (ans == 2) {
      reg1 = Wire.read(); // receive Reg 01H
      reg2 = Wire.read(); // receive Reg 02H
      if (reg2 & 0x80) {  // if VL bit is on, initialize date
        setTime();
        Wire.beginTransmission(RTC_ADRS);
        Wire.write(0x00); // address 00h
        Wire.write(0x20); // control1: TEST=0, STOP=1
        Wire.write(0x11); // control2: disable interrupt
        Wire.write((byte)bin2bcd(00));  // seconds
        Wire.write((byte)bin2bcd(minute));  // minutes
        Wire.write((byte)bin2bcd(hour));  // hours
        Wire.write((byte)bin2bcd(day));  // days
        Wire.write((byte)bin2bcd(1));  // week days
        Wire.write((byte)bin2bcd(month));  // months
        Wire.write((byte)bin2bcd(year));  // years
        Wire.write(0x80); // alarm minute
        Wire.write(0x80); // alarm hour
        Wire.write(0x80); // alarm hour
        Wire.write(0x80); // alarm week day
        Wire.write(0x83); // CLKOUT 1Hz
        Wire.write(0x00); // timer control
        Wire.write(0x00); // timer
        ans = Wire.endTransmission();
        if (ans == 0) {
          // start counting time
          Wire.beginTransmission(RTC_ADRS);
          Wire.write(0x00);
          Wire.write(0x00);
          ans = Wire.endTransmission();
          delay(1000);
        }
        else {
          Serial.println("initialized RTC failed");
          while (1);
        }
      }
    }
  }
}

void getRTC(byte tm[7])
{
  Wire.beginTransmission(RTC_ADRS);
  Wire.write(0x02);
  byte ans = Wire.endTransmission();
  if (ans == 0) {
    ans = Wire.requestFrom(RTC_ADRS, 7);
    if (ans == 7) {
      tm[0] = Wire.read() & 0x7f;  // second
      tm[1] = Wire.read() & 0x7f;  // minute
      tm[2] = Wire.read() & 0x3f;  // hour
      tm[3] = Wire.read() & 0x3f;  // day
      tm[4] = Wire.read() & 0x07;  // week day
      tm[5] = Wire.read() & 0x1f;  // month
      tm[6] = Wire.read();  // year
    }
  }
}

void dateTime(uint16_t* date, uint16_t* time)
{
  uint8_t t[7];
  uint16_t year;
  uint8_t month, day, hour, minute, second;
  byte tm[7];
  
  getRTC(tm);
  for (int i = 0; i < 7; i++) {
    t[i] = (uint8_t)bcd2bin(tm[i]);
  }
  year = t[6] + 2000;
  
  *date = FAT_DATE(year, t[5], t[3]);
  *time = FAT_TIME(t[2], t[1], t[0]);
}


