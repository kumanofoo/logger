#include <SD.h>
#include <Wire.h>
#include <stdio.h>

const int chipSelect = 10;

void setup() {
  byte tm[7];
  char date[20];
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

  rtc();
  getRTC(tm);
  getCTime(date, tm);
  Serial.println(date);
  
  Serial.print(F("Initializing SD card..."));

  pinMode(10, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or no present"));
    while (1);
  }
  SdFile::dateTimeCallback( &dateTime );
  Serial.println(F("ok."));
}

void getCTime(char c[20], byte tm[7])
{
  char t[7];
  for (int i = 0; i < 7; i++) {
    t[i] = bcd2bin(tm[i]);
  }
  sprintf(c, "%04d-%02d-%02dT%02d:%02d:%02d", t[6]+2000, t[5], t[3], t[2], t[1], t[0]);  
}

void getCTime2(char c[20], byte tm[7])
{
  char t[7];
  for (int i = 0; i < 7; i++) {
    t[i] = bcd2bin(tm[i]);
  }
  sprintf(c, "%02d%02d%02d%02d", t[3], t[2], t[1], t[0]);  
}

void loop() {
  byte tm[7];
  char date[20];
  char date2[20];
  char filename[24];
  for (int i = 0; i < 15; i++) {
    getRTC(tm);
    getCTime(date, tm);
    getCTime2(date2, tm);
    sprintf(filename, "%s.txt", date2);
    Serial.print(date);
    Serial.print(':');
    Serial.println(filename);
    getData(60*60, filename);     
  }
  Serial.println("done!");
  while (1);
}

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
      //if (reg2 & 0x80*0) {  // if VL bit is on, initialize date
      if (1) {
        Wire.beginTransmission(RTC_ADRS);
        Wire.write(0x00); // address 00h
        Wire.write(0x20); // control1: TEST=0, STOP=1
        Wire.write(0x11); // control2: disable interrupt
        Wire.write((byte)bin2bcd(00));  // seconds
        Wire.write((byte)bin2bcd(38));  // minutes
        Wire.write((byte)bin2bcd(17));  // hours
        Wire.write((byte)bin2bcd(7));  // days
        Wire.write((byte)bin2bcd(1));  // week days
        Wire.write((byte)bin2bcd(12));  // months
        Wire.write((byte)bin2bcd(15));  // years
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


