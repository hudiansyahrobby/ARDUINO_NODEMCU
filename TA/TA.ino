#include <DHT.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define DHTPIN 2 //pin DATA konek ke pin 2 Arduino
#define DHTTYPE DHT11 //tipe sensor DHT11

static const int RX = 4, TX = 3;
static const int espTX = 5, espRX = 6;

int flameSensorPin = 9;
int buzzerPin = 11;
int smokeSensorPin = A0;
int data[19];

int smokeSensorThreshold = 400;

// GPS
float lat = 28.5458, lon = 77.1703;

// Sensor
float temperature;
int smokeSensorValue, flameSensorValue;

DHT dht(DHTPIN, DHTTYPE);

TinyGPS gps;//

SoftwareSerial gpsneo(RX, TX);
SoftwareSerial espSerial(espTX, espRX);



void setup ()
{

  pinMode (flameSensorPin, INPUT) ;

  pinMode (buzzerPin, OUTPUT);
  pinMode(smokeSensorPin, INPUT);


  dht.begin();

  Serial.begin(9600);
  espSerial.begin(115200);
  gpsneo.begin(9600);
  delay(1000);
}

void loop ()
{
  int smokeSensorValue = analogRead(smokeSensorPin);
  int flameSensorValue = digitalRead (flameSensorPin) ;
  float temperature = dht.readTemperature();

  int hotTemperature = 40.5;
  boolean isSmokeExist =  smokeSensorValue > smokeSensorThreshold;
  boolean isFlameExist = flameSensorValue == HIGH;

  boolean isFireExist = isFlameExist && isSmokeExist && temperature > hotTemperature;
  boolean isSmokeOrFlameExist = isSmokeExist || isFlameExist;

  Serial.println(flameSensorValue);
  Serial.println(smokeSensorValue);
  Serial.println(temperature);


  while (gpsneo.available()) { // check for gps data
    if (gps.encode(gpsneo.read())) // encode gps data
    {
      gps.f_get_position(&lat, &lon); // get latitude and longitude
    }
  }

  String latitude = String(lat, 6);
  String longitude = String(lon, 6);

  if (isFireExist || isSmokeOrFlameExist) {
    alarm();
  };

  StaticJsonBuffer<3000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = temperature;
  root["flame"] = flameSensorValue;
  root["smoke"] = smokeSensorValue;
  root["latitude"] = latitude;
  root["longitude"] = longitude;

  root.printTo(espSerial);
  jsonBuffer.clear();

  delay(1000);
}

void alarm()  {
  tone(buzzerPin, 1000); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzerPin);     // Stop sound...
  delay(500);        // ...for 1sec
}
