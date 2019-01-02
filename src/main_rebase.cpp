/*
 * main_rebase.cpp
 *
 *  Created on: Jan 1, 2019
 *      Author: nhan
 */
// Include libraries to use
#include <Arduino.h>                                               // core library for everything
#include <freertos/FreeRTOS.h>                                     // freeRTOS core library
#include <freertos/task.h>                                         // freeRTOS library for task managing
#include <WiFi.h>                                                  // core Library for establish WiFi connection
#include <WiFiMulti.h>                                             // library for easy WiFi usage
#include <ESPmDNS.h>                                               // library to create multicast DNS (LAN ONLY)
#include <SPI.h>                                                   // SPI library to communicate with MFRC522
#include <MFRC522.h>                                               // MFRC522 Core Library
#include <HTTPClient.h>                                            // Library to implement HTTP Protocol
// Define value
#define RFID_SCAN 0                                                // Define Value for sendHTTPRequest
#define PEOPLE_PRESENT 1        
#define PEOPLE_NOT_PRESENT 2
// Function declaration
void RFIDCheckTask(void *pvParameters);
void sendHTTPRequest(String UID, int mode);
void blockRFIDReader(int blockTime);
void blockRFIDTask(void *pvParameters);
void setup();
void loop();
void HTTPResponseHandler(String HTTPResponse);

String readHTTPResponse(int delayTime);
String HTTPRequestBuilder(String UID, int mode);

// Constant Variable Declaration
const char *serverIP = "112.197.235.17";                           // CAUTION, need changing
const String className = "11T";                                    // change base on class
const String UUID = "0e47f9cf-ac45-46a8-baeb-c5ec048ceec3";        // Generate a new one at https://www.uuidgenerator.net/
const int serverPORT = 2705;                                       // Service server port
const int MFRC522RstPin = 13;                                      // Connect to MFRC522 SDA Pin
const int MFRC522SsPin = 12;                                       // Connect to MFRC522 RST Pin

// Normal Variable Declaration
byte rfUID[4];                                                     // Array for 4 byte RFID UID
int FLAG_ISBLOCKED = 0;                                            // FLAG for block function

WiFiMulti wifiMulti;                                               // Instance of WiFiMulti class, handle all WiFiMulti related thing
MFRC522 mfrc522(MFRC522SsPin, MFRC522RstPin);                      // Instance of MFRC522 class, handle all RFID related thing
WiFiClient wifiClient;                                             // WifiClient for HTTP sending and response

void setup() {
	// setup communication
	Serial.begin(115200);                                          // Initial Serial Connection to debug
	SPI.begin();                                                   // Initial SPI connection to comunicate with MFRC522

	// prepare MRFC522
	mfrc522.PCD_Init();                                            // Initial MFRC522
	mfrc522.PCD_DumpVersionToSerial();                             // Print MFRC522 version to serial, connection check

	// connect to WiFi using WiFiMulti function
	wifiMulti.addAP("DD_WRT", "NguyenThiHongTho");                 // Add AP to WiFiMulti List
	wifiMulti.addAP("Hello_World", "NguyenThiHongTho");            // Add AP to WiFiMulti List
	while (wifiMulti.run() != WL_CONNECTED) {                      // Wait for WiFi to connect
		Serial.println(".");
		delay(100);
	}
	Serial.print("Connected to");                                  // If connected, print WiFi SSID
	Serial.println(WiFi.SSID());

	// freeRTOS time!!
	xTaskCreate(RFIDCheckTask, /* Task main function */"RFIDCheckTask", /* Name of Task */
	30000, /* Task size in bytes */NULL, /* Parameter passed as input of the task */
	3, /* Priority of the task */NULL); /* Task handle */
}
void loop() {
	vTaskDelay(10);                                                // delay here to avoid watchdog timeout
} // end loop
void RFIDCheckTask(void *pvParameters) {
	String card_UID = "";                                          // create a String to hold card UID
	while (1) {                                                    // infinite loop
		vTaskDelay(10); // delay here for no reason
		if (FLAG_ISBLOCKED == 0 && mfrc522.PICC_IsNewCardPresent() // If it is not blocked and there is new card present
				&& mfrc522.PICC_ReadCardSerial()) {                // and it's available to read
			for (int i = 0; i < 4; i++) {                          // for loop 4 time to read 4 rfUID byte
				rfUID[i] = mfrc522.uid.uidByte[i];
				card_UID = card_UID + String(rfUID[i]);
			} // end for loop
			Serial.println(card_UID);                              // debug
			sendHTTPRequest(card_UID, RFID_SCAN);                  // send HTTP Request to Server by calling sendHTTPRequest function
			card_UID = "";                                         // after send it to server, reset it holder
			mfrc522.PICC_HaltA();                                  // Halt the RFID card
			mfrc522.PCD_StopCrypto1();                             // deauth the authenticated PICC
		} // end if
	} // end infinite loop;
} // end RFIDCheckTask
void sendHTTPRequest(String UID, int mode) {                       // function to send HTTP request
	if (!wifiClient.connect(serverIP, serverPORT)) {               // connect to server using specific port
		Serial.println("Connect to host failed");                  // debug
		return; // no further execution
	} else { // if connect successfully
		wifiClient.print(HTTPRequestBuilder(UID, mode));           // calling wifiClient.print to send HTTP request
	} // end if else
	Serial.println("");
	HTTPResponseHandler(readHTTPResponse(5000));                   // handle HTTP Response
} // end sendHTTPRequest

String HTTPRequestBuilder(String UID, int mode) {                  // function to create HTTPRequest
	String httpRequest = "";                                       // String to hold HTTPResponse
	switch (mode) { // check mode
	case RFID_SCAN:
		httpRequest = String("GET ") + "/:" + className + "/" + UID
				+ " HTTP/1.1\r\n" + "Host: " + serverIP + "\r\n"
				+ "Connection: close \r\n\r\n";
		break;
	case PEOPLE_PRESENT:
		httpRequest = String("GET ") + "/:" + className + "/" + "ON"
				+ " HTTP/1.1\r\n" + "Host: " + serverIP + "\r\n"
				+ "Connection: close\r\n\r\n";
		break;
	case PEOPLE_NOT_PRESENT:
		httpRequest = String("GET ") + "/:" + className + "/" + "OFF"
				+ " HTTP/1.1\r\n" + "Host: " + serverIP + "\r\n"
				+ "Connection: close\r\n\r\n";
		break;
	}
	Serial.println("HTTP REQUEST IS:");
	Serial.println(httpRequest);
	return httpRequest;
}

String readHTTPResponse(int delayTime) {                           // function to read HTTP response
	String httpResponse = "";                                      // Create a String to hold HTTP Response
	unsigned long timeOut = millis();                              // start the timer
	while (wifiClient.available() == 0) {                          // while no data received,
		if (millis() - timeOut > delayTime) {                      // if timeout
			Serial.println("NO HTTP RESPONSE RECIEVED ON TIME");
		} // end if
	} // end while
	Serial.println("Recieved:");                                   // debug
	while (wifiClient.available()) {                               // read until buffer is empty
		httpResponse = wifiClient.readString();                    // read from buffer
		Serial.println(httpResponse);                              // debug
	} // end while
	return httpResponse;
} // end readHTTPResponse

void HTTPResponseHandler(String httpResponse) {                    // function to parse HTTP response
	String header;                                                 // String to store Header e.g HTTP/1.0 200 OK
	String body;                                                   // Remaining part of the response
	String content;                                                // the content
	header = httpResponse.substring(0, httpResponse.indexOf("\n"));// first line of the response
	Serial.println(header);                                        //debug
	if (header.indexOf("200 OK")) {                                // if response is 200 OK
		body = httpResponse.substring(httpResponse.indexOf("\n")); // the body of the response
		content = body.substring(body.indexOf("GMT") + 7);         // take the content out of the body
		if (content.indexOf("OK") != -1) {                         // if content is "OK"
			Serial.println("Content: OK");                         // debug, do nothing
		} else if (content.indexOf("NOTFOUND") != -1) {            // if content is "NOTFOUND"
			Serial.println("Content: NOTFOUND");                   // debug
			Serial.println("Block RFID for 10s");                  // debug
			blockRFIDReader(10000);                                // block RFID for 10000ms (10s)
		} else if (content.indexOf("ERROR") != -1) {               // if content is "ERROR"
			Serial.println("Content: ERROR");                      // debug
			Serial.println("Block RFID for 30s");                  // debug
			blockRFIDReader(30000);                                // block RFID for 30000ms (30s)
		} // end if...else
	} // end if
} // end HTTPResponseHandler
void blockRFIDReader(int blockTime) {                              // function to handle blocking function
	xTaskCreate(blockRFIDTask, "blockRFIDTask", 1000, (void *) blockTime, 1,
	NULL);                                                         // create a block Task
}                                                                  // end blockRFIDReader
void blockRFIDTask(void *pvParameters) {                           // block Task
	int delayTime = (int) pvParameters;                            // cast the parameter back to int and assign it to a variable
	FLAG_ISBLOCKED = 1;                                            // set the FLAG_ISBLOCKED
	vTaskDelay(delayTime);                                         // delay for delayTime
	FLAG_ISBLOCKED = 0;                                            // clear the FLAG_ISBLOCKED
	vTaskDelete(NULL);                                             // delete itself
} // end block Task
