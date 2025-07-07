## Hi Everyone ╰(*°▽°*)╯🍉

Kita akan belajar menggunakan sensor MPU6050 dengan NodeMCU ESP8266 sebagai alat detektor getaran yang dapat diaplikasikan pada motor induksi di rumahmu. Ini berfungsi untuk mengetahui apakah motor induksi yang kalian miliki masih bagus atau bahkan sudah dalam kondisi abnormal dan butuh perawatan. Cara ini efektif dan mudah digunakan untuk kalian di rumah. Selamat mencoba 🥐.

Berikut langkah-langkah yang bisa kalian ikuti:
1. **Persiapkan alat dan bahan**:  
    - NodeMCU ESP8266  
    - Sensor MPU6050  
    - Kabel jumper  
    - OLED 0,96 inci  
    - Breadboard (opsional)  
    - Software Arduino IDE

2. **Rangkaian**:  
    - Hubungkan sensor MPU6050 ke NodeMCU ESP8266 sesuai dengan pin I2C (SDA ke D2, SCL ke D1, VCC ke 3.3V, GND ke GND).
    - Jika menggunakan OLED 0,96 inci, hubungkan ke NodeMCU ESP8266 sesuai dengan pin I2C (VCC ke 3.3V, GND ke GND, SDA ke D2, SCL ke D1).
    ![rangkaian](img/img.png)
    #### Konfigurasi Pinout

| Komponen        | NodeMCU ESP8266 | Keterangan      |
|-----------------|-----------------|-----------------|
| MPU6050 - VCC   | 3.3V            | Power           |
| MPU6050 - GND   | GND             | Ground          |
| MPU6050 - SDA   | D2 (GPIO4)      | I2C Data        |
| MPU6050 - SCL   | D1 (GPIO5)      | I2C Clock       |
| OLED - VCC      | 3.3V            | Power           |
| OLED - GND      | GND             | Ground          |
| OLED - SDA      | D2 (GPIO4)      | I2C Data (paralel dengan MPU6050) |
| OLED - SCL      | D1 (GPIO5)      | I2C Clock (paralel dengan MPU6050) |

---

3. **Kode Program**:  
    - Pastikan Anda sudah menginstal library yang dibutuhkan (lihat langkah 6).
    - Berikut contoh kode sederhana untuk membaca data getaran dari MPU6050 dan menampilkannya di Serial Monitor serta OLED:

<details>
<summary><strong>Lihat Contoh Kode Sederhana (klik untuk membuka/tutup)</strong></summary>

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MPU6050_ADDR 0x68
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
    Wire.begin();
    Serial.begin(115200);

    // Inisialisasi OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("OLED gagal (OLED failed)"));
        while (1);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Inisialisasi MPU6050
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x6B); // Register power management
    Wire.write(0);
    Wire.endTransmission();
}

void loop() {
    int16_t ax, ay, az;
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 6);
    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();

    // Nilai getaran di bawah ini adalah satuan G (gravitasi), BUKAN mm/s
    // Menghitung magnitudo percepatan (dalam satuan G), bukan kecepatan getaran (mm/s)
    float getaran = sqrt(ax * ax + ay * ay + az * az) / 16384.0;

    Serial.print("Getaran: ");
    Serial.println(getaran, 2);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Getaran:");
    display.setCursor(0, 10);
    display.print(getaran, 2);
    display.display();

    delay(500);
}
```

> **Catatan:**  
> - Kode di atas hanya menampilkan nilai getaran dalam satuan *G* (gravitasi), **bukan** mm/s.  
> - Jika ingin hasil dalam mm/s, gunakan kode versi lanjutan yang sudah dilengkapi filter, integrasi, dan perhitungan RMS.  
> - Anda dapat menemukan kode tersebut di folder [`Arduino IDE code/Vrms_Tampil_Oled-SerialMonitor-.ino`](./Arduino%20IDE%20code/Vrms_Tampil_Oled-SerialMonitor-.ino) pada repository ini jika Anda membutuhkan nilai getaran dalam satuan mm/s.

</details>

   



4. **Upload Program**:  
    - Gunakan Arduino IDE untuk mengunggah kode program pembacaan data getaran dari MPU6050 ke NodeMCU.
    - Unggah kode program ekstensi .ino pada folder Arduino IDE Code.

5. **Monitoring Data**:  
    - Buka Serial Monitor pada Arduino IDE untuk melihat data getaran yang terbaca dari sensor.  
    ![serial monitor](img/sm.png)
    - Jika Anda menggunakan Webserver, siapkan database Anda di MySQL dengan membuat tabel baru bernama "tgetaran" dan membuat kolom baru seperti pada gambar di bawah ini:  
    ![Database](img/tget.png)
    Ada 3 kolom yang dapat digunakan untuk menyimpan data, yaitu: Id, Waktu, Getaran.
    Berikut contoh tampilan web sederhana yang berhasil menampilkan data dari database.  
    ![web](img/web.png)

    **Catatan tambahan untuk Webserver:**
    - Anda dapat menggunakan ESP8266 untuk mengirim data ke server menggunakan HTTP POST/GET.
    - Pastikan NodeMCU terhubung ke WiFi dan endpoint API sudah tersedia untuk menerima data.
    - Contoh kode pengiriman data ke server dapat ditambahkan sesuai kebutuhan.

6. **Analisis**:  
    Amati perubahan data getaran. Jika nilai getaran di luar batas normal berdasarkan standar ISO 10816, berarti motor induksi perlu dicek atau dirawat.  
    ![iso](img/iso.jpg)
    - **Batas Normal Getaran**:  
      - Mesin kecil: ≤ 2.8 mm/s (baik), 2.8–7.1 mm/s (waspada), > 7.1 mm/s (bahaya)
      - Mesin besar: ≤ 4.5 mm/s (baik), 4.5–11.2 mm/s (waspada), > 11.2 mm/s (bahaya)
    - Lakukan perawatan jika nilai getaran melebihi batas yang ditentukan.

### Langkah Tambahan: Unduh Library

7. **Unduh Library yang Dibutuhkan**:
    - Buka Arduino IDE.
    - Pilih menu **Sketch > Include Library > Manage Libraries...**
    - Cari dan instal library berikut:
        - **MPU6050** (by Electronic Cats atau Jeff Rowberg)
        - **Adafruit SSD1306** (untuk OLED)
        - **Adafruit GFX Library** (pendukung OLED)

---

### Troubleshooting

- Jika data tidak muncul di Serial Monitor, pastikan baudrate sudah sesuai (115200).
- Jika OLED tidak menampilkan data, cek alamat I2C dan wiring.
- Jika NodeMCU tidak terdeteksi di Arduino IDE, pastikan driver sudah terinstal dan port sudah benar.

---

Semoga berhasil! Jika ada pertanyaan, silakan tinggalkan komentar. 🚀

