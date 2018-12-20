/*
 * old-code.cpp
 *
 *  Created on: Dec 20, 2018
 *      Author: nhan
 */
/////////////////////////////////
// Generated with a lot of love//
// with TUNIOT FOR ESP32     //
// Website: Easycoding.tn      //
/////////////////////////////////
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 5
#define RST_PIN 22
IPAddress staticIP725_30(192,168,1,10);
IPAddress gateway725_30(192,168,1,1);
IPAddress subnet725_30(255,255,255,0);
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x3F, 16, 2);

byte nuidPICC[4];
String deptrai="/";
WiFiServer server(80);

WiFiClient client;

String myresultat;



String myurl ="/";
int ok,ok1;
void setup()
{
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init();
  Serial.println("START");
  WiFi.begin("TP-LINK","0949321325");
  while ((!(WiFi.status() == WL_CONNECTED))){
    delay(300);
    Serial.print("..");

  }
  Serial.println("Connected");
  WiFi.config(staticIP725_30, gateway725_30, subnet725_30);
  Serial.println("Your IP is");
  Serial.println((WiFi.localIP()));
  server.begin();
}
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;
  }
  message = message + " ";
  for (int pos = 0; pos < message.length(); pos++) {
    int button1 = analogRead(34);
    if(button1>100) lcd.noBacklight();
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);

  }
}
String something = "";
void loop(){
  client =server.available();
  int button = analogRead(34);
  Serial.println(button);
  int quangtro=analogRead(36);
  if(rfid.PICC_IsNewCardPresent())
    if(rfid.PICC_ReadCardSerial()) {
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
        deptrai=deptrai+String(nuidPICC[i]);
      }
      Serial.println(deptrai);
      if(client.connect("192.168.1.103",2705)){
      client.print(String("GET ") + "11T" +deptrai + " HTTP/1.1\r\n"
      "Host: " + "192.168.1.103" + "\r\n"
      "Connection: close\r\n\r\n");
      deptrai="/";
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      }
    }
    if(quangtro <3000&&ok==0)
      if(client.connect("192.168.1.103",2705)){
        client.print(String("GET ") + ":11T/ON" + " HTTP/1.1\r\n"
        "Host: " + "192.168.1.103" + "\r\n"
        "Connection: close\r\n\r\n");
      ok=1;
    }
    if(quangtro>3000&&ok==1)
      if(client.connect("192.168.1.103",2705)){
        client.print(String("GET ") + ":11T/OFF" + " HTTP/1.1\r\n"
        "Host: " + "192.168.1.103" + "\r\n"
        "Connection: close\r\n\r\n");
       ok=0;
    }
    if(button>100) {
      lcd.noBacklight();
    }
    if(client ) {
       something = client.readStringUntil('\r');
      if(something != "") {
        lcd.clear();
        something.remove(0, 5);
        something.remove(something.length()-9,9);
        Serial.println(something);
        lcd.backlight();
      }
    }
    if(something!=""){
      lcd.setCursor(0,0);
  //    scrollText(0,something,250,16);
    }
}





