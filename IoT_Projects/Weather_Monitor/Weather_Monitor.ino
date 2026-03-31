#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <UbidotsESPMQTT.h>



#define BMP_SDA 21
#define BMP_SCL 22

#define TOKEN "BBUS-Ag5FI9SMIehs269iGA90p0pb6c2uDr"   // Your Ubidots TOKEN

#define WIFISSID "Raj_Wifi" // Your SSID

#define WIFIPASS "0100101" // Your Wifi Pass


Adafruit_BMP280 bmp280;
Ubidots client(TOKEN);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
  Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Init... T2_Weather");

 
  Serial.println("Initializing BMP280");
  boolean status = bmp280.begin(0x76);
  if (!status) {
    Serial.println("BMP280 Not connected!");
  }
  Serial.println("Done");

  Serial.print("Connecting to SSID: ");
  Serial.print(WIFISSID);
  Serial.print(", Password: ");
  Serial.println(WIFIPASS);
  client.wifiConnection(WIFISSID, WIFIPASS);
  Serial.println("Done");

Serial.println(" Initializing Ubidots Connection...");
client.ubidotsSetBroker("industrial.api.ubidots.com");  // Sets the broker properly for the business account
client.setDebug(true);                        // Pass a true or false bool value to activate debug messages
client.begin(callback);
Serial.println("Done");

 
Serial.println("DONE");
}
void loop() {
  // Acquiring data from BMP280
  float temperature = bmp280.readTemperature();
  float pressure1 = bmp280.readPressure();
  float pressure = pressure1/10;
  float altitude = bmp280.readAltitude();
  float wbp = bmp280.waterBoilingPoint(pressure);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" Pa");
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");
  Serial.print("Water Boiling Point: ");
  Serial.print(wbp);
  Serial.println(" F");

  // Establising connection with Ubidots
  if (!client.connected()) {
  client.reconnect();
  }

  // Publising data of both variable to Ubidots
  client.add("temperature", temperature);  // Insert your variable Labels and the value to be sent
  client.add("pressure", pressure);
  client.add("altitude", altitude);  // Insert your variable Labels and the value to be sent
  client.add("wbp", wbp);
  client.ubidotsPublish("weather-monitor"); // insert your device label here
  client.loop();
  delay(5000);
}
