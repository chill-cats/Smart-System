/*
 * main.cpp
 *
 *  Created on: Dec 20, 2018
 *      Author: nhan
 */
#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>

const char* SSID = "HELLO";
const char* PASS = "HI";

String hello = "Hello";
String *hi = &hello;

void setup() {
	Serial.begin(115200);

}
void loop() {
	Serial.println(hello);
	Serial.println("");
	Serial.println(*hi);
}



