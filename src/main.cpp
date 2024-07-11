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
//const char *HOST = "https://d1-tutorial.rei512-mc.workers.dev/";
const char *HOST = "www.howsmyssl.com";

const char* test_root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n" \
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n" \
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n" \
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n" \
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n" \
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n" \
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n" \
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n" \
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n" \
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n" \
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n" \
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n" \
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n" \
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n" \
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n" \
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n" \
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n" \
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n" \
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n" \
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n" \
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n" \
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n" \
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n" \
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n" \
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n" \
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n" \
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n" \
"-----END CERTIFICATE-----\n";

AsyncWebServer Server(80);         //  ポート番号（HTTP）
WiFiClientSecure client;
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
		client.println("GET https://www.howsmyssl.com/a/check HTTP/1.0");
		client.println("Host: www.howsmyssl.com");
		client.println("Connection: close");
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
	}

	Serial.print("Initializing WebServer...\n");

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
	delay(200);
}