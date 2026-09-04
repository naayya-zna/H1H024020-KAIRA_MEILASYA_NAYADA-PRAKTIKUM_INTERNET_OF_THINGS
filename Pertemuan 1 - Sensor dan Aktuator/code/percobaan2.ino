#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define RELAYPIN 26

DHT dht(DHTPIN, DHTTYPE);

const float suhuOn = 30.0;
const float suhuOff = 28.0;

bool statusRelay = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);

  Serial.println("Memulai kendali aktuator relay...");
}

void loop() {
  float suhu = dht.readTemperature();

  if (isnan(suhu)) {
    Serial.println("Gagal membaca data sensor!");
  } 
  else {
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C -> ");

    if (suhu > suhuOn) {
      statusRelay = true;
    } 
    else if (suhu < suhuOff) {
      statusRelay = false;
    }

    if (statusRelay) {
      digitalWrite(RELAYPIN, HIGH);
      Serial.println("Aktuator: ON");
    } 
    else {
      digitalWrite(RELAYPIN, LOW);
      Serial.println("Aktuator: OFF");
    }
  }

  delay(2000);
}
