#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>

#define RFID_SCAN 0
#define LIGHT_ON 1
#define LIGHT_OFF 2

void rfidCheck(void* pvParameters);
void taskTwo(void* taskTwo);
void sendHTTPRequest(int mode);
String httpRequestBuilder(int mode);
String UID = "";

String className = "11T";
const int resetPin = 22; // Reset pin
const int ssPin = 21; // Slave select pin

byte rfUID[4];

WiFiMulti wifiMulti;
MFRC522 mfrc522(ssPin, resetPin); // Create instance

void setup() {
	Serial.begin(112500);
	SPI.begin();
	mfrc522.PCD_Init();

	pinMode(2, OUTPUT);

	wifiMulti.addAP("DD_WRT", "NguyenThiHongTho");
	wifiMulti.addAP("NguyenDeptrai", "12345678");
	wifiMulti.addAP("third_ap", "third_ap_password");
	delay(1000);

	while (wifiMulti.run() != WL_CONNECTED) {
		Serial.println(".");
		delay(100);
	}
	Serial.print("Connected to: ");
	Serial.println(WiFi.SSID());

	delay(1000);
	mfrc522.PCD_DumpVersionToSerial();

	xTaskCreate(rfidCheck, /* Task function. */
	"rfidCheckingTask", /* String with name of task. */
	10000, /* Stack size in bytes. */
	NULL, /* Parameter passed as input of the task */
	3, /* Priority of the task. */
	NULL); /* Task handle. */

	xTaskCreate(taskTwo, /* Task function. */
	"TaskTwo", /* String with name of task. */
	10000, /* Stack size in bytes. */
	NULL, /* Parameter passed as input of the task */
	1, /* Priority of the task. */
	NULL); /* Task handle. */

}

void loop() {
	vTaskDelay(10);
}

void rfidCheck(void *pvParameters) {
	while (true) {
		vTaskDelay(10);
		if (mfrc522.PICC_IsNewCardPresent()) {
			if (mfrc522.PICC_ReadCardSerial()) {
				mfrc522.PICC_DumpToSerial(&mfrc522.uid);
				for (int i = 0; i <= 4; i++) {
					rfUID[i] = mfrc522.uid.uidByte[i];
					UID = UID + String(rfUID[i]);
				}
				sendHTTPRequest(RFID_SCAN);
			}
		}
	}
}

void taskTwo(void * parameter) {
	while (true) {
		Serial.println("Hello from task 2");
		vTaskDelay(1000);
	}
}
String httpRequestBuilder(int mode) {
	String httpRequest = "";
	switch (mode) {
	case RFID_SCAN:
		httpRequest = String(String("192.168.1.103:2705") + String("/") + String(className) + String("/") + String(UID));
		UID = "";
		break;
	case LIGHT_ON:
		httpRequest = String(String("192.168.1.103:2705") + String("/") + String(":") + String(className) + String("/ON"));
		break;
	case LIGHT_OFF:
		httpRequest = String(String("192.168.1.103:2705") + String("/") + String(":") + String(className) + String("/OFF"));
	}
	Serial.println(httpRequest);
	return httpRequest;
}
void sendHTTPRequest(int mode) {
	HTTPClient http;
	http.begin(httpRequestBuilder(mode));
	int http_code = http.GET();
	if (http_code > 0) {
		Serial.println("Error sending http GET");
	}
}
