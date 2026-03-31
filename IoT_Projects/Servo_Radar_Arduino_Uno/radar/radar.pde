import processing.serial.*;

Serial myPort;
String data = "";
int angle = 0;
float distance = 0;

void setup() {
  size(900, 600);
  myPort = new Serial(this, "COM6", 9600);  // CHANGE COM PORT
  myPort.bufferUntil('\n');
}

void draw() {
  background(0);
  translate(width/2, height);

  drawRadar();
  drawLine();
  drawObject();
}

void drawRadar() {

  stroke(0,255,0);
  noFill();

  // Each 10 cm = 100 pixels
  arc(0,0,800,800,PI,TWO_PI);  // 40 cm
  arc(0,0,600,600,PI,TWO_PI);  // 30 cm
  arc(0,0,400,400,PI,TWO_PI);  // 20 cm
  arc(0,0,200,200,PI,TWO_PI);  // 10 cm

  // Angle lines
  for(int i = 0; i <= 180; i += 30){
    float x = 400 * cos(radians(i));
    float y = 400 * sin(radians(i));
    line(0,0,x,-y);
  }

  // Distance labels
  fill(0,255,0);
  textSize(15);
  text("10cm", -20, -100);
  text("20cm", -20, -200);
  text("30cm", -20, -300);
  text("40cm", -20, -400);
}

void drawLine() {
  stroke(0,255,0);
  float x = 400 * cos(radians(angle));
  float y = 400 * sin(radians(angle));
  line(0,0,x,-y);
}
void drawObject() {

  if (distance > 0 && distance <= 40) {

    float mappedDist = distance * 10;   // 1 cm = 10 pixels

    float x = mappedDist * cos(radians(angle));
    float y = mappedDist * sin(radians(angle));

    fill(255,0,0);
    noStroke();
    ellipse(x, -y, 12, 12);
  }
}
void serialEvent(Serial myPort) {
  data = trim(myPort.readStringUntil('\n'));
  if(data != null){
    String[] values = split(data, ',');
    if(values.length == 2){
      angle = int(values[0]);
      distance = float(values[1]);
    }
  }
}
