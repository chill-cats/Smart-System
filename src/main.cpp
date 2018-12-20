#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>

void taskOne(void* taskOne);
void taskTwo(void* taskTwo);

WiFiMulti wifiMulti;

void setup() {
	Serial.begin(112500);
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



	xTaskCreate(taskOne, /* Task function. */
				"TaskOne", /* String with name of task. */
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

void taskOne(void * parameter) {
	while (true) {
		digitalWrite(2, HIGH);
		vTaskDelay(1000);
		digitalWrite(2, LOW);
		vTaskDelay(1000);
	}
}

void taskTwo(void * parameter) {
	while (true) {
		Serial.println("Hello from task 2");
		vTaskDelay(1000);
	}
}
