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

const char* serverName = "http://192.168.1.17:3000/";

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

  sendToFirebase();

  int hotTemperature = 40.5;
  int smokeSensorThreshold = 400;

  boolean isSmokeExist = smokeSensorValue > smokeSensorThreshold;
  boolean isFlameExist = flameSensorValue == HIGH;

  boolean isFireExist = isFlameExist && isSmokeExist && temperature > hotTemperature;
  boolean isSmokeOrFlameExist = isSmokeExist || isFlameExist;


  if (isFireExist) {
    sendPushNotification("Kebakaran");
    delay(5000);
  } else if (isSmokeOrFlameExist) {
    sendPushNotification("Peringatan");
    delay(5000);
  }

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

void sendPushNotification(String fireStatus) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String httpRequestData = "";
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
    if (fireStatus == "Kebakaran") {
      httpRequestData = "{\"to\":\"ExponentPushToken[eX94TXN37U4r4Ms9rn8NPV]\",\"title\":\"Berbahaya\",\"body\":\"Terjadi Kebakaran\"}";
    } else if (fireStatus == "Peringatan") {
      httpRequestData = "{\"to\":\"ExponentPushToken[eX94TXN37U4r4Ms9rn8NPV]\",\"title\":\"Peringatan\",\"body\":\"Peringatan\"}";
    }

    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

}
