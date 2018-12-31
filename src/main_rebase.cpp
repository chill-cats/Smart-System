/*
 * main_rebase.cpp
 *
 *  Created on: Jan 1, 2019
 *      Author: nhan
 */
// Include libraries to use
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>

//Function declaration
void RFIDCheckTask(void *pvParameters);
void sendHTTPRequest();
void blockRFIDReader(int blockTime);
void blockRFIDTask(void *pvParameters);
void setup();
void loop();

String readHTTPReponse(int delayTime);
String HTTPResponseHandler(String HTTPResponse);
String HTTPRequestBuilder();

// Constant Variable Declaration
const String serverIP = "112.197.235.17";
const String className = "11T";
const String UUID = "0e47f9cf-ac45-46a8-baeb-c5ec048ceec3";
const int serverPORT = 2705;
const int MFRC522RstPin = 22;
const int MFRC522SsPin = 21;

// Normal Variable Declaration
byte rfUID[4];
int FLAG_ISBLOCKED = 0;

WiFiMulti wifiMulti;
MFRC522 mfrc522(MFRC522SsPin, MFRC522RstPin);
WiFiClient wifiClient;
