#include <Wire.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include <RTClib.h>
#include <SD.h>

#define TIME_PAR_FILE 60 // minutes
#define CTIME_STRING_BUFFER 20

// Akizuki LCD
// 1:VDD
// 2:VSS
// 3:VO contrast
// 4:RS
// 5:R/W
// 6:E
// 7-14:DB0-DB17
//
// LiquidCrystal(rs, enable, d4, d5, d6, d7)
// LiquidCrystal(rs, rw, enable, d4, d5, d6, d7)
// LiquidCrystal(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7)
// LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7)
LiquidCrystal lcd(7,6,5,4,3,2);

// Switch
int swSet = 8;
int swSelect = 9;

// Sensor
int sensor0 = 0;
int sensor1 = 1;

uint16_t year = 2015;
uint8_t month = 12;
uint8_t day = 15;
uint8_t hour = 12;
uint8_t minute = 00;
const int chipSelect = 10;

RTC_DS1307 RTC;

//=====================================
// initialize
//=====================================
void setup() {
  byte tm[7];
  char date[20];

  pinMode(swSet, INPUT_PULLUP);
  pinMode(swSelect, INPUT_PULLUP);

  // initialize LCD
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  
  // initialize serial
  Serial.begin(9600);
  while (!Serial);
  
  // initialize RTC
  Serial.print(F("Initializing RTC..."));
  Wire.begin();
  RTC.begin();
  if (!RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__,__TIME__));
  }
  Serial.println(F("ok."));
  DateTime now = RTC.now();
  sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());
  Serial.println(date);

  // initialize SD card
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or no present"));
    fatalError("SD card failed  ");
  }
  SdFile::dateTimeCallback( &dateTime );
  Serial.println(F("ok."));

  int yes;
  int no;
  while (true) {
    now = RTC.now();
    sprintf(date, "%d-%02d-%02d %02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute());
    lcd.setCursor(0,0);
    lcd.print(date);
    lcd.setCursor(0,1);
    lcd.print(F("collect? [Y:red]"));

    int i;
    for (i = 0; i < 20; i++) {
      yes = digitalRead(swSet);
      no  = digitalRead(swSelect);
      delay(50);
      if (yes == 0 || no == 0) break;
    }
    if (no == 0) {
      lcd.setCursor(0,1);
      lcd.print(F("set date time..."));
      delay(2000);
      lcd.clear();
      while (digitalRead(swSet) == 0 || digitalRead(swSelect) == 0);
      adjustDateTime();
      lcd.setCursor(0,1);
      lcd.print(F("setting...      "));
      delay(1000);
      while (digitalRead(swSet) == 0 || digitalRead(swSelect) == 0);
    }
    if (yes == 0) {
      lcd.setCursor(0,1);
      lcd.print(F("please wait...  "));
      delay(2000);
      lcd.clear();
      while (digitalRead(swSet) == 0 || digitalRead(swSelect) == 0);
      break;
    }
  }
}

//============================================
// main loop
//============================================
void loop() {
  char date[20];
  char filename[24];
  long i;

  // display voltage
  while (true) {
    DateTime now = RTC.now();
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());
    lcd.setCursor(0,0);
    lcd.print(date);
    lcd.setCursor(0,1);
    float s0 = (float)analogRead(sensor0)*5.0/1023.0;
    float s1 = (float)analogRead(sensor1)*5.0/1023.0;
    lcd.print(s0,2);
    lcd.print(F(" "));
    lcd.print(s1,2);
    lcd.print(F("  [red]"));
    for (i = 0; i < 400000; i++) {
      if (digitalRead(swSet) == 0) break;
    }
    if (i != 400000) break;
  }
  delay(200);
  while (digitalRead(swSet) == 0);

  // logging
  while (true) {
    DateTime now = RTC.now();
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());
    sprintf(filename, "%02d%02d%02d%02d.CSV",
            now.day(), now.hour(), now.minute(), now.second());
    Serial.print(date);
    Serial.print(' ');
    Serial.println(filename);
    lcd.setCursor(0,1);
    lcd.print(date);
    int n = getData(TIME_PAR_FILE, filename);
    if (n < 0) {
      lcd.setCursor(0,1);
      fatalError("file failed     ");
    }
    if (n == 0) {
      Serial.println(F("stop"));
      lcd.setCursor(0,1);
      lcd.print(F("stop            "));
      while (digitalRead(swSet) == 0);
      break;
    }
  }
}

//============================================
// abortLogging()
//============================================
#define ABORT_LIMIT 200
int abortDelay = ABORT_LIMIT;
bool abortLogging()
{
  if (digitalRead(swSet) == 0) {
    abortDelay--;
    if (abortDelay == 0) {
      abortDelay = ABORT_LIMIT;
      return true;
    }
  }
  else {
    abortDelay = ABORT_LIMIT;
  }

  return false;
}

//============================================
// getData()
//============================================
int getData(int min_par_file, char *filename)
{
  int j;
  long num = 0;
  char date[20];
  int pre_min = 70;
  byte sec = 0;
  
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    while (true) {
      int s0 = analogRead(sensor0);
      int s1 = analogRead(sensor1);

      DateTime now = RTC.now();
      int cur_min = now.minute();
      if (pre_min == 70) {
        pre_min = cur_min;
        sec = now.second();
      }
      if (cur_min != pre_min && sec == now.second()) {
        pre_min = cur_min;
        min_par_file--;
        if (min_par_file == 0) break;
      }
      sprintf(date, "%d-%02d-%02dT%02d:%02d:%02d",
              now.year(), now.month(), now.day(),
              now.hour(), now.minute(), now.second());
      dataFile.print(date);
      dataFile.print(",");
      dataFile.print(num);
      num++;
      dataFile.print(",");
      dataFile.print(s0);
      dataFile.print(",");
      dataFile.println(s1);
      if (abortLogging()) {
        dataFile.close();
        return 0;
      }
      delay(17);
    }
    dataFile.close();
  }
  else {
    Serial.print(F("error opening "));
    Serial.println(filename);
    fatalError("can't open file ");
    return -1;
  }
  return 1;
}

//============================================
// adjustDateTime()
//============================================
void adjustDateTime()
{
  char date[32];
  int dt = 500;
  int cnt = 5;

  DateTime now = RTC.now();
  year = now.year();
  month = now.month();
  day = now.day();
  hour = now.hour();
  minute = now.minute();
  
  sprintf(date, "%d-%02d-%02d %02d:%02d", year, month, day, hour, minute);
  lcd.cursor();
  lcd.home();
  lcd.print(date);

  // adjust year
  dt = 500;
  cnt = 5;
  while (true) {
    lcd.setCursor(3,0);
    if (digitalRead(swSet) == 0) break;
    if (digitalRead(swSelect) == 0) {
      year++;
      lcd.setCursor(0,0);
      lcd.print(year);
      lcd.setCursor(3,0);
      delay(dt);
      cnt--;
      if (cnt == 0) dt = 100;  // fast
    }
    else {
      cnt = 5;
      dt = 500; // slow
    }
  }
  delay(150);
  while (digitalRead(swSet) == 0);

  // adjust month
  cnt = 5;
  dt = 500;
  while (true) {
    lcd.setCursor(6,0);
    if (digitalRead(swSet) == 0) break;
    if (digitalRead(swSelect) == 0) {
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
  while (digitalRead(swSet) == 0);

  // adust day
  cnt = 5;
  dt = 500;
  while (true) {
    lcd.setCursor(9,0);
    if (digitalRead(swSet) == 0) break;
    if (digitalRead(swSelect) == 0) {
      day++;
      if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day == 31) day = 1;
      }
      else if (month == 2) {
        int y = year;
        // check leap year 
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
  while (digitalRead(swSet) == 0);

  // adjust hour
  cnt = 5;
  dt = 500;
  while (1) {
    lcd.setCursor(12,0);
    if (digitalRead(swSet) == 0) break;
    if (digitalRead(swSelect) == 0) {
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
  while (digitalRead(swSet) == 0);

  // adjust minute
  cnt = 5;
  dt = 500;
  while (1) {
    lcd.setCursor(15,0);
    if (digitalRead(swSet) == 0) break;
    if (digitalRead(swSelect) == 0) {
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
  RTC.adjust(DateTime(year, month, day, hour, minute, 0));
  lcd.noCursor();
}

//============================================
// fatalError()
//============================================
void fatalError(const char *message)
{
  char date[32];

  DateTime now = RTC.now();
  sprintf(date, "%d-%02d-%02dT%02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());
  lcd.setCursor(0,0);
  lcd.print(date);
  lcd.setCursor(0,1);
  lcd.print(message);
  while (true) {
    lcd.noDisplay();
    delay(500);
    lcd.display();
    delay(500);
  }
}

//============================================
// dateTime()
// for SD card time stamp
//============================================
void dateTime(uint16_t* date, uint16_t* time)
{
  DateTime now = RTC.now();
  *date = FAT_DATE(now.year(), now.month(), now.day());
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
