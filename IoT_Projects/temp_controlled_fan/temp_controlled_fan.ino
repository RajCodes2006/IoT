#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;

#define INA 9
#define INB 10

void setup() {
  Serial.begin(9600);

  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);

  if (!bmp.begin(0x76)) {   // try 0x77 if needed
    Serial.println("BMP280 not found");
    while (1);
  }
}

void loop() {

  float temperature = bmp.readTemperature();
  int speed = 0;

  if (temperature < 25) {
    speed = 0;
  }
  else if (temperature >= 25 && temperature < 30) {
    speed = 120;
  }
  else if (temperature >= 30 && temperature < 35) {
    speed = 180;
  }
  else {
    speed = 255;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C  | Speed: ");
  Serial.println(speed);

  if (speed == 0) {
    digitalWrite(INA, HIGH);   // brake
    digitalWrite(INB, HIGH);
  } 
  else {
    digitalWrite(INB, LOW);    // forward
    analogWrite(INA, speed);
  }

  delay(1000);
}