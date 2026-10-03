#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid     = "rakjel";
const char* password = "entersaja";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

const char* topicPerintah =
  "unsoed/tk245004/kelompok2/perintah";

const int ledPin = D5;

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi callback ketika pesan MQTT diterima
void callback(char* topic, byte* payload, unsigned int length) {

  String pesan;

  // Mengubah payload MQTT menjadi String
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  Serial.print("Pesan diterima: ");
  Serial.println(pesan);

  // Membuat objek JSON
  JsonDocument doc;

  // Melakukan parsing JSON
  DeserializationError error = deserializeJson(doc, pesan);

  // Mengecek apakah JSON valid
  if (error) {
    Serial.print("Gagal parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Mengambil nilai perintah dan intensitas
  const char* perintah = doc["perintah"];
  int intensitas = doc["intensitas"];

  // Jika perintah ON, LED dinyalakan sesuai nilai intensitas
  if (String(perintah) == "ON") {
    analogWrite(ledPin, intensitas);

    Serial.print("Aktuator: ON | Intensitas: ");
    Serial.println(intensitas);
  }

  // Jika perintah OFF, LED dimatikan
  else if (String(perintah) == "OFF") {
    analogWrite(ledPin, 0);

    Serial.println("Aktuator: OFF");
  }
}

// Fungsi untuk menghubungkan ESP8266 ke WiFi
void hubungkanWiFi() {

  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi berhasil terhubung!");
}

// Fungsi untuk menghubungkan ESP8266 ke broker MQTT
void hubungkanMQTT() {

  while (!client.connected()) {

    String clientId =
      "ESP8266Client-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      // Subscribe ke topic perintah
      client.subscribe(topicPerintah);

      Serial.print("Subscribe ke topic: ");
      Serial.println(topicPerintah);

    } else {

      Serial.print("Gagal, rc=");
      Serial.print(client.state());

      Serial.println(" coba lagi dalam 2 detik");

      delay(2000);
    }
  }
}

// Fungsi setup dijalankan satu kali
void setup() {

  Serial.begin(115200);

  // Mengatur LED sebagai output
  pinMode(ledPin, OUTPUT);

  // Memastikan LED mati saat awal
  analogWrite(ledPin, 0);

  // Menghubungkan ke WiFi
  hubungkanWiFi();

  // Mengatur broker MQTT
  client.setServer(mqttServer, mqttPort);

  // Menentukan fungsi callback
  client.setCallback(callback);
}

// Fungsi loop dijalankan terus-menerus
void loop() {

  // Mengecek koneksi MQTT
  if (!client.connected()) {
    hubungkanMQTT();
  }

  // Memproses pesan MQTT yang masuk
  client.loop();
}
