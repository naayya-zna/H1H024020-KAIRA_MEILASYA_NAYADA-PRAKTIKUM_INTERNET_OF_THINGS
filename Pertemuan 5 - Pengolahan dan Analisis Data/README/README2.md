# Modul 5B – Akuisisi, Analisis, dan Visualisasi Data Suhu melalui MQTT

**Mata Kuliah:** TK245005 | **Semester:** 2026/5 | **Praktikan:** Kaira Meilasya Nayada (H1H024020)

---

## 1. Detail Percobaan

Percobaan kedua (5B) bertujuan untuk memahami proses pengiriman dan penerimaan data suhu menggunakan protokol **MQTT**, penyimpanan data ke dalam file **CSV**, perhitungan statistik, deteksi anomali menggunakan metode **Z-score**, serta visualisasi data suhu menggunakan **Python**. ESP8266 bertindak sebagai **publisher** yang mengirim data suhu mock (JSON) ke broker `broker.hivemq.com` pada topic `unsoed/tk245004/kelompok2/data`. Skrip Python di Visual Studio Code bertindak sebagai **subscriber** pada topic yang sama, kemudian menganalisis dan memvisualisasikan data yang diterima.

### Rangkaian

| No | Komponen | Keterangan |
|---|---|---|
| 1 | ESP8266 (NodeMCU) | Pengirim data suhu mock (publisher) |
| 2 | WiFi | Internet |
| 3 | MQTT Broker | broker.hivemq.com (port 1883) |
| 4 | Komputer | Menjalankan skrip Python (subscriber dan analisis) |

## 2. Library / Dependencies yang Diperlukan

### Arduino IDE (ESP8266)

| Library | Fungsi |
|---|---|
| `ESP8266WiFi.h` | Menghubungkan ESP8266 ke jaringan WiFi |
| `PubSubClient.h` | Komunikasi menggunakan protokol MQTT (publish/subscribe) |
| `ArduinoJson.h` | Membuat (serialisasi) data JSON |
| `math.h` | Fungsi `sin()` untuk membangkitkan suhu mock |

`ESP8266WiFi.h` sudah termasuk dalam board package ESP8266, sedangkan `PubSubClient` dan `ArduinoJson` dipasang melalui **Library Manager** pada Arduino IDE.

### Python (VS Code)

| Library | Fungsi |
|---|---|
| `json` | Mengolah data berformat JSON |
| `time` | Mengambil waktu penerimaan data |
| `paho-mqtt` | Komunikasi MQTT di sisi Python |
| `pandas` | Mengolah data dalam bentuk tabel (DataFrame) |
| `matplotlib` | Membuat grafik suhu dan anomali |

Instalasi: `pip install paho-mqtt pandas matplotlib`

## 3. Penjelasan Code

### 3.1 Arduino IDE (Publisher)

#### 3.1.1 Deklarasi & Konfigurasi

```cpp
const char* ssid       = "dips!";
const char* password   = "12345678";
const char* mqttServer = "broker.hivemq.com";
const int   mqttPort   = 1883;
const char* topicData  = "unsoed/tk245004/kelompok2/data";
```

Bagian ini mendefinisikan kredensial WiFi, alamat dan port broker MQTT, serta topic tujuan pengiriman data. Objek `WiFiClient espClient` dan `PubSubClient client(espClient)` dibuat sebagai dasar koneksi jaringan dan MQTT.

#### 3.1.2 Fungsi `bacaSuhuMock()`

Pengganti sensor DHT22. Suhu dibentuk dari suhu dasar `28.0 + 2.0 * sin(jumlahBaca * 0.08)`, ditambah noise `random(-5, 6) * 0.1`, dan lonjakan `+8.0` setiap 25 pembacaan sebagai anomali buatan.

#### 3.1.3 Fungsi `setupWiFi()`

Mengatur mode `WIFI_STA`, memanggil `WiFi.begin(ssid, password)`, menunggu sampai status `WL_CONNECTED` (cetak titik setiap 500 ms), lalu menampilkan pesan berhasil dan alamat IP ESP8266.

#### 3.1.4 Fungsi `reconnectMQTT()`

Selama client belum terhubung ke broker, fungsi ini:

1. Membuat `clientId` dari ID chip (`"ESP8266-Mock-" + String(ESP.getChipId(), HEX)`).
2. Mencoba `client.connect(clientId.c_str())`.
3. Jika berhasil, menampilkan `MQTT berhasil terhubung!`.
4. Jika gagal, menampilkan kode `client.state()` dan mencoba lagi setelah 5 detik.

#### 3.1.5 Fungsi `setup()`

1. `Serial.begin(115200)` dan `delay(1000)`.
2. `randomSeed(micros())`.
3. `setupWiFi()`.
4. `client.setServer(mqttServer, mqttPort)`.
5. `reconnectMQTT()`.

#### 3.1.6 Fungsi `loop()`

1. Memeriksa dan menyambungkan kembali WiFi dan MQTT jika terputus.
2. `client.loop()` untuk menjaga komunikasi MQTT tetap berjalan.
3. Membaca suhu dari `bacaSuhuMock()`.
4. Membuat JSON `{"suhu": <nilai>}` dengan `StaticJsonDocument<128>` dan `serializeJson()`.
5. `client.publish(topicData, payload)`.
6. Menampilkan nomor pembacaan, suhu, topic, dan status publish ke Serial Monitor.
7. `delay(5000)` sebelum pengiriman berikutnya.

### 3.2 Python (Subscriber & Analisis)

| Tahap | Penjelasan |
|---|---|
| **Konfigurasi** | `BROKER`, `PORT`, `TOPIC` harus sama dengan sisi Arduino; `JUMLAH_SAMPEL = 30` sebagai jumlah data yang diterima. |
| **`on_connect()`** | Jika `reason_code == 0`, lakukan `client.subscribe(TOPIC)`. |
| **`on_message()`** | Decode payload, `json.loads()`, ambil `payload["suhu"]`, lalu simpan `{waktu, suhu}` ke list `data`. Data tidak valid diabaikan lewat `try-except`. Koneksi diputus (`disconnect()`) setelah 30 sampel terkumpul. |
| **Tahap 1 – Akuisisi** | Membuat klien `mqtt.Client` (callback API versi 2), `connect()`, lalu `loop_forever()`. |
| **Tahap 2 – Analisis** | Membuat DataFrame; menghitung rata-rata dan standar deviasi; menghitung **Z-score** `(suhu - rata2) / std`; kolom `anomali = abs(zscore) > 2`; menyimpan ke `data_suhu.csv`; mencetak ringkasan hasil analisis. |
| **Tahap 3 – Visualisasi** | Plot garis suhu dengan marker, titik anomali berwarna merah, dan garis rata-rata putus-putus; grafik disimpan ke `grafik_suhu.png` lalu ditampilkan. |

## 4. Penjelasan Percabangan / Conditional

### Arduino IDE

| Percabangan | Lokasi | Penjelasan |
|---|---|---|
| `while (WiFi.status() != WL_CONNECTED)` | `setupWiFi()` | Menahan program sampai ESP8266 terhubung ke WiFi. |
| `while (!client.connected())` | `reconnectMQTT()` | Mengulang percobaan koneksi ke broker sampai berhasil. |
| `if (client.connect(...)) { ... } else { ... }` | `reconnectMQTT()` | Jika berhasil, tampilkan pesan sukses; jika gagal, tampilkan kode error dan tunggu 5 detik. |
| `if (WiFi.status() != WL_CONNECTED)` | `loop()` | Menyambungkan kembali WiFi jika terputus. |
| `if (!client.connected())` | `loop()` | Menyambungkan kembali ke broker MQTT jika koneksi terputus. |
| `if (jumlahBaca % 25 == 0)` | `bacaSuhuMock()` | Menyisipkan lonjakan suhu +8 °C sebagai anomali buatan. |
| `berhasil ? "Berhasil" : "Gagal"` | `loop()` | Menampilkan status hasil `publish`. |

### Python

| Percabangan | Lokasi | Penjelasan |
|---|---|---|
| `if reason_code == 0` | `on_connect()` | Subscribe hanya dilakukan jika koneksi ke broker berhasil. |
| `try ... except (...)` | `on_message()` | Data dengan format tidak sesuai atau atribut `suhu` yang tidak ada akan diabaikan. |
| `if len(data) >= JUMLAH_SAMPEL` | `on_message()` | Memutus koneksi setelah jumlah sampel terpenuhi. |
| `if len(data) == 0 ... else` | Tahap 2 | Jika tidak ada data, tampilkan peringatan; jika ada, lanjut ke analisis. |
| `if pd.isna(std) or std == 0` | Tahap 2 | Z-score diisi 0 agar tidak terjadi pembagian dengan nol. |
| `df["zscore"].abs() > 2` | Tahap 2 | Menandai data sebagai anomali bila \|z\| > 2. |

## 5. Hasil Pengamatan (Ringkasan)

Pengamatan dilakukan terhadap 20 data suhu dengan interval pengiriman 5 detik.

| No | Waktu Pengiriman | Suhu (°C) | Z-score | Status Anomali | Status CSV |
|---|---|---|---|---|---|
| 1 | 12:13:12 | 28,66 | -0,71 | Tidak | Tersimpan |
| 2 | 12:13:17 | 28,32 | -0,92 | Tidak | Tersimpan |
| 3 | 12:13:22 | 28,88 | -0,57 | Tidak | Tersimpan |
| 4 | 12:13:27 | 28,43 | -0,85 | Tidak | Tersimpan |
| 5 | 12:13:32 | 28,88 | -0,57 | Tidak | Tersimpan |
| 6 | 12:13:37 | 28,72 | -0,67 | Tidak | Tersimpan |
| 7 | 12:13:42 | 28,86 | -0,59 | Tidak | Tersimpan |
| 8 | 12:13:47 | 29,29 | -0,32 | Tidak | Tersimpan |
| 9 | 12:13:52 | 29,22 | -0,36 | Tidak | Tersimpan |
| 10 | 12:13:57 | 29,73 | -0,05 | Tidak | Tersimpan |
| 11 | 12:14:02 | 30,00 | 0,14 | Tidak | Tersimpan |
| 12 | 12:14:07 | 29,56 | -0,10 | Tidak | Tersimpan |
| 13 | 12:14:12 | 29,82 | -0,18 | Tidak | Tersimpan |
| 14 | 12:14:17 | 30,00 | -0,12 | Tidak | Tersimpan |
| 15 | 12:14:22 | 29,56 | -0,15 | Tidak | Tersimpan |
| 16 | 12:14:27 | 29,82 | 0,01 | Tidak | Tersimpan |
| 17 | 12:14:32 | 29,46 | -0,21 | Tidak | Tersimpan |
| 18 | 12:14:37 | 29,78 | -0,02 | Tidak | Tersimpan |
| 19 | 12:14:42 | 29,90 | -0,06 | Tidak | Tersimpan |
| 20 | 12:14:47 | 30,30 | -0,31 | Tidak | Tersimpan |

**Hasil analisis data:**

| Parameter | Hasil |
|---|---|
| Jumlah data | 20 |
| Rata-rata suhu | 29,81 °C |
| Standar deviasi | 1,62 |
| Suhu minimum | 28,32 °C |
| Suhu maksimum | 30,30 °C |
| Jumlah anomali | 0 |

## 6. Modifikasi Program

Sesuai jawaban pertanyaan praktikum poin 4, ditambahkan perhitungan **Moving Average** (jendela 5 data) pada Python, lalu membandingkan jumlah anomali **sebelum** dan **sesudah** Moving Average:

```python
# Menghitung Moving Average dengan jendela 5 data
# min_periods=1 membuat data awal tetap dapat dihitung
df["suhu_ma"] = df["suhu"].rolling(
    window=5,
    min_periods=1
).mean()

# Menghitung rata-rata dan standar deviasi suhu mentah
rata2_mentah = df["suhu"].mean()
std_mentah = df["suhu"].std()

# Menghitung Z-score suhu mentah
if pd.isna(std_mentah) or std_mentah == 0:
    df["zscore_mentah"] = 0.0
else:
    df["zscore_mentah"] = (
        df["suhu"] - rata2_mentah
    ) / std_mentah

# Menentukan anomali sebelum proses Moving Average
df["anomali_mentah"] = df["zscore_mentah"].abs() > 2

# Menghitung rata-rata dan standar deviasi data hasil filter
rata2_ma = df["suhu_ma"].mean()
std_ma = df["suhu_ma"].std()

# Menghitung Z-score setelah Moving Average
if pd.isna(std_ma) or std_ma == 0:
    df["zscore_ma"] = 0.0
else:
    df["zscore_ma"] = (
        df["suhu_ma"] - rata2_ma
    ) / std_ma

# Menentukan anomali setelah proses Moving Average
df["anomali_ma"] = df["zscore_ma"].abs() > 2

# Membandingkan jumlah anomali sebelum dan sesudah filter
jumlah_anomali_mentah = int(df["anomali_mentah"].sum())
jumlah_anomali_ma = int(df["anomali_ma"].sum())

print("===== PERBANDINGAN ANOMALI =====")
print(f"Anomali sebelum Moving Average: {jumlah_anomali_mentah}")
print(f"Anomali setelah Moving Average: {jumlah_anomali_ma}")

# Menyimpan data mentah dan hasil filter ke CSV
df.to_csv("data_suhu.csv", index=False)
print("Data hasil analisis tersimpan di data_suhu.csv")
```

## 7. Jawaban Pertanyaan Praktikum (Terkait Code)

1. **Fungsi blok `try-except`:** digunakan untuk menangani kesalahan saat menerima dan memproses pesan MQTT. Pada `on_message()`, blok `try` mencoba mengubah pesan JSON menjadi data Python dan mengambil nilai suhu. Jika data tidak valid, misalnya format JSON salah atau atribut `suhu` tidak tersedia, blok `except` menampilkan pesan kesalahan dan mengabaikan data tersebut. Apabila blok `try-except` dihilangkan, kesalahan pemrosesan dapat menghentikan pemrosesan pesan dan menimbulkan gangguan pada program.

2. **Z-score dan batas \|z\| > 2:** Z-score menunjukkan seberapa jauh suatu nilai dari rata-rata dalam satuan standar deviasi. Z-score positif berarti data di atas rata-rata, sedangkan negatif berarti di bawah rata-rata. Batas \|z\| > 2 menandai data yang berjarak lebih dari dua standar deviasi dari rata-rata sebagai anomali, sehingga membantu mengidentifikasi nilai yang berbeda secara signifikan dari sebagian besar data. Namun, hasil deteksi tetap bergantung pada distribusi dan karakteristik data.

3. **Kelemahan Z-score pada data bertren:** Z-score menggunakan rata-rata dan standar deviasi keseluruhan data sebagai acuan. Apabila data memiliki tren naik atau turun yang tajam, perubahan normal akibat tren dapat terdeteksi sebagai anomali. Sebaliknya, anomali tertentu dapat tidak terdeteksi karena rata-rata dan standar deviasi ikut dipengaruhi perubahan data. Oleh karena itu, metode ini kurang sesuai untuk data bertren kuat tanpa pengolahan tambahan.

4. **Modifikasi program:** penambahan Moving Average dan perbandingan jumlah anomali sebelum dan sesudah filter (lihat bagian 6).

## 8. Kendala

Kendala yang dihadapi adalah memastikan koneksi antara ESP8266, broker MQTT, dan Python di VS Code berjalan dengan baik. Selain itu, data suhu yang diterima Python tidak sama dengan data yang ditampilkan pada Serial Monitor Arduino IDE, sehingga kesesuaian data antara pengirim dan penerima perlu diperiksa kembali.

---

## 9. Pertanyaan Analisis Modul 5

### 9.1 Ringkasan alur kedua percobaan

Pada Percobaan 5A, data suhu simulasi diolah menggunakan metode Moving Average untuk menghaluskan perubahan suhu. Program juga menghitung nilai suhu minimum dan maksimum selama proses berjalan.

Pada Percobaan 5B, ESP8266 mengirimkan data suhu simulasi ke broker MQTT HiveMQ. Data diterima oleh Python di VS Code, kemudian dianalisis menggunakan statistik dan metode Z-score untuk mendeteksi anomali. Hasil analisis disimpan dalam file CSV dan divisualisasikan dalam bentuk grafik.

### 9.2 Pengolahan data di sisi perangkat vs di sisi komputer

Pengolahan data di sisi perangkat dilakukan langsung pada mikrokontroler sehingga data dapat diproses dengan cepat tanpa harus selalu dikirim ke komputer. Namun, kemampuan pemrosesannya terbatas karena kapasitas memori dan sumber daya mikrokontroler relatif kecil.

Sebaliknya, pengolahan data di sisi komputer dapat menggunakan pustaka seperti Pandas dan Matplotlib untuk melakukan analisis statistik dan visualisasi yang lebih kompleks. Komputer memiliki sumber daya yang lebih besar, tetapi prosesnya membutuhkan komunikasi data dan koneksi antara perangkat dengan komputer.

### 9.3 Pengaruh ukuran jendela Moving Average

Ukuran jendela Moving Average memengaruhi tingkat kehalusan data dan kecepatan respons sistem kendali. Jendela yang besar menghasilkan data lebih halus karena fluktuasi lebih teredam, tetapi respons terhadap perubahan kondisi menjadi lebih lambat. Sebaliknya, jendela yang kecil membuat sistem lebih cepat merespons perubahan, tetapi data lebih mudah terpengaruh noise.

### 9.4 Manfaat analisis data dan deteksi anomali pada sistem IoT

Analisis data dan deteksi anomali membantu mengidentifikasi kondisi yang tidak normal pada sistem IoT. Pada sistem *smart farming*, perubahan suhu atau kelembapan yang tidak wajar dapat menjadi peringatan agar kondisi lingkungan segera diperiksa. Pada sistem *smart home*, deteksi anomali dapat membantu mengenali perubahan suhu ruangan yang tidak biasa sehingga sistem dapat memberikan peringatan atau menyesuaikan kerja aktuator.
