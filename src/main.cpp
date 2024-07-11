#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <stdlib.h>
#include <stdio.h>

Adafruit_BME280 bme;
File file;
#define JST     3600* 9

#define SDA 14
#define SCL 26

const char *SSID = "TP-Link_2A30";
const char *PASSWORD = "48251969";
const char *HOST = "http2d1telemetry.deltav-lab.workers.dev";
//const char *HOST = "www.howsmyssl.com";

const char* test_root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n" \
"VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n" \
"A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n" \
"WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n" \
"IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n" \
"AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n" \
"QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n" \
"HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n" \
"BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n" \
"9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n" \
"p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n" \
"-----END CERTIFICATE-----\n" ;

AsyncWebServer Server(80);         //  ポート番号（HTTP）
WiFiClientSecure client;
const char *NTP1 = "ntp.nict.jp";
const char *NTP2 = "ntp.jst.mfeed.ad.jp";

static const char *date[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
unsigned char count;
char str[150];
char fileName[32];

unsigned char status;

time_t t;
struct tm *tm;

double temp;
double pressure;
double humid;

String processor(const String& var) {
	Serial.println(var);
	if (var == "TEMP") {
		return String(temp);
	} else if(var == "PRESSURE") {
		return String(pressure);
	} else if(var == "HUMID") {
		return String(humid);
	} else {
		return String();
	}
}

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	delay(50);
	while (!Serial){
	}

	Serial.print("\n\nInitilization is Start\n");

	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);
	Wire.begin(SDA, SCL);

	while (!bme.begin(0x76)) {
		Serial.print("BME280 sensor is unavailable\n");
		delay(1000);
	}
	Serial.print("BME280 sensor is Available\n");

	/*
	while(!SD.begin()){
		Serial.print("Card Mount Failed\n");
		delay(2000);
	}
	Serial.print("Card is Mounted\n");
	*/



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
		Serial.printf("%04d/%02d/%02d(%s) %02d:%02d:%02d (UTC+09)\n",
				tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
				date[tm->tm_wday],
				tm->tm_hour, tm->tm_min, tm->tm_sec);
	} else {
		Serial.print("NTP Server is Timeout. Set a Tentative Date of 1970/1/1 00:00:00 (UTC+00)\n");
	}

	jump:

	client.setCACert(test_root_ca);
  //client.setCertificate(test_client_cert); // for client verification
  //client.setPrivateKey(test_client_key);	// for client verification

	Serial.println("\nStarting connection to server...");

	if (!client.connect(HOST, 443)) {
		Serial.println("Connection failed!");
	} else {
		Serial.println("Connected to server!");
		// Make a HTTP request:
		sprintf(str, "GET https://http2d1telemetry.deltav-lab.workers.dev/api/howmyssl  HTTP/1.0");
		client.println(str);
		Serial.println(str);
		sprintf(str, "Host: http2d1telemetry.deltav-lab.workers.dev");
		client.println(str);
		Serial.println(str);
		sprintf(str, "Connection: close");
		client.println(str);
		Serial.println(str);
		client.println();

		while (client.connected()) {
			String line = client.readStringUntil('\n');
			if (line == "\r") {
				Serial.println("headers received");
				break;
			}
		}
		// if there are incoming bytes available
		// from the server, read them and print them:
		while (client.available()) {
			char c = client.read();
			Serial.write(c);
		}
	    client.stop();
		Serial.println("\nClient stop");
	}

	Serial.print("\nInitializing WebServer...\n");

	// SPIFFSの初期化とファイル存在確認
	if(!SPIFFS.begin(true)){
		Serial.println("An Error has occurred while mounting SPIFFS\nServer initialization is Incomplete");
		goto finish;
	}
	if (!SPIFFS.exists("/index.html")) {
		Serial.println("/index.html does not exist\nServer initialization is Incomplete");
		goto finish;
	}
	if (!SPIFFS.exists("/style.css")) {
		Serial.println("/style.css does not exist\nServer initialization is Incomplete");
		goto finish;
	}

	Server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
		request->send(SPIFFS, "/index.html", String(), false, processor);
	});

	Server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/index.html");
	});
	// style.cssにアクセスされた時のレスポンス
	Server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/style.css", "text/css");
	});
	//tcpip_init(NULL, NULL);

	Serial.printf("Available heap: %d\n", ESP.getFreeHeap());

	tcpip_adapter_init();
	
	Server.begin();

	Serial.println("Server start!");

finish:

	Serial.print("\nInitialization is completed.\n\n");
	Serial.print("Day\tTime\tTemperature [degC]\tPressure [hPa]\tHumidity [\%RH]\n");

	count = 0;
}

void loop() {
	// put your main code here, to run repeatedly:
	t = time(NULL);
	tm = localtime(&t);

	/*
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
	*/

	temp=bme.readTemperature();
	pressure=bme.readPressure() / 100.0F;
	humid=bme.readHumidity();

	sprintf(str, "%02d-%02d-%02d\t%02d:%02d:%02d\t%.2lf\t%.2lf\t%.2lf\n", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, temp, pressure, humid);
	//Serial.print("Writing: ");
	Serial.print(str);

	Serial.println("\nStarting connection to server...");

	if (!client.connect(HOST, 443)) {
		Serial.println("Connection failed!");
	} else {
		Serial.println("Connected to server!");
		// Make a HTTP request:
		sprintf(str, "GET https://http2d1telemetry.deltav-lab.workers.dev/api/insert?time=%d&temp=%.2lf&pres=%.2lf&humi=%.2lf  HTTP/1.0", t, temp, pressure, humid);
		client.println(str);
		Serial.println(str);
		sprintf(str, "Host: http2d1telemetry.deltav-lab.workers.dev");
		client.println(str);
		Serial.println(str);
		sprintf(str, "Connection: close");
		client.println(str);
		Serial.println(str);
		client.println();

		while (client.connected()) {
			String line = client.readStringUntil('\n');
			if (line == "\r") {
				Serial.println("headers received");
				break;
			}
		}
		// if there are incoming bytes available
		// from the server, read them and print them:
		while (client.available()) {
			char c = client.read();
			Serial.write(c);
		}
	    client.stop();
		Serial.println("\nClient stop");
	}
	/*
	file = SD.open(fileName, FILE_APPEND);
	status = file.print(str);
	if(!status){
		Serial.print("!!!!Write failed!!!!\n");
	}
	//Serial.print("File closed: ");
	//Serial.println(fileName);
	file.close();
	*/
	delay(10000);
}