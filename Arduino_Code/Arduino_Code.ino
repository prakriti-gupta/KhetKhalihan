// SLAVE ARDUINO.

/*
    DHT22
   1 - VCC
   2 - 8
   3 - Unused
   4 - GND
*/

/*
   Soil Sensor
   1 - VCC
   GND - GND
   3 - Unused
   4 - A0
*/

/*
   LDR
   One leg of LDR - 5V.
   Second leg of LDR - 10K resistor and the resistor is connected to A1.
   Second leg of resistor to GND.
*/

/*
   RX - TX Serial communication
   Node MCU D1 - A4
   Node MCU D2 - A5
   Node MCU GND - GND
*/

#include <DHT.h>
#include <Wire.h>

#define DHTPIN 8  // Choose a Digital PIN not Analog PIN.
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int fanPin = 7;
const int pumpPin = A3;
const int heaterPin = 4;
const int LDRPin = A1;
const int soilSensor = A0;  // Always connect to A0.
char cropId[1] = {'0'}; // 1,2,3,4, 0 for nothing

bool isWaterPumpOn = false;
bool isFanOn = false;
bool isHeaterOn = false;
int humidity = 0, temperature = 0, moisture = 0, LDRStatus = 0;

class Crop {
  public:
    int id;
    char *_name;
    int maxH;
    int minH;
    int minT;
    int maxT;
    int minM;
    int maxM;
    Crop(int _id, char *n, int maxHumid, int minHumid, int maxTemp, int minTemp, int maxMoisture, int minMoisture) {
      id = _id;
      _name = n;
      maxH = maxHumid ;
      minH = minHumid;
      maxT = maxTemp;
      minT = minTemp;
      maxM = maxMoisture;
      minM = minMoisture;
    }
};

Crop Kharif(1, "Kharif", 60, 50, 32, 18, 90, 60);
Crop Cash(2, "Cash", 35, 20, 65, 40, 90, 60);
Crop Plantation(3, "Plantation", 30, 18, 60, 50, 90, 60);
Crop Zaid(4, "Zaid", 75, 50, 12, 10, 30, 10);
const Crop crops[4] = {Kharif, Cash, Plantation, Zaid};

void handleCrop(char cropId);
void handleKharif(void);
void handleCash(void);
void handlePlantation(void);
void handleZaid(void);
void handleDefault(void);

void setup(void) {

  Serial.begin(9600);

  dht.begin();

  pinMode(LDRPin, INPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(soilSensor, INPUT);

  // I2C Protocol
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop(void) {

  delay(2000);

  if (cropId[0] == '0') {
    Serial.println("returning from loop...");
    return;
  }

  // Dht22
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  // Soil Sensor
  moisture = analogRead(soilSensor);
  moisture = map(moisture, 1, 14, 0, 1);

  // Ldr
  LDRStatus = analogRead(LDRPin);
  LDRStatus = map(LDRStatus, 1, 11, 0, 1);

  handleCrop(cropId[0]);
}

// function that executes whenever data is received from master , received from NodeMCU
void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    cropId[0] = Wire.read(); // receive byte as a character
    Serial.println(cropId[0]);
  }
}

// function that executes whenever data is requested from master , sends to NodeMCU
void requestEvent(void) {

  static int n = 0;
  int x = n % 6;
  int num;
  char cstr[16];
  if (x == 0) {
    num = temperature;
    itoa(num, cstr, 10);
    cstr[2] = 'T';
    Wire.write(cstr);
  } else if (x == 1) {
    num = humidity;
    itoa(num, cstr, 10);
    cstr[2] = 'H';
    Wire.write(cstr);
  } else if (x == 2) {
    num = LDRStatus;
    itoa(num, cstr, 10);
    cstr[2] = 'S';
    Wire.write(cstr);
  } else if (x == 3) {
    num = moisture;
    itoa(num, cstr, 10);
    cstr[2] = 'M';
    Wire.write(cstr);
  } else if (x == 4) {
    if (isFanOn == true) {
      cstr[0] = '1';
      cstr[1] = '1';
      cstr[2] = 'F';
    } else {
      cstr[0] = '0';
      cstr[1] = '0';
      cstr[2] = 'F';
    }
    Wire.write(cstr);
  } else if (x == 5) {
    if (isWaterPumpOn == true) {
      cstr[0] = '1';
      cstr[1] = '1';
      cstr[2] = 'P';
    } else {
      cstr[0] = '0';
      cstr[1] = '0';
      cstr[2] = 'P';
    }
    Wire.write(cstr);
  } else if (x == 6) {
    if (isHeaterOn == true) {
      cstr[0] = '1';
      cstr[1] = '1';
      cstr[2] = '#';
    } else {
      cstr[0] = '0';
      cstr[1] = '0';
      cstr[2] = '#';
    }
    Wire.write(cstr);
  }
  n++;
}

void handleCrop(char cropId) {
  switch (cropId) {
    case '1' :
      handleKharif();
      break;
    case '2' :
      handleCash();
      break;
    case '3' :
      handlePlantation();
      break;
    case '4' :
      handleZaid();
      break;
    default:
      handleDefault();
      break;
  }
}

void handleKharif(void) {
  Serial.println("handling kharif");
  return;

  // Humidity
  if ((humidity > crops[0].minH) && (humidity < crops[0].maxH)) {
    digitalWrite(heaterPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  else if (humidity > crops[0].maxH) {
    digitalWrite(heaterPin, HIGH);
    digitalWrite(fanPin, LOW);
  }
  else {
    digitalWrite(fanPin, HIGH);
    digitalWrite(heaterPin, LOW);
  }

  // Temperature
  if ((temperature > crops[0].minT) && (temperature < crops[0].maxT)) {
    digitalWrite(heaterPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  else if (temperature > crops[0].maxT) {
    digitalWrite(fanPin, HIGH);
    digitalWrite(heaterPin, LOW);
  }
  else {
    digitalWrite(heaterPin, HIGH);
    digitalWrite(fanPin, LOW);
  }

  // Moisture
  if ((moisture > crops[0].minM) && (moisture < crops[0].maxM)) {
    digitalWrite(pumpPin, LOW);
    digitalWrite(heaterPin, LOW);
  }
  else if (moisture > crops[0].maxT) {
    digitalWrite(pumpPin, LOW);
    digitalWrite(heaterPin, HIGH);
  }
  else {
    digitalWrite(pumpPin, HIGH);
    digitalWrite(heaterPin, LOW);
  }
}

void handleCash(void) {
  if ((humidity > crops[1].minH) && (humidity < crops[1].maxH)) {
    digitalWrite(heaterPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  else if (humidity > crops[1].maxH) {
    digitalWrite(heaterPin, HIGH);
    digitalWrite(fanPin, LOW);
  }
  else {
    digitalWrite(fanPin, HIGH);
    digitalWrite(heaterPin, LOW);
  }
}

void handlePlantation(void) {
  if ((humidity > crops[2].minH) && (humidity < crops[2].maxH)) {
    digitalWrite(heaterPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  else if (humidity > crops[2].maxH) {
    digitalWrite(heaterPin, HIGH);
    digitalWrite(fanPin, LOW);
  }
  else {
    digitalWrite(fanPin, HIGH);
    digitalWrite(heaterPin, LOW);
  }
}

void handleZaid(void) {
  if ((humidity > crops[3].minH) && (humidity < crops[3].maxH)) {
    digitalWrite(heaterPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  else if (humidity > crops[3].maxH) {
    digitalWrite(heaterPin, HIGH);
    digitalWrite(fanPin, LOW);
  }
  else {
    digitalWrite(fanPin, HIGH);
    digitalWrite(heaterPin, LOW);
  }
}

void handleDefault(void) {
  digitalWrite(fanPin, LOW);
  digitalWrite(pumpPin, LOW);
  digitalWrite(heaterPin, LOW);
}
