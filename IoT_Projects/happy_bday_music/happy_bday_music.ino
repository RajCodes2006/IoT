#define BUZZER 1
#define RESOLUTION 8

#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523

void setup() {
  Serial.begin(115200);
  ledcAttach(BUZZER, 2000, RESOLUTION);
}

void loop() {
  if (Serial.available()) {
    char key = Serial.read();

    switch (key) {
      case 'a': ledcWriteTone(BUZZER, C4); break;
      case 's': ledcWriteTone(BUZZER, D4); break;
      case 'd': ledcWriteTone(BUZZER, E4); break;
      case 'f': ledcWriteTone(BUZZER, F4); break;
      case 'g': ledcWriteTone(BUZZER, G4); break;
      case 'h': ledcWriteTone(BUZZER, A4); break;
      case 'j': ledcWriteTone(BUZZER, B4); break;
      case 'k': ledcWriteTone(BUZZER, C5); break;
      case 'x': ledcWriteTone(BUZZER, 0); break; // stop
    }
  }
}
