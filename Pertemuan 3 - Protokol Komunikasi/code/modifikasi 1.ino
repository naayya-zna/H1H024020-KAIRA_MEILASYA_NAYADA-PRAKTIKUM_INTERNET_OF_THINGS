#include <ESP8266WiFi.h>          // Library untuk menghubungkan ESP8266 ke jaringan WiFi
#include <ESP8266HTTPClient.h>    // Library untuk melakukan komunikasi HTTP pada ESP8266
#include <WiFiClientSecure.h>     // Library untuk koneksi HTTPS yang aman
#include <ArduinoJson.h>          // Library untuk membuat dan mengolah data dalam format JSON

const char* ssid     = "rakjel";      // Nama jaringan WiFi yang akan digunakan
const char* password = "entersaja";   // Password jaringan WiFi
const char* serverUrl = "https://httpbin.org/post"; // Alamat server untuk menguji pengiriman HTTP POST

void setup() {
  Serial.begin(115200);           // Memulai komunikasi Serial Monitor dengan baud rate 115200
  WiFi.begin(ssid, password);     // Memulai proses koneksi ESP8266 ke jaringan WiFi
  Serial.print("Menghubungkan ke WiFi"); // Menampilkan pesan bahwa ESP8266 sedang mencoba terhubung

  while (WiFi.status() != WL_CONNECTED) { // Mengulang proses selama ESP8266 belum terhubung ke WiFi
    delay(500);                    // Memberikan jeda selama 500 milidetik
    Serial.print(".");             // Menampilkan titik sebagai tanda proses koneksi masih berlangsung
  }

  Serial.println();                       // Membuat baris baru pada Serial Monitor
  Serial.println("WiFi berhasil terhubung!"); // Menampilkan pesan bahwa koneksi WiFi berhasil
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { // Mengecek apakah ESP8266 masih terhubung ke WiFi
    WiFiClientSecure client;           // Membuat objek client untuk melakukan koneksi HTTPS
    client.setInsecure();              // Menonaktifkan verifikasi sertifikat SSL untuk keperluan pengujian

    HTTPClient http;                   // Membuat objek HTTPClient untuk komunikasi dengan server
    http.begin(client, serverUrl);     // Menghubungkan HTTPClient dengan server tujuan
    http.addHeader("Content-Type", "application/json"); // Memberi tahu server bahwa data yang dikirim berbentuk JSON

    JsonDocument doc;                  // Membuat dokumen JSON untuk menyimpan data yang akan dikirim
    doc["suhu"] = 28.5;                // Menambahkan data suhu sebesar 28,5 °C ke dalam JSON
    doc["kelembaban"] = 65.0;          // Menambahkan data kelembaban sebesar 65% ke dalam JSON
    doc["waktu"] = millis();           // MODIFIKASI: menambahkan timestamp (dalam milidetik sejak boot)
                                        // agar setiap data yang dikirim memiliki penanda waktu pengiriman

    String requestBody;                // Membuat variabel String untuk menampung data JSON
    serializeJson(doc, requestBody);   // Mengubah objek JSON menjadi teks/string yang siap dikirim

    Serial.print("Mengirim data: ");   // Menampilkan keterangan bahwa data akan dikirim
    Serial.println(requestBody);       // Menampilkan isi data JSON yang dikirim ke server

    int httpResponseCode = http.POST(requestBody); // Mengirim data JSON menggunakan metode HTTP POST dan menyimpan kode response dari server

    if (httpResponseCode > 0) { // Mengecek apakah server memberikan response
      Serial.print("Kode Response HTTP: "); // Menampilkan keterangan kode response HTTP
      Serial.println(httpResponseCode);      // Menampilkan kode response dari server
      Serial.println("Isi Response:");       // Menampilkan keterangan isi response
      Serial.println(http.getString());      // Mengambil dan menampilkan response dari server
    } else {
      Serial.print("Pengiriman gagal, kode error: "); // Menampilkan pesan jika pengiriman gagal
      Serial.println(httpResponseCode);                // Menampilkan kode error yang diperoleh
    }

    http.end(); // Mengakhiri koneksi HTTP dan membebaskan sumber daya
  }
  delay(10000); // Memberikan jeda 10 detik sebelum melakukan pengiriman berikutnya
}
