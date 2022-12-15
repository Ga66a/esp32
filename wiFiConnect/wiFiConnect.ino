#include <WiFi.h>

// Replace with your network credentials (STATION)
const char* ssid = "gabba";
const char* password = "qwerty123456";

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
    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(1000);
    }
  
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  initWiFi();
}

void loop() {
  printWiFiStatus();
  if (!WiFi.isConnected()){
    Serial.println("WiFi reconnect");
    WiFi.reconnect();
  }
  delay(5000);
  // put your main code here, to run repeatedly:
}