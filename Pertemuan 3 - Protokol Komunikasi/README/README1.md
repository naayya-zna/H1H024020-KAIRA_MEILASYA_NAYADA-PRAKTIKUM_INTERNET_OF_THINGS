## 1. Detail Percobaan

Percobaan pertama bertujuan untuk memahami dan mengimplementasikan proses komunikasi
data menggunakan protokol **HTTP** dengan metode **POST**. Perangkat ESP8266 mengirimkan
data sensor (suhu dan kelembaban) dalam format **JSON** menuju server pengujian
`https://httpbin.org/post` melalui jaringan WiFi. Setiap 10 detik, perangkat mengirim ulang
data JSON, lalu menampilkan kode response HTTP dan isi response dari server melalui
Serial Monitor.

## 2. Library / Dependencies yang Diperlukan

| Library | Fungsi |
|---|---|
| `ESP8266WiFi.h` | Menghubungkan ESP8266 ke jaringan WiFi |
| `ESP8266HTTPClient.h` | Melakukan komunikasi HTTP (GET/POST) dari ESP8266 |
| `WiFiClientSecure.h` | Membuat koneksi HTTPS (aman/terenkripsi) ke server |
| `ArduinoJson.h` | Membuat dan mengolah data dalam format JSON |

Semua library di atas dipasang melalui **Library Manager** pada Arduino IDE, kecuali
`ESP8266WiFi.h` dan `ESP8266HTTPClient.h` yang sudah termasuk dalam board package
ESP8266.

## 3. Penjelasan Code

### 3.1 Deklarasi & Konfigurasi
```cpp
const char* ssid     = "rakjel";
const char* password = "entersaja";
const char* serverUrl = "https://httpbin.org/post";
```
Bagian ini mendefinisikan kredensial WiFi (SSID dan password) serta alamat server tujuan
pengiriman data (`httpbin.org/post`, layanan publik untuk menguji request HTTP karena akan
mengembalikan/echo data yang dikirim).

### 3.2 Fungsi `setup()`
Fungsi ini dijalankan sekali saat ESP8266 pertama kali menyala/di-reset. Tugasnya:
1. Memulai komunikasi Serial Monitor dengan baud rate `115200`.
2. Memulai proses koneksi ke WiFi menggunakan `WiFi.begin(ssid, password)`.
3. Menunggu (melalui perulangan `while`) sampai status WiFi berubah menjadi `WL_CONNECTED`.
4. Menampilkan pesan konfirmasi bahwa WiFi berhasil terhubung.

### 3.3 Fungsi `loop()`
Fungsi ini berjalan berulang-ulang selama perangkat menyala. Alurnya:
1. Mengecek status koneksi WiFi.
2. Membuat objek `WiFiClientSecure` dan memanggil `client.setInsecure()` agar ESP8266
   tidak perlu memverifikasi sertifikat SSL server (dipakai untuk keperluan pengujian saja,
   **tidak disarankan untuk produksi**).
3. Membuat objek `HTTPClient` dan menghubungkannya ke `serverUrl` lewat `http.begin()`.
4. Menambahkan header `Content-Type: application/json` agar server tahu format data yang
   dikirim adalah JSON.
5. Membuat dokumen JSON (`JsonDocument doc`) berisi pasangan key-value `suhu` dan
   `kelembaban`, lalu mengubahnya menjadi String (`serializeJson`).
6. Mengirim data dengan `http.POST(requestBody)` dan menyimpan kode response yang
   dikembalikan server.
7. Menampilkan kode response dan isi response ke Serial Monitor.
8. Menutup koneksi dengan `http.end()` untuk membebaskan resource.
9. Memberi jeda 10 detik (`delay(10000)`) sebelum pengiriman berikutnya.

## 4. Penjelasan Percabangan / Conditional

| Percabangan | Lokasi | Penjelasan |
|---|---|---|
| `while (WiFi.status() != WL_CONNECTED)` | `setup()` | Menahan program agar tidak lanjut ke tahap berikutnya sebelum ESP8266 benar-benar terhubung ke WiFi. Setiap 500 ms dicetak tanda titik sebagai indikator proses masih berlangsung. |
| `if (WiFi.status() == WL_CONNECTED)` | `loop()` | Memastikan proses pengiriman HTTP POST hanya dilakukan jika WiFi dalam keadaan terhubung, mencegah error saat koneksi terputus. |
| `if (httpResponseCode > 0) { ... } else { ... }` | `loop()` | Membedakan status pengiriman: jika kode response lebih besar dari 0 berarti server memberikan balasan (berhasil terkirim), sedangkan jika bernilai negatif berarti terjadi kegagalan pada level koneksi. |

## 5. Hasil Pengamatan (Ringkasan)

Pengujian dilakukan sebanyak 10 kali pengiriman dengan interval 10 detik. Seluruh
pengiriman menghasilkan **HTTP Response Code 200** dan data JSON `{"suhu":28.5,"kelembaban":65}`
berhasil diterima kembali (echo) oleh `httpbin.org`, menandakan seluruh proses pengiriman
**berhasil**.

## 6. Modifikasi Program

Sesuai jawaban pertanyaan praktikum poin 4, dilakukan modifikasi dengan menambahkan
field waktu pengiriman ke dalam JSON menggunakan `millis()`:
```cpp
doc["waktu"] = millis();
```
Field ini mencatat waktu (dalam milidetik sejak ESP8266 menyala/reset) saat data dikirim,
sehingga setiap data yang di-POST memiliki penanda waktu.

## 7. Jawaban Pertanyaan Praktikum (Terkait Code)

1. **Flowchart proses pengiriman data HTTP POST**: Mulai → Inisialisasi Serial & WiFi →
   Hubungkan WiFi → (Loop) Apakah WiFi terhubung? → jika Tidak, kembali mencoba
   menghubungkan; jika Ya → Buat JSON (suhu & kelembaban) → Atur Header
   `Content-Type: application/json` → Kirim HTTP POST ke Server → Terima Response Kode
   & Body → Tampilkan Response di Serial Monitor → Delay 10 detik → kembali mengecek
   status WiFi (looping).
2. Perintah `http.addHeader("Content-Type", "application/json")` digunakan untuk
   memberitahukan kepada server bahwa data yang dikirimkan pada HTTP request memiliki
   format JSON.
3. HTTP Response Code `200` menunjukkan bahwa request yang dikirimkan berhasil diproses
   oleh server. Contoh kode lain: `404 Not Found`, yang menunjukkan resource/alamat yang
   diminta tidak ditemukan di server.
4. Modifikasi program: penambahan `doc["waktu"] = millis();` (lihat bagian 6 di atas).

## 8. Kendala

Kendala utama yang dihadapi adalah library `ArduinoJson` tidak terdeteksi oleh Arduino IDE,
sehingga kompilasi program gagal dengan pesan error `ArduinoJson.h: No such file or
directory`. Setelah ditelusuri, library sebenarnya sudah terpasang tetapi berada pada folder
yang berbeda dengan **Sketchbook Location** yang digunakan Arduino IDE. Sudah dicoba
memindahkan library ke `C:\Arduino\libraries`, namun tetap tidak terdeteksi. Atas saran
asisten praktikum, dicoba install ulang Arduino IDE, namun karena download manual juga
tidak bisa terdeteksi, akhirnya percobaan diselesaikan menggunakan laptop teman dari
kelompok lain.
