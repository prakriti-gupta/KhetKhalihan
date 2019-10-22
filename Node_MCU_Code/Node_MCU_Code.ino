#include <Wire.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SocketIoClient.h>
#include <ESP8266WiFiMulti.h>

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;

SocketIoClient webSocket;

char t[10];
char cropId[1] = {'0'}; // 1,2,3,4, 0 for nothing
char temperature[5] = {'0', '0'}; // T
char humidity[5] = {'0', '0'}; // H
char moisture[5] = {'0', '0'}; // M
char sunlight[5] = {'0', '0'}; // S
bool isFanOn = false; // F
bool isWaterPumpOn = false; // P
bool isHeaterOn = false; // #

void observeCropDataFromServer(const char * payload, size_t length) {
  Serial.println(payload);
  cropId[0] = payload[0];

  char cropIdData[200];
  sprintf(cropIdData, "{\"msg\" : %c }", cropId[0]);
  webSocket.emit("cropData", cropIdData);

  Serial.flush();
  Wire.beginTransmission(8);
  Wire.write(cropId[0]);
  Wire.endTransmission();
}

void setup(void) {

  Serial.begin(9600);

  // I2C Bus
  Wire.begin(D1, D2);

  // WebSockets Protocol
  USE_SERIAL.begin(9600);
  USE_SERIAL.setDebugOutput(true);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("Manas", "0987654321"); // ssid, password
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  webSocket.on("cropData", observeCropDataFromServer);
  webSocket.begin("18.222.125.38", 80); // server's IP Address , Port number (80 for http/https);
}

void loop(void) {

  webSocket.loop();

  char cropIdData[200];
  sprintf(cropIdData, "{\"msg\" : %c }", cropId[0]);
  webSocket.emit("cropData", cropIdData);

  if (cropId[0] == '0') { // user has not logged in and has therefore has not selected a crop
    Serial.println("Returning from loop function..");
    cropId[0];
    return;
  }

  Wire.beginTransmission(8);
  Wire.endTransmission();

  Wire.requestFrom(8, 3);
  int i = 0; // counter for each byte as it arrives
  while (Wire.available()) {
    t[i] = Wire.read(); // every character that arrives it put in order in the empty array "t"
    i = i + 1;
  }

  // Dht22
  if (t[2] == 'T') {
    temperature[0] = t[0];
    temperature[1] = t[1];
  }

  // Dht22
  if (t[2] == 'H') {
    humidity[0] = t[0];
    humidity[1] = t[1];
  }

  // Ldr
  if (t[2] == 'S') {
    sunlight[0] = t[0];
    sunlight[1] = t[1];
  }

  // Soil
  if (t[2] == 'M') {
    moisture[0] = t[0];
    moisture[1] = t[1];
  }

  // Fan
  if (t[2] == 'F') {
    if ((t[0] == '0') && (t[1] == '0')) {
      isFanOn = false;
    } else {
      isFanOn = true;
    }
  }

  // Water Pump
  if (t[2] == 'P') {
    if ((t[0] == '0') && (t[1] == '0')) {
      isWaterPumpOn = false;
    } else {
      isWaterPumpOn = true;
    }
  }

  // Heater
  if (t[2] == '#') {
    if ((t[0] == '0') && (t[1] == '0')) {
      isHeaterOn = false;
    } else {
      isHeaterOn = true;
    }
  }

  char temperatureData[200];
  sprintf(temperatureData, "{\"msg\" : %c%c }", temperature[0], temperature[1]);
  webSocket.emit("temperature", temperatureData);

  char humidityData[200];
  sprintf(humidityData, "{\"msg\" : %c%c }", humidity[0], humidity[1]);
  webSocket.emit("humidity", humidityData);

  char sunlightData[200];
  sprintf(sunlightData, "{\"msg\" : %c%c }", sunlight[0], sunlight[1]);
  webSocket.emit("sunlight", sunlightData);

  char moistureData[200];
  sprintf(moistureData, "{\"msg\" : %c%c }", moisture[0], moisture[1]);
  webSocket.emit("moisture", moistureData);

  char isFanOnData[200];
  sprintf(isFanOnData, "{\"msg\" : %d }", isFanOn);
  webSocket.emit("isFanOn", isFanOnData);

  char isWaterPumpOnData[100];
  sprintf(isWaterPumpOnData, "{\"msg\" : %d }", isWaterPumpOn);
  webSocket.emit("isWaterPumpOn", isWaterPumpOnData);

  char isHeaterOnData[100];
  sprintf(isHeaterOnData, "{\"msg\" : %d }", isHeaterOn);
  webSocket.emit("isHeaterOn", isHeaterOnData);

  delay(3000);
}
