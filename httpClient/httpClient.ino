#include <WiFi.h>
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "DHT.h"

#define LED 2
#define DHTPIN 4
#define DHTTYPE DHT22
#define uS_TO_S_FACTOR 1000000
#define mS_TO_S_FACTOR 1000
#define SECONDS_TO_SLEEP 60
//#define DEEP_SLEEP true
#define WIFI_CONNECT_TIMEOUT 200  //*100ms

// Replace with your network credentials (STATION)
const int voltage_measuring_pin = 36;
const char* ssid = "iot-network";
const char* password = "iot-pa$$w0rd";
const String serverName = "https://iot.cloud.ga66a.ru";
const String apiPath = "/api/v1/iot";
//int ads_value = 0;
const char* firmwareUrl = "http://test/test/test";
bool deepSleep = true;

DHT dht(DHTPIN, DHTTYPE);

void printWiFiStatus() {
  Serial.println("");
  Serial.println((String) "WiFi mac address: " + WiFi.macAddress());
  Serial.println((String) "AutoConnect: " + WiFi.getAutoConnect());
  Serial.println((String) "AutoREConnect: " + WiFi.getAutoReconnect());
  Serial.println((String) "BSSID: " + WiFi.BSSIDstr());
  Serial.println((String) "RSSI: " + WiFi.RSSI());
  Serial.println((String) "Status: " + WiFi.status());
  Serial.println("IP:" + WiFi.localIP().toString());
  Serial.println("");
}

void initWiFi() {

  WiFi.disconnect();
  delay(100);

  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_5dBm);

  Serial.print("Connecting");
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if (attempt >= WIFI_CONNECT_TIMEOUT) {
      ESP.restart();
    }
    delay(100);
    ++attempt;
  }
}
void serverExchange() {
  serverGet();
  serverPost();
}

void serverPost() {
  double ads_value = analogRead(voltage_measuring_pin);
  double voltage = (ads_value * 3.3 / 4095) / 0.2;
  String path = serverName + apiPath + "/device/";

  StaticJsonDocument<1024> device;
  device["mac"] = WiFi.macAddress();
  device["firmwareUrlCurrent"] = firmwareUrl;
  if (deepSleep) {
    device["deepSleep"] = "true";
  } else {
    device["deepSleep"] = "false";
  }
  JsonArray indicators = device.createNestedArray("indicators");
  ///LED
  JsonObject indicator_0 = indicators.createNestedObject();
  indicator_0["name"] = "LED";
  indicator_0["pin"] = LED;
  indicator_0["currentState"] = digitalRead(LED);
  indicator_0["type"] = "Switch";
  ///DHT Temperature
  JsonObject indicator_1 = indicators.createNestedObject();
  indicator_1["name"] = "DHT22T";
  indicator_1["pin"] = DHTPIN;
  indicator_1["indication"] = dht.readTemperature();
  indicator_1["type"] = "Temperature";
  ///DHT Temperature
  JsonObject indicator_2 = indicators.createNestedObject();
  indicator_2["name"] = "DHT22H";
  indicator_2["pin"] = DHTPIN;
  indicator_2["indication"] = dht.readHumidity();
  indicator_2["type"] = "Humidity";
  //Voltage
  JsonObject indicator_3 = indicators.createNestedObject();
  indicator_3["name"] = "ACC";
  indicator_3["pin"] = voltage_measuring_pin;
  indicator_3["indication"] = voltage;
  indicator_3["type"] = "Voltage";

  String request;
  serializeJson(device, request);

  Serial.println("POST data: " + path);
  Serial.println("Request: " + request);
  HTTPClient http;

  http.begin(path.c_str());
  http.addHeader("Content-Type", "application/json");
  int httpResponceCode = http.POST(request);
  String payload = http.getString();
  if (httpResponceCode > 0) {
    Serial.println((String) "HTTP POST responce code: " + httpResponceCode);
    String payload = http.getString();
    Serial.println("Responce: " + payload);
  } else {
    Serial.println("Error code: " + http.errorToString(httpResponceCode));
  }
}

void serverGet() {
  //digitalWrite(LED, HIGH);
  HTTPClient http;
  String path = serverName + apiPath + "/device/" + WiFi.macAddress();
  http.begin(path.c_str());
  int httpResponceCode = http.GET();
  if (httpResponceCode > 0) {
    Serial.println((String) "HTTP GET responce code: " + httpResponceCode);
    String payload = http.getString();
    Serial.println("Responce: " + payload);
    StaticJsonDocument<200> jsonDoc;
    deserializeJson(jsonDoc, payload);
    JsonArray indicators = jsonDoc.as<JsonArray>();

    for (JsonObject indicator : indicators) {
      uint8_t pin = indicator["pin"].as<uint8_t>();
      uint8_t newState = indicator["targetState"].as<uint8_t>();
      Serial.println((String) "TargerState: " + newState + " High: " + HIGH);
      digitalWrite(pin, newState);
    }
  } else {
    Serial.println("Error code: " + http.errorToString(httpResponceCode));
  }
  //digitalWrite(LED, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  //analogReadResolution(12);
  pinMode(voltage_measuring_pin, INPUT);
  initWiFi();
  dht.begin();
}

void loop() {
  //printWiFiStatus();
  if (!WiFi.isConnected()) {
    Serial.println("WiFi reconnect");
    WiFi.reconnect();
    if (WiFi.isConnected()) {
      serverExchange();
    }
  } else {
    serverExchange();
  }
  //delay(10000);
  Serial.println((String) "Going to sleep for " + SECONDS_TO_SLEEP + " seconds.");
  Serial.flush();
  delay(100);
  if (deepSleep) {
    esp_sleep_enable_timer_wakeup(SECONDS_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  } else {
    delay(SECONDS_TO_SLEEP * mS_TO_S_FACTOR);
  }
}