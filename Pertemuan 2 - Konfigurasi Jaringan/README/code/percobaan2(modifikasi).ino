#include <ESP8266WiFi.h> 

const char* sta_ssid     = "naye";          
const char* sta_password = "woylahbroo";    
const char* ap_ssid      = "ESP_Hotspot";   
const char* ap_password  = "12345678";      

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(sta_ssid, sta_password);
  WiFi.softAP(ap_ssid, ap_password);

  Serial.println("Mode AP+STA Aktif!");
  
  Serial.print("IP Mode Station (STA)    : ");
  Serial.println(WiFi.localIP());
  Serial.print("IP Mode Access Point (AP): ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  int jumlahClient = WiFi.softAPgetStationNum();
  Serial.print("Jumlah client terhubung ke AP: ");
  Serial.println(jumlahClient);
  
  delay(5000);
}
