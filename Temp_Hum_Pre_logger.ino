#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
File file;
#define JST     3600* 9

const char *SSID = "TP-Link_2A30";
const char *PASSWORD = "48251969";

const char *NTP1 = "ntp.nict.jp";
const char *NTP2 = "ntp.jst.mfeed.ad.jp";

static const char *date[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
unsigned char count;
char str[64];
char fileName[32];

unsigned char status;

time_t t;
struct tm *tm;

double temp;
double pressure;
double humid;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(50);
  while (!Serial){
  }

  Serial.print("\n\nInitilization is Start\n");

  while (!bme.begin(0x76)) {
    Serial.print("BME280 sensor is unavailable\n");
    delay(1000);
  }
  Serial.print("BME280 sensor is Available\n");

  while(!SD.begin()){
    Serial.print("Card Mount Failed\n");
    delay(2000);
  }
  Serial.print("Card is Mounted\n");



  Serial.print("Wi-Fi connection is Start");
  WiFi.begin(SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    if(count++ > 200) {
      Serial.print("Wi-Fi connection is failure\n");
      configTime(JST, 0, NULL);
      goto jump;
    }
    delay(100);
  }

  Serial.print("done\n");
  Serial.printf("Wi-Fi is Connected, IP address: ");
  Serial.println(WiFi.localIP());
  configTime(JST, 0, NTP1, NTP2);
  t = time(NULL);
  tm = localtime(&t);
  Serial.print("Getting RealTime for NTP Server");
  count = 0;
  while(tm->tm_year < 71 && count++ < 200) {
      t = time(NULL);
      tm = localtime(&t);
      Serial.print(".");
      delay(100);
  }
  if(count < 999) {
    Serial.print("done\nNTP Server is Reached. Now RealTime is ");
    Serial.printf("%04d/%02d/%02d(%s) %02d:%02d:%02d (UTC+09)",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        date[tm->tm_wday],
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  } else {
    Serial.print("NTP Server is Timeout. Set a Tentative Date of 1970/1/1 00:00:00 (UTC+00)");
  }

  jump:

  Serial.print("\nInitialization is completed.\n\n");

  count = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  t = time(NULL);
  tm = localtime(&t);
  if(count != tm->tm_mday) {
    Serial.print("File Opening: ");
    sprintf(fileName, "/%04d-%02d-%02d.csv", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
    Serial.println(fileName);
    file = SD.open(fileName, FILE_APPEND);

    while(!file){
      Serial.print("Failed to open file for Appending\n");
      delay(2000);
    }
    Serial.print("done\n\n");
    count = tm->tm_mday;
  }
  temp=bme.readTemperature();
  pressure=bme.readPressure() / 100.0F;
  humid=bme.readHumidity();

  sprintf(str, "%02d:%02d:%02d\t%lf\t%lf\t%lf\n", tm->tm_hour, tm->tm_min, tm->tm_sec, temp, pressure, humid);
  //Serial.print("Writing: ");
  Serial.print(str);
  file = SD.open(fileName, FILE_APPEND);
  status = file.print(str);
  if(!status){
    Serial.print("!!!!Write failed!!!!\n");
  }
  //Serial.print("File closed: ");
  //Serial.println(fileName);
  file.close();
  delay(200);
}
