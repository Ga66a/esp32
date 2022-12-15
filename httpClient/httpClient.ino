#include <WiFi.h>
#include "HTTPClient.h"
#include "ArduinoJson.h"

#define LED 2
#define uS_TO_S_FACTOR 1000000
#define mS_TO_S_FACTOR 1000
#define SECONDS_TO_SLEEP 60
#define DEEP_SLEEP false

// Replace with your network credentials (STATION)
const char* ssid = "gabba";
const char* password = "qwerty123456";
const String serverName = "https://iot.cloud.ga66a.ru";
const String apiPath = "/api/v1/iot";

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
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
}
void serverExchange() {
  serverGet();
  serverPost();
}

void serverPost() {
  String path = serverName + apiPath + "/device/";
  // DynamicJsonBuffer jsonBuffer(1024);
  // JsonObject& device = jsonBuffer.createObject();
  // device["mak"] = WiFi.macAddress();
  // JsonArray& indicators = device.createNestedArray("indicators");

  // JsonObject& indicator_0 = indicators.createNestedObject();
  // indicator_0["name"] = "LED";
  // indicator_0["cuddentState"] = "0";
  // indicator_0["type"] = "Switch";
  StaticJsonDocument<200> device;
  device["mak"] = WiFi.macAddress();
  JsonArray indicators = device.createNestedArray("indicators");

  ///LED
  JsonObject indicator_0 = indicators.createNestedObject();
  indicator_0["name"] = "LED";
  indicator_0["pin"] = LED;
  indicator_0["currentState"] = digitalRead(LED);
  indicator_0["type"] = "Switch";

  ///TEST_TEST
  JsonObject indicator_1 = indicators.createNestedObject();
  indicator_1["name"] = "TEST SWITCH 2";
  indicator_1["currentState"] = "0";
  indicator_1["type"] = "Switch";

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
  initWiFi();
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
  Serial.println((String) "Going to sleep for "+SECONDS_TO_SLEEP+" seconds.");
  Serial.flush();
  delay(1000);
  if (DEEP_SLEEP) {
    esp_sleep_enable_timer_wakeup(SECONDS_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  } else {
    delay(SECONDS_TO_SLEEP * mS_TO_S_FACTOR);
  }
}