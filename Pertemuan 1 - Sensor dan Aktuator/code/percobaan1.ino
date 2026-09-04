#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Memulai akuisisi data sensor DHT22...");
}

void loop() {
  float totalSuhu = 0;
  float totalKelembaban = 0;
  int jumlahBaca = 5;

  for (int i = 0; i < jumlahBaca; i++) {
    float kelembaban = dht.readHumidity();
    float suhu = dht.readTemperature();

    if (isnan(kelembaban) || isnan(suhu)) {
      Serial.println("Gagal membaca data dari sensor DHT22!");
      return;
    }

    totalSuhu += suhu;
    totalKelembaban += kelembaban;

    delay(2000);
  }

  float rataSuhu = totalSuhu / jumlahBaca;
  float rataKelembaban = totalKelembaban / jumlahBaca;

  Serial.print("Rata-rata Suhu: ");
  Serial.print(rataSuhu);
  Serial.print(" °C, Rata-rata Kelembaban: ");
  Serial.print(rataKelembaban);
  Serial.println(" %");

  delay(2000);
}
