#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <MFRC522.h>

void rfidCheck(void* pvParameters);
void taskTwo(void* taskTwo);

const char *className = "11T";

const int resetPin = 22; // Reset pin
const int ssPin = 21; // Slave select pin

WiFiMulti wifiMulti;
MFRC522 mfrc522(ssPin, resetPin); // Create instance
QueueHandle_t queue;
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

	queue = xQueueCreate(10, sizeof(long));

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
