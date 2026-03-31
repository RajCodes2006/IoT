#include <Servo.h>

#define TRIG 5
#define ECHO 6
#define BUZZER 3
#define SERVO_PIN 9

Servo myServo;

long duration;
float distance;

int pos = 30;
int stepSize = 2;
int minAngle = 0;
int maxAngle = 180;

// 🔥 Required for smart buzzer
unsigned long previousMillis = 0;
int buzzerState = LOW;

float getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH, 25000);

  if (duration == 0) return 400;

  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(9600);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);

  myServo.attach(SERVO_PIN);
}

void loop() {

  myServo.write(pos);
  delay(8);   // let servo settle slightly

  float d = getDistance();

  Serial.print(pos);
  Serial.print(",");
  Serial.println(d);

  // 🔥 Smart Buzzer Logic
  unsigned long currentMillis = millis();

  if (d <= 40 && d > 0) {

    int interval = map(d, 5, 40, 50, 400);
    interval = constrain(interval, 50, 400);

    int freq = map(d, 5, 40, 3500, 1200);
    freq = constrain(freq, 1200, 3500);

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      if (buzzerState == LOW) {
        tone(BUZZER, freq);
        buzzerState = HIGH;
      } else {
        noTone(BUZZER);
        buzzerState = LOW;
      }
    }

  } else {
    noTone(BUZZER);
  }

  pos += stepSize;

  if (pos >= maxAngle || pos <= minAngle) {
    stepSize = -stepSize;
  }

  delay(4);
}