#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22
#define SS_PIN  21

#define BUZZER 15
#define BUZZER_FREQ 1500
#define BUZZER_RESOLUTION 8

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

// Your Data
char nameData[]    = "NAVRAJ SINGH";
char uidData[]     = "25BCS11848";
char sectionData[] = "206-A";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  ledcAttach(BUZZER, BUZZER_FREQ, BUZZER_RESOLUTION);
  ledcWrite(BUZZER, 0);

  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  Serial.println("Scan card to write full student data...");
}

void loop() {

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("Card Detected");

  writeBlock(1, nameData);
  writeBlock(2, uidData);
  writeBlock(4, sectionData);

  Serial.println("All Data Written Successfully");

  // 🔔 3 Beeps
  for (int i = 0; i < 3; i++) {
    ledcWrite(BUZZER, 128);
    delay(150);
    ledcWrite(BUZZER, 0);
    delay(150);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  while(1);
}

void writeBlock(byte blockNum, char *data) {

  byte buffer[16] = {0};

  int len = strlen(data);
  if (len > 16) len = 16;

  memcpy(buffer, data, len);

  byte sector = blockNum / 4;
  byte trailerBlock = sector * 4 + 3;

  status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &key,
        &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth Failed Block ");
    Serial.println(blockNum);
    return;
  }

  status = mfrc522.MIFARE_Write(blockNum, buffer, 16);

  if (status == MFRC522::STATUS_OK) {
    Serial.print("Block ");
    Serial.print(blockNum);
    Serial.println(" Written");
    Serial.print("Data: ");
    Serial.println(data);
  } else {
    Serial.print("Write Failed Block ");
    Serial.println(blockNum);
  }
}

