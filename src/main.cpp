/*
 * main.cpp
 *
 *  Created on: Dec 20, 2018
 *      Author: nhan
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <freertos/FreeRTOS.h>

WiFiMulti wifiMulti;

void setup() {
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);

	wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
	wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
	wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

	Serial.println("Connecting Wifi...");
	if (wifiMulti.run() == WL_CONNECTED) {
		Serial.println("");
		Serial.println("WiFi connected");
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
	}
}
void loop() {
}

