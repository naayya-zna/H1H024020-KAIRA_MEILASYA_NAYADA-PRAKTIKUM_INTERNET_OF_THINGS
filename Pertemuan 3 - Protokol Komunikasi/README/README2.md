## 1. Detail Percobaan

Percobaan kedua bertujuan untuk memahami dan mengimplementasikan komunikasi data
menggunakan protokol **MQTT** dengan pola **publish-subscribe**. ESP8266 bertindak sebagai
**publisher** yang mengirimkan data JSON (suhu dan kelembaban) ke sebuah **topic** pada
broker MQTT publik `broker.hivemq.com` (port `1883`). Data yang dipublikasikan kemudian
diverifikasi dengan aplikasi **MQTT client** yang bertindak sebagai **subscriber** pada topic
yang sama.

## 2. Library / Dependencies yang Diperlukan

| Library | Fungsi |
|---|---|
| `ESP8266WiFi.h` | Menghubungkan ESP8266 ke jaringan WiFi |
| `PubSubClient.h` | Implementasi protokol MQTT (connect, publish, subscribe, loop) untuk ESP8266 |
| `ArduinoJson.h` | Membuat dan mengolah data dalam format JSON |

Aplikasi MQTT client (mis. MQTT Explorer / MQTTX / MyMQTT) digunakan di sisi
subscriber untuk memverifikasi data yang dipublikasikan, terhubung ke broker dan
topic yang sama dengan yang digunakan ESP8266.

## 3. Penjelasan Code

### 3.1 Deklarasi & Konfigurasi
```cpp
const char* mqttServer = "broker.hivemq.com";
const int   mqttPort   = 1883;
const char* mqttTopic  = "unsoed/tk245004/kelompokAnda/sensor";

WiFiClient espClient;
PubSubClient client(espClient);
```
Selain kredensial WiFi, bagian ini mendefinisikan alamat broker MQTT, port yang digunakan
(`1883`, port standar MQTT tanpa enkripsi), serta topic tempat data dipublikasikan. Objek
`WiFiClient` dibungkus ke dalam objek `PubSubClient` agar dapat berkomunikasi dengan
protokol MQTT di atas koneksi TCP/WiFi.

### 3.2 Fungsi `hubungkanWiFi()`
Fungsi bantuan (helper) yang memulai `WiFi.begin(ssid, password)` dan menahan proses
menggunakan `while` sampai ESP8266 benar-benar terhubung ke jaringan WiFi, mirip dengan
proses pada Percobaan 1.

### 3.3 Fungsi `hubungkanMQTT()`
Fungsi bantuan untuk menghubungkan ESP8266 ke broker MQTT. Di dalamnya:
1. Membuat `clientId` unik secara acak (`"ESP8266Client-" + String(random(0xffff), HEX)`)
   agar tidak terjadi bentrok ID dengan client lain yang terhubung ke broker yang sama.
2. Mencoba melakukan koneksi dengan `client.connect(clientId.c_str())`.
3. Jika gagal, menampilkan kode error (`client.state()`) dan mencoba kembali setelah
   jeda 2 detik.

### 3.4 Fungsi `setup()`
Dijalankan sekali di awal. Memulai Serial Monitor, memanggil `hubungkanWiFi()`, kemudian
menentukan alamat broker dan port MQTT melalui `client.setServer(mqttServer, mqttPort)`.

### 3.5 Fungsi `loop()`
Dijalankan berulang. Alurnya:
1. Mengecek apakah koneksi MQTT masih aktif (`client.connected()`); jika tidak, memanggil
   `hubungkanMQTT()` untuk menyambung ulang.
2. Memanggil `client.loop()` agar proses komunikasi MQTT (keep-alive, dsb.) tetap berjalan.
3. Membuat data JSON berisi `suhu` dan `kelembaban`, lalu mengubahnya menjadi array
   karakter (`char buffer[128]`) menggunakan `serializeJson`.
4. Mempublikasikan data ke `mqttTopic` melalui `client.publish(mqttTopic, buffer)`.
5. Menampilkan informasi topic dan data yang dikirim ke Serial Monitor.
6. Memberi jeda 5 detik (`delay(5000)`) sebelum publish data berikutnya.

## 4. Penjelasan Percabangan / Conditional

| Percabangan | Lokasi | Penjelasan |
|---|---|---|
| `while (WiFi.status() != WL_CONNECTED)` | `hubungkanWiFi()` | Menahan program sampai ESP8266 terhubung ke WiFi, sama seperti pada Percobaan 1. |
| `while (!client.connected())` | `hubungkanMQTT()` | Mengulang proses koneksi ke broker MQTT selama client belum berhasil terhubung. |
| `if (client.connect(clientId.c_str())) { ... } else { ... }` | `hubungkanMQTT()` | Jika koneksi ke broker berhasil, tampilkan pesan sukses; jika gagal, tampilkan kode error dan tunggu 2 detik sebelum mencoba lagi. |
| `if (!client.connected())` | `loop()` | Mengecek status koneksi MQTT setiap iterasi loop; jika terputus, panggil kembali `hubungkanMQTT()` untuk menyambungkan ulang secara otomatis. |

## 5. Hasil Pengamatan (Ringkasan)

Pengujian dilakukan sebanyak 10 kali publish dengan interval ±10 detik. Broker selalu dalam
status **Terhubung**, dan seluruh data JSON `{"suhu":28.5,"kelembaban":65}` yang
dipublikasikan pada topic `unsoed/tk245004/kelompok2/sensor` berhasil diterima secara
identik oleh aplikasi **subscriber**, sehingga seluruh proses pengiriman berstatus **Berhasil**.

## 6. Kendala

Kendala pada Percobaan 2 masih sama dengan Percobaan 1, yaitu library `ArduinoJson`
tidak terdeteksi oleh Arduino IDE, sehingga proses kompilasi mengalami error yang sama.

## 7. Jawaban Pertanyaan Percobaan 2

1. **Topic** digunakan sebagai alamat/jalur untuk mengelompokkan pesan MQTT. Publisher
   mengirimkan data ke topic tertentu, sedangkan subscriber menerima data dari topic yang
   diikuti. Topic perlu dibuat unik agar data dari beberapa perangkat/kelompok tidak
   tercampur satu sama lain.
2. `client.loop()` digunakan untuk menjaga proses komunikasi MQTT tetap berjalan.
   Perintah ini dipanggil secara berulang di dalam `loop()` agar client dapat menangani
   komunikasi dengan broker (termasuk keep-alive) dan mempertahankan koneksi MQTT.
3. Apabila koneksi ke broker terputus, program mendeteksi bahwa `client.connected()`
   bernilai `false`. Program kemudian menjalankan fungsi `hubungkanMQTT()` sehingga
   perangkat mencoba melakukan koneksi ulang ke broker secara otomatis.

---

## 8. Pertanyaan Analisis Modul 3 (HTTP vs MQTT)

### 8.1 Ringkasan alur kedua percobaan
Pada percobaan pertama, komunikasi dilakukan menggunakan protokol **HTTP** dengan
metode **POST**. ESP8266 terlebih dahulu terhubung ke jaringan WiFi, kemudian membuat
data suhu dan kelembaban dalam format JSON, dan mengirimkannya ke server
`httpbin.org/post`. Hasil pengiriman dapat dilihat melalui Serial Monitor berupa data JSON
yang dikirim, kode response HTTP, serta response body dari server.

Pada percobaan kedua, komunikasi dilakukan menggunakan protokol **MQTT** dengan pola
**publish-subscribe**. ESP8266 terhubung ke WiFi, lalu terhubung ke broker MQTT
`broker.hivemq.com` melalui port `1883`. Data suhu dan kelembaban dibuat dalam format
JSON dan dipublikasikan ke topic MQTT yang telah ditentukan, kemudian dapat diterima oleh
subscriber yang melakukan subscribe pada topic tersebut.

### 8.2 Perbandingan overhead komunikasi HTTP vs MQTT
HTTP memiliki overhead komunikasi yang lebih besar karena setiap pengiriman data
dilakukan dengan pola **request-response**: ESP8266 harus mengirim request ke server dan
menunggu response. Selain data JSON, terdapat informasi tambahan pada komunikasi HTTP
seperti header yang menambah ukuran data yang dikirim. MQTT memiliki overhead yang
lebih kecil karena dirancang sebagai protokol komunikasi yang ringan untuk perangkat IoT.
MQTT menggunakan pola publish-subscribe, sehingga ESP8266 cukup melakukan publish
data ke topic dan broker akan meneruskannya kepada subscriber yang berlangganan topic
tersebut. Dengan demikian, pola MQTT lebih sederhana dan ringan untuk pengiriman data
IoT dibanding HTTP.

### 8.3 Protokol yang lebih cocok untuk data sensor berkelanjutan
**MQTT** lebih cocok digunakan, karena MQTT memiliki overhead yang relatif kecil dan
menggunakan koneksi yang dapat dipertahankan (persistent connection) antara perangkat
dengan broker. Data sensor cukup dipublikasikan ke topic tertentu sehingga dapat diterima
secara real-time oleh perangkat atau aplikasi yang melakukan subscribe, tanpa perlu
membuat koneksi baru setiap kali mengirim data seperti pada HTTP.

### 8.4 Peran JSON dalam komunikasi data IoT
JSON berperan sebagai format pertukaran data yang terstruktur dan mudah dipahami oleh
berbagai perangkat maupun aplikasi. Pada praktikum ini, data suhu dan kelembaban dari
ESP8266 dibuat dalam bentuk pasangan key-value, seperti `suhu` dan `kelembaban`.
Penggunaan JSON membuat data dari ESP8266 lebih mudah diterima dan diproses oleh
server, broker MQTT, maupun aplikasi lain yang mendukung format JSON. Dengan demikian,
perangkat IoT tidak harus menggunakan format data yang berbeda-beda untuk setiap
platform, sehingga JSON membantu meningkatkan **interoperabilitas**, yaitu kemampuan
berbagai sistem untuk saling bertukar dan memahami data.
