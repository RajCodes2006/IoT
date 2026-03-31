#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ── Pin Definitions ────────────────────────────────────────────────────────
#define RST_PIN           22
#define SS_PIN            21
#define BUZZER            15
#define BUZZER_FREQ       1500
#define BUZZER_RESOLUTION 8

// I2C pins for LCD
#define LCD_SDA           4
#define LCD_SCL           5

// ── WiFi Credentials ───────────────────────────────────────────────────────
#define WIFI_SSID         "Raj_Wifi"
#define WIFI_PASSWORD     "01000101"

// ── Google Sheet URL ───────────────────────────────────────────────────────
const String SHEET_URL = "https://script.google.com/macros/s/AKfycbxyZCZKqqHAAU3-BMeN2FZfcRT1Ar4W-HZF1TjIDv_kKaEpLHArweSufOxNCz2HI5qr8Q/exec";

// ── LCD: address 0x27 (try 0x3F if blank) ─────────────────────────────────
// VCC → 5V (Vin), GND → GND, SDA → GPIO4, SCL → GPIO5
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── RFID ───────────────────────────────────────────────────────────────────
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

byte bufferLen     = 18;
byte readBlockData[18];

// ══════════════════════════════════════════════════════════════════════════
// Helper: print two lines on LCD (max 16 chars each)
// ══════════════════════════════════════════════════════════════════════════
void lcdPrint(const String& line1, const String& line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, 16));
  if (line2.length()) {
    lcd.setCursor(0, 1);
    lcd.print(line2.substring(0, 16));
  }
}

// ══════════════════════════════════════════════════════════════════════════
// Helper: beep buzzer N times
// ══════════════════════════════════════════════════════════════════════════
void beep(int times = 1, int onMs = 150, int offMs = 150) {
  for (int i = 0; i < times; i++) {
    ledcWrite(BUZZER, 128);
    delay(onMs);
    ledcWrite(BUZZER, 0);
    if (i < times - 1) delay(offMs);
  }
}

// ══════════════════════════════════════════════════════════════════════════
// Helper: URL-encode spaces
// ══════════════════════════════════════════════════════════════════════════
String urlEncode(String str) {
  str.replace(" ", "%20");
  return str;
}

// ══════════════════════════════════════════════════════════════════════════
// Read one MIFARE block and return trimmed string
// ══════════════════════════════════════════════════════════════════════════
String readBlock(byte blockNum) {
  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.printf("Auth Failed — Block %d\n", blockNum);
    return "";
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);

  if (status != MFRC522::STATUS_OK) {
    Serial.printf("Read Failed — Block %d\n", blockNum);
    return "";
  }

  String data = String((char*)readBlockData);
  data.trim();
  return data;
}

// ══════════════════════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // LCD (I2C on custom pins, VCC → 5V)
  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcdPrint("RFID Attendance", "Initializing...");
  delay(1000);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcdPrint("Connecting WiFi", WIFI_SSID);
  Serial.print("Connecting to WiFi");

  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - wifiStart > 15000) {       // 15-second timeout
      Serial.println("\nWiFi Timeout! Restarting...");
      lcdPrint("WiFi Timeout!", "Restarting...");
      delay(2000);
      ESP.restart();
    }
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nWiFi Connected: " + WiFi.localIP().toString());
  lcdPrint("WiFi Connected!", WiFi.localIP().toString());
  delay(1500);

  // Buzzer
  ledcAttach(BUZZER, BUZZER_FREQ, BUZZER_RESOLUTION);
  ledcWrite(BUZZER, 0);
  beep(1, 80);   // single short beep = ready

  // RFID
  SPI.begin();
  mfrc522.PCD_Init();

  lcdPrint("Scan Your Card", "");
  Serial.println("Ready — scan a card.");
}

// ══════════════════════════════════════════════════════════════════════════
// LOOP
// ══════════════════════════════════════════════════════════════════════════
void loop() {

  // ── Reconnect WiFi if dropped ──────────────────────────────────────────
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost — reconnecting...");
    lcdPrint("WiFi Lost!", "Reconnecting...");
    WiFi.reconnect();
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 10000)
      delay(300);
    if (WiFi.status() == WL_CONNECTED) {
      lcdPrint("WiFi Restored!", "");
      delay(1000);
    } else {
      lcdPrint("No WiFi!", "Check Router");
      delay(3000);
    }
    lcdPrint("Scan Your Card");
  }

  // ── Wait for card ──────────────────────────────────────────────────────
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial())   return;

  Serial.println("\n── Card Detected ──");
  lcdPrint("Card Detected!", "Reading...");
  beep(1, 120);

  // ── Read blocks ────────────────────────────────────────────────────────
  String name    = readBlock(1);
  String id      = readBlock(2);
  String section = readBlock(4);

  // Halt card now (data already read)
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // ── Validate ───────────────────────────────────────────────────────────
  if (name == "" || id == "") {
    Serial.println("Invalid / blank card data.");
    lcdPrint("Invalid Card!", "Try Again");
    beep(2, 300, 100);
    delay(2000);
    lcdPrint("Scan Your Card");
    return;
  }

  // ── Print to Serial ────────────────────────────────────────────────────
  Serial.println("Name:    " + name);
  Serial.println("ID:      " + id);
  Serial.println("Section: " + section);

  // ── Show on LCD ────────────────────────────────────────────────────────
  lcdPrint(name, "ID: " + id);
  delay(1200);
  lcdPrint("Sec: " + section, "Sending...");

  // ── Send to Google Sheet ───────────────────────────────────────────────
  String url = SHEET_URL
             + "?name="    + urlEncode(name)
             + "&uid="     + urlEncode(id)
             + "&section=" + urlEncode(section);

  Serial.println("URL: " + url);

  WiFiClientSecure client;
  client.setInsecure();           // skip SSL cert verification

  HTTPClient https;
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  https.setTimeout(15000);

  if (https.begin(client, url)) {
    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP %d — %s\n", httpCode, https.getString().c_str());

      if (httpCode == 200) {
        lcdPrint("Marked Present!", name);
        beep(3, 150, 150);        // 3 beeps = success
      } else {
        lcdPrint("HTTP Error:", String(httpCode));
        beep(2, 400, 100);
      }

    } else {
      Serial.println("HTTPS Error: " + https.errorToString(httpCode));
      lcdPrint("Network Error", https.errorToString(httpCode));
      beep(2, 400, 100);
    }

    https.end();

  } else {
    Serial.println("Failed to begin HTTPS");
    lcdPrint("HTTPS Failed", "Check URL");
    beep(2, 400, 100);
  }

  delay(3000);
  lcdPrint("Scan Your Card");
}
