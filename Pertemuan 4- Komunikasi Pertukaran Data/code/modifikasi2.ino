#include <ESP8266WiFi.h> 
#include <PubSubClient.h> 
#include <ArduinoJson.h> 
#include <DHT.h> 
 
const char* ssid     = "rakjel"; 
const char* password = "entersaja"; 
 
// Broker MQTT 
const char* mqttServer = "broker.hivemq.com"; 
const int mqttPort = 1883; 
 
// Topic MQTT 
const char* topicData       = "unsoed/tk245004/kelompok2/data"; 
const char* topicPerintah1  = "unsoed/tk245004/kelompok2/perintah1"; 
const char* topicPerintah2  = "unsoed/tk245004/kelompok2/perintah2"; 
 
// Sensor DHT11 
#define DHTPIN 4 
#define DHTTYPE DHT11 
 
// Aktuator 
const int ledPin1 = D5;   // LED pertama 
const int ledPin2 = D6;   // LED kedua 
 
DHT dht(DHTPIN, DHTTYPE); 
 
WiFiClient espClient; 
PubSubClient client(espClient); 
 
// Variabel publish non-blocking 
unsigned long waktuTerakhirPublish = 0; 
const long intervalPublish = 5000; 
 
// Callback ketika menerima pesan MQTT 
void callback(char* topic, byte* payload, unsigned int length) { 
 
  String pesan = ""; 
 // Mengubah payload menjadi String 
  for (unsigned int i = 0; i < length; i++) { 
    pesan += (char)payload[i]; 
  } 
 
  // Parsing JSON 
  JsonDocument doc; 
 
  if (deserializeJson(doc, pesan)) { 
    return; 
  } 
 
  // Mengambil isi perintah 
  const char* perintah = doc["perintah"]; 
 
  // Mengontrol LED pertama 
  if (String(topic) == topicPerintah1) { 
 
    if (String(perintah) == "ON") { 
      digitalWrite(ledPin1, HIGH); 
    } else { 
      digitalWrite(ledPin1, LOW); 
    } 
 
    Serial.print("LED 1 : "); 
    Serial.println(perintah); 
  } 
 
  // Mengontrol LED kedua 
  else if (String(topic) == topicPerintah2) { 
 
    if (String(perintah) == "ON") { 
      digitalWrite(ledPin2, HIGH); 
    } else { 
      digitalWrite(ledPin2, LOW); 
    } 
 
    Serial.print("LED 2 : "); 
    Serial.println(perintah); 
  } 
} 
 
// Koneksi WiFi 
void hubungkanWiFi() { 
 
  WiFi.begin(ssid, password); 
 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
 } 
 
  Serial.println("WiFi berhasil terhubung!"); 
} 
 
// Koneksi MQTT 
void hubungkanMQTT() { 
 
  while (!client.connected()) { 
 
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX); 
 
    if (client.connect(clientId.c_str())) { 
 
      // Subscribe kedua topic 
      client.subscribe(topicPerintah1); 
      client.subscribe(topicPerintah2); 
 
      Serial.println("Subscribe topic LED 1 dan LED 2"); 
 
    } else { 
      delay(2000); 
    } 
  } 
} 
 
// Setup 
void setup() { 
 
  Serial.begin(115200); 
 
  pinMode(ledPin1, OUTPUT); 
  pinMode(ledPin2, OUTPUT); 
 
  dht.begin(); 
 
  hubungkanWiFi(); 
 
  client.setServer(mqttServer, mqttPort); 
  client.setCallback(callback); 
} 
 
// Loop 
void loop() { 
 
  // Cek koneksi MQTT 
  if (!client.connected()) { 
    hubungkanMQTT(); 
  } 
 
  // Memproses pesan MQTT 
  client.loop(); 
 
  // Publish data suhu setiap 5 detik 
  if (millis() - waktuTerakhirPublish > intervalPublish) { 
 
    waktuTerakhirPublish = millis(); 
 
    float suhu = dht.readTemperature(); 
 
    if (!isnan(suhu)) { 
 
      JsonDocument doc; 
 
      doc["suhu"] = suhu; 
 
      char buffer[128]; 
 
      serializeJson(doc, buffer); 
 
      client.publish(topicData, buffer); 
 
      Serial.print("Data terkirim: "); 
      Serial.println(buffer); 
    } 
  } 
}
