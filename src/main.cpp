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
#define OLD_HTTP 0
#define NEW_HTTP 1
#define FROM_SEND 0
#define FROM_ANY 1

void rfidCheck(void* pvParameters);
void taskTwo(void* taskTwo);
void sendHTTPRequest(int mode);
void sendHTTPRequest_OLD(int mode);
String readHTTPResponse(int mode);

String httpRequestBuilder(int mode);
String UID = "";
const char *HOST = "112.197.235.17";
const int PORT = 2705;
String className = "11T";
const int resetPin = 22; // Reset pin
const int ssPin = 21; // Slave select pin
byte rfUID[4];

WiFiMulti wifiMulti;
MFRC522 mfrc522(ssPin, resetPin); // Create instance
WiFiClient client;
HTTPClient http;

void setup() {
	Serial.begin(112500);
	SPI.begin();
	mfrc522.PCD_Init();

	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH);

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
	30000, /* Stack size in bytes. */
	NULL, /* Parameter passed as input of the task */
	3, /* Priority of the task. */
	NULL); /* Task handle. */

	xTaskCreate(taskTwo, /* Task function. */
	"TaskTwo", /* String with name of task. */
	1000, /* Stack size in bytes. */
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
				for (int i = 0; i < 4; i++) {
					rfUID[i] = mfrc522.uid.uidByte[i];
					UID = UID + String(rfUID[i]);
				}
				Serial.println(UID);
				sendHTTPRequest_OLD(RFID_SCAN);
				mfrc522.PICC_HaltA();
				mfrc522.PCD_StopCrypto1();
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
String httpRequestBuilder(int mode, int ver) {
	String httpRequest = "";
	switch (mode) {
	case RFID_SCAN:
		if (ver == NEW_HTTP) {
			httpRequest = String(
					String("112.197.235.17:2705") + String("/") + String(":")
							+ String(className) + String("/") + String(UID));
			UID = "";
		}
		if (ver == OLD_HTTP) {
			httpRequest = String("GET ") + "/:" + className + "/" + UID
					+ " HTTP/1.1\r\n" + "Host: " + HOST + "\r\n"
					+ "Connection: close\r\n\r\n";
			UID = "";
		}
		break;
	case LIGHT_ON:
		if (ver == NEW_HTTP) {
			httpRequest = String(
					String("192.168.1.103:2705") + String("/") + String(":")
							+ String(className) + String("/ON"));
		}
		if (ver == OLD_HTTP) {
			httpRequest = String("GET ") + "/:" + className + "/" + "ON"
					+ " HTTP/1.1\r\n" + "Host: " + HOST + "\r\n"
					+ "Connection: close\r\n\r\n";
		}
		break;
	case LIGHT_OFF:
		if (ver == NEW_HTTP) {
			httpRequest = String(
					String("192.168.1.103:2705") + String("/") + String(":")
							+ String(className) + String("/OFF"));
		}
		if (ver == OLD_HTTP) {
			httpRequest = String("GET ") + "/:" + className + "/" + "OFF"
					+ " HTTP/1.1\r\n" + "Host: " + HOST + "\r\n"
					+ "Connection: close\r\n\r\n";
		}
	}
	Serial.println(httpRequest);
	return httpRequest;
}
void sendHTTPRequest(int mode) {

	http.begin(httpRequestBuilder(mode));
	int http_code = http.GET();
	if (http_code > 0) {
		Serial.println("Error sending http GET");
		String payload = http.getString();
		Serial.println(payload);
	}
	http.end();
}
void sendHTTPRequest_OLD(int mode) {
	String line;
	if (!client.connect(HOST, PORT)) {
		Serial.println("Connect to host failed");
	}
	client.print(httpRequestBuilder(mode, OLD_HTTP));
	Serial.println();
	if (readHTTPResponse(FROM_SEND).indexOf("OK") != -1) {
		digitalWrite(2, HIGH);
	} else {
		digitalWrite(2, LOW);
	}
}
String readHTTPResponse(int mode) {
	String httpResponse = "";
	if (mode == FROM_SEND) {
		while (client.available() == 0) {
			unsigned long timeOut = millis();
			if (millis() - timeOut > 5000) {
				Serial.println("NO HTTP RESPONSE RECIEVED ON TIME");
			}
		}
		while (client.available()) {
			String line = client.readStringUntil('\r');
			Serial.print(line);
			httpResponse = line;
		}
	}
	if (mode == FROM_ANY) {
		while (client.available()) {
			String line = client.readStringUntil('\r');
			Serial.println(line);
			httpResponse = line;
		}
	}
	return httpResponse;
}
