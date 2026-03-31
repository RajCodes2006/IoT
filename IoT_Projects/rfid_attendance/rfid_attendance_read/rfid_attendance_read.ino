#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define RST_PIN 22
#define SS_PIN  21
#define BUZZER  15
#define BUZZER_FREQ 1500
#define BUZZER_RESOLUTION 8

#define WIFI_SSID "Raj_Wifi"
#define WIFI_PASSWORD "01000101"

const String sheet_url = "https://script.google.com/macros/s/AKfycbw6r45aQL7DoBFI33ktL5HhmCdiAUm627bmRJ00OxL3LCzgux4xwMDnwUUW51d5H9n_iA/exec";

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

byte bufferLen = 18;
byte readBlockData[18];

void setup() {
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi Connected");

  ledcAttach(BUZZER, BUZZER_FREQ, BUZZER_RESOLUTION);
  ledcWrite(BUZZER, 0);

  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\nCard Detected");

  // 🔔 Single beep on detect
  ledcWrite(BUZZER, 128);
  delay(120);
  ledcWrite(BUZZER, 0);

  String name    = readBlock(1);
  String id      = readBlock(2);
  String section = readBlock(4);

  if (name == "" || id == "") {
    Serial.println("Invalid Data. Skipping HTTP.");
    delay(1000);
    loop();
  }

  Serial.println("---- Student Data ----");
  Serial.println("Name: " + name);
  Serial.println("ID: " + id);
  Serial.println("Section: " + section);
  Serial.println("----------------------");

  name.replace(" ", "%20");
  id.replace(" ", "%20");
  section.replace(" ", "%20");

  String url = sheet_url +"?name=" + name +"&uid=" + id +"&section=" + section;

  Serial.println(url);

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    https.setTimeout(15000);

    if (https.begin(client, url)) {

      int httpCode = https.GET();

      if (httpCode > 0) {
        Serial.printf("HTTP Code: %d\n", httpCode);
        Serial.println(https.getString());

        if (httpCode == 200) {
          // 🔔🔔🔔 3 PWM beeps on success
          for (int i = 0; i < 3; i++) {
            ledcWrite(BUZZER, 128);
            delay(150);
            ledcWrite(BUZZER, 0);
            delay(150);
          }
        }

      } 
      else {
        Serial.printf("HTTPS Error: %s\n",https.errorToString(httpCode).c_str());
      }

      https.end();
    }
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(3000);
}

String readBlock(byte blockNum) {

  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,blockNum,&key,&(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth Failed Block ");
    Serial.println(blockNum);
    return "";
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read Failed Block ");
    Serial.println(blockNum);
    return "";
  }

  String data = String((char*)readBlockData);
  data.trim();
  return data;
}