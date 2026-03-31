#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RST_PIN 22
#define SS_PIN  21
#define BUZZER  15

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define WIFI_SSID "Raj_Wifi"
#define WIFI_PASSWORD "01000101"

const String sheet_url = "https://script.google.com/macros/s/AKfycbxyZCZKqqHAAU3-BMeN2FZfcRT1Ar4W-HZF1TjIDv_kKaEpLHArweSufOxNCz2HI5qr8Q/exec";

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

byte bufferLen = 18;
byte readBlockData[18];

// OLED
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void showText(String l1, String l2 = "", String l3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println(l1);

  display.setCursor(0, 20);
  display.println(l2);

  display.setCursor(0, 40);
  display.println(l3);

  display.display();
}

void setup() {
  Serial.begin(9600);

  // OLED init
  Wire.begin(4, 5);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  showText("Starting...");
  Serial.println("OLED Initialized");

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  showText("Connecting WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nWiFi Connected");
  showText("WiFi Connected");

  ledcAttach(BUZZER, 1500, 8);
  ledcWrite(BUZZER, 0);

  SPI.begin();
  mfrc522.PCD_Init();

  showText("Ready", "Scan Card");
  Serial.println("RFID Ready. Scan Card...");
}

void loop() {

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\nCard Detected");

  // beep
  ledcWrite(BUZZER, 128);
  delay(120);
  ledcWrite(BUZZER, 0);

  String name    = readBlock(1);
  String id      = readBlock(2);
  String section = readBlock(4);

  if (name == "" || id == "") {
  Serial.println("Invalid Data. Skipping HTTP.");
  showText("Invalid Card");
  delay(1000);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  showText("Ready", "Scan Card");
  loop();
  return;
  }

  Serial.println("---- Student Data ----");
  Serial.println("Name: " + name);
  Serial.println("ID: " + id);
  Serial.println("Section: " + section);
  Serial.println("----------------------");

  showText("Reading...", name, id);

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
          name.replace("%20"," ");
          showText("Success", name);

          // 3 beeps
          for (int i = 0; i < 3; i++) {
            ledcWrite(BUZZER, 128);
            delay(150);
            ledcWrite(BUZZER, 0);
            delay(150);
          }
        }

      } else {
        Serial.printf("HTTPS Error: %s\n", https.errorToString(httpCode).c_str());
        showText("HTTP Failed");
      }

      https.end();
    }
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(3000);
  showText("Ready", "Scan Card");
}

String readBlock(byte blockNum) {

  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    blockNum, &key, &(mfrc522.uid)
  );

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