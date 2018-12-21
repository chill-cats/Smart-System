#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <MFRC522.h>

void rfidCheck(void* taskOne);
void taskTwo(void* taskTwo);

#define SS_PIN 5
#define RST_PIN 22

WiFiMulti wifiMulti;
MFRC522 mfrc522(SS_PIN, RST_PIN);
QueueHandle_t queue;
void setup() {
	Serial.begin(112500);
	SPI.begin();
	mfrc522.PCD_Init();

	pinMode(2, OUTPUT);

	wifiMulti.addAP("DD_WRT", "NguyenThiHongTho");
	wifiMulti.addAP("second_ap", "second_ap_password");
	wifiMulti.addAP("third_ap", "third_ap_password");
	delay(1000);

	while(wifiMulti.run() != WL_CONNECTED) {
		Serial.println(".");
		delay(100);
	}
	Serial.print("Connected to: ");
	Serial.println(WiFi.SSID());

	delay(1000);

	queue = xQueueCreate(10, sizeof(long));

	xTaskCreate(rfidCheck, /* Task function. */
				"rfidCheckingTask", /* String with name of task. */
				10000, /* Stack size in bytes. */
				NULL, /* Parameter passed as input of the task */
				1, /* Priority of the task. */
				NULL); /* Task handle. */

	xTaskCreate(taskTwo, /* Task function. */
				"TaskTwo", /* String with name of task. */
				10000, /* Stack size in bytes. */
				NULL, /* Parameter passed as input of the task */
				1, /* Priority of the task. */
				NULL); /* Task handle. */

}

void loop() {
}

void rfidCheck(void * parameter) {

}

void taskTwo(void * parameter) {
	while (true) {
		Serial.println("Hello from task 2");
		vTaskDelay(1000);
	}
}
