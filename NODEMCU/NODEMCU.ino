#include  <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define FIREBASE_HOST "log-ta-default-rtdb.firebaseio.com"
#define WIFI_SSID "Bale Black" // Change the name of your WIFI
#define WIFI_PASSWORD "0987654321" // Change the password of your WIFI
#define FIREBASE_AUTH "0l1PgZrD0ZhkL51jZ4srAUPOIPZ4yzgm6MjRQp7F"

// GPS
float latitude, longitude;

// Sensor
float temperature;
int smokeSensorValue, flameSensorValue;

SoftwareSerial espSerial(D6, D5);

const char* serverName = "http://192.168.1.106:1880/update-sensor";

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);
  connectToWifi();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  if (Firebase.failed())
  {
    Serial.print(Firebase.error());
  } else {

    Serial.print("Firebase Connected");

  }

  while (!Serial) {
    continue; // wait for serial port to connect. Needed for native USB port only
  }
}


void loop() {

  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& doc = jsonBuffer.parseObject(espSerial);

  if (doc == JsonObject::invalid()) {
    Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }
  doc.prettyPrintTo(Serial);
  temperature = doc["temperature"];
  flameSensorValue = doc["flame"];
  smokeSensorValue = doc["smoke"];;
  latitude = doc["latitude"];
  longitude = doc["longitude"];


  //  jsonBuffer.clear();
  sendToFirebase();

}

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}

void sendToFirebase() {

  Firebase.setFloat("flame", flameSensorValue);
  Firebase.setFloat ("temperature", temperature);
  Firebase.setFloat ("smoke", smokeSensorValue);
  Firebase.setFloat ("latitude", latitude);
  Firebase.setFloat ("longitude", longitude);

}

void sendPushNotification() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);

    int httpResponseCode = http.POST();

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }



}
