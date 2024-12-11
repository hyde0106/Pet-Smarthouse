# Pet Smart Home
## Introduction to the Problem and the Solution

Dalam era modern yang serba cepat, banyak pemilik hewan peliharaan menghadapi tantangan dalam merawat hewan kesayangan mereka. Kesibukan sehari-hari membuat pemilik hewan kesulitan untuk mengecek jadwal makan, memantau kondisi lingkungan, serta memastikan keamanan hewan peliharaan. Masalah ini dapat berdampak pada kesehatan dan kesejahteraan hewan.

**Pet Smart Home** hadir sebagai solusi berbasis IoT untuk mempermudah perawatan hewan peliharaan. Sistem ini mengintegrasikan berbagai teknologi pintar untuk otomatisasi jadwal makan, pengontrolan pencahayaan, pemantauan suhu kandang, dan pelacakan lokasi hewan. Sistem ini dirancang agar dapat dioperasikan secara mudah melalui aplikasi **Blynk**, yang memungkinkan kontrol jarak jauh, pemberitahuan real-time, serta pemantauan kondisi lingkungan hewan.

## Hardware Design and Implementation Details

Desain hardware **Pet Smart Home** bertujuan untuk mendukung semua fitur utama dengan komponen elektronik berikut ini. 

1. **ESP32**  
   Mikrokontroler utama dengan konektivitas Wi-Fi dan BLE untuk mengintegrasikan fitur perangkat keras dengan aplikasi Blynk.

2. **Sensor DHT11/DHT22**  
   Digunakan untuk memantau suhu dan kelembapan kandang hewan. Data dari sensor ini mengontrol kipas secara otomatis untuk menjaga kenyamanan suhu kandang.

3. **Servo Motor**  
   Mengoperasikan mekanisme hidup/mati kipas secara otomatis dengan menggerakkan servo ketika suhu lingkungan sudah di atas 30°C.

4. **Photoresistor**  
   Mendeteksi tingkat pencahayaan di kandang dan mengontrol LED untuk pencahayaan otomatis atau manual melalui aplikasi.

5. **LED**  
   Memberikan pencahayaan di kandang yang dapat diatur intensitasnya secara otomatis atau manual menggunakan slider pada aplikasi.

6. **BLE Beacon**  
   Dipasang pada tubuh hewan (misalnya, di kalung) untuk memantau posisi hewan. Jika hewan keluar dari radius tertentu, sistem akan memberikan peringatan.

7. **Buzzer**  
   Digunakan sebagai peringatan tambahan saat hewan tidak terdeteksi dalam radius tertentu selama periode waktu tertentu.

**Skematik Rangkaian Hardware**

![Wokwi Schematic](https://github.com/user-attachments/assets/10bb1063-3a1a-444a-9531-ed18953e3e1a)
Rangkaian pertama berfungsi untuk menerapkan fitur jadwal pengingat makan, pemantauan intensitas cahaya dan suhu, serta menyalakan LED dan servo berdasarkan hasil pemantauan menggunakan photoresistor dan DHT11/DHT22.

![Wokwi Schematic](https://github.com/user-attachments/assets/0b422863-51aa-4875-8952-84cf8f8efab3)
Rangkaian kedua berfungsi untuk menerapkan fitur pelacakan hewan peliharaan. Jika ESP32 yang kedua jaraknya semakin jauh dengan ESP32 yang pertama, ESP32 yang pertama akan menyalakan buzzer. 

## Software Implementation Details

Pengembangan perangkat lunak **Pet Smart Home** bertujuan untuk mengintegrasikan fungsi hardware dengan sistem IoT berbasis aplikasi **Blynk**. Berikut ini merupakan fitur utama dan kode terkait fitur tersebut. 

1. **Notifikasi dan Pemberian Makan**  
  Sistem memantau jadwal makan secara real-time dan mengirimkan notifikasi ke aplikasi Blynk. Pengguna dapat memberi makan hewan hanya ketika waktu makan telah tiba, yakni pada jam 8 dan 12. Caranya dengan menekan tombol `Feed` di Blynk maksimal lima kali.
  ```
  void checkFeedingTime() {
     int currentHour = hour();  // Dapatkan jam saat ini dari RTC

     if (currentHour == 8 || currentHour == 12) {  // Contoh jadwal makan
        if (!isFeedingTime) {                       // Jika sudah waktu makan
           isFeedingTime = true;
           feedButtonPressCount = 0;
           Serial.println("Notifikasi: Waktunya memberi makan hewan!");
        }
     } else {
        if (isFeedingTime) {  // Jika belum waktu makan
           isFeedingTime = false;
           feedButtonPressCount = 0;  // Reset hitungan tombol di luar waktu makan
           Serial.println("Notifikasi: Di luar waktu makan, hitungan tombol direset.");
        }
     }
  }
  ```

2. **Pengaturan Pencahayaan**  
  Sistem menggunakan photoresistor untuk mendeteksi intensitas cahaya. Pengguna dapat mengontrol pencahayaan melalui aplikasi atau mengandalkan sistem otomatis.
  ```
  void updateLampStatus() {
     int ldrValue = analogRead(PHOTORESISTOR_PIN);     // Baca nilai dari sensor photoresistor
     int lightValue = map(ldrValue, 0, 4095, 255, 0);  // Konversi ke rentang 0-255

     if (lampStatus) {                    // Jika lampu diaktifkan
        analogWrite(LED_PIN, lightValue);  // Atur intensitas LED sesuai cahaya ruangan
        Serial.printf("Lamp ON - LDR Value: %d, LED Intensity: %d\n", ldrValue, lightValue);
     } else {
        analogWrite(LED_PIN, 0);  // Matikan LED
        Serial.println("Lamp OFF");
     }

     // Perbarui status ke aplikasi Blynk
     Blynk.virtualWrite(V0, lampStatus ? "ON" : "OFF");
     Blynk.virtualWrite(V1, brightness);
  }
  ```

3. **Pemantauan Suhu Kandang**  
  Data dari sensor DHT11/DHT22 digunakan untuk mengaktifkan motor servo secara otomatis saat suhu melebihi ambang batas 30°C.
  ```
  void readDHT(void *pvParameters) {
     servoMotor.attach(SERVO_PIN);  // Inisialisasi servo motor
     servoMotor.write(0);           // Posisi awal kipas (mati)

     while (true) {
        float temperature = dht.getTemperature();  // Variabel untuk menyimpan nilai suhu
        float humidity = dht.getHumidity();        // Variabel untuk menyimpan nilai kelembaban

        if (isnan(temperature) || isnan(humidity)) {  // Periksa jika pembacaan gagal
           Serial.println("Gagal membaca data dari DHT11!");
        } else {  // Jika pembacaan berhasil, simpan nilai ke Blynk
           Serial.printf("Suhu: %.1f°C, Kelembapan: %.1f%%\n", temperature, humidity);
           Blynk.virtualWrite(V3, temperature);  // Nilai suhu yang ditampilkan ke Blynk
           Blynk.virtualWrite(V4, humidity);     // Nilai kelembaban yang ditampilkan ke Blynk

           // Atur hidup/mati kipas berdasarkan suhu
           if (temperature >= 30.0) {
              if (!fanState) {
                 fanState = true;
                 servoMotor.write(90);  // Aktifkan kipas
                 Serial.println("Kipas menyala!");
              }
           } else {
              if (fanState) {
                 fanState = false;
                 servoMotor.write(0);  // Matikan kipas
                 Serial.println("Kipas mati.");
              }
           }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Tunda 1 detik
     }
  }
  ```

4. **Pelacakan Lokasi Hewan**  
  Sistem menggunakan BLE beacon untuk melacak keberadaan hewan dan memberikan peringatan jika hewan keluar dari radius aman melalui buzzer. 
  ```
  void bleScanTask(void *parameter) {
     while (true) {
        pBLEScan->start(5, false);  // Scan BLE selama 5 detik

        // Memeriksa apakah perangkat target tidak ditemukan dalam 5 menit terakhir
        unsigned long currentTime = millis() / 1000;                              // Mendapatkan waktu saat ini dalam detik
        if ((currentTime - lastSeenTime) > WARNING_INTERVAL || lastRSSI < -80) {  // Jika perangkat tidak ditemukan atau di luar jangkauan (RSSI terlalu lemah)
           Serial.println("WARNING: Pet hasn't been found in the last 5 minutes or out of range!");
           Serial.println("RSSI: " + String(lastRSSI));

           // Menyalakan buzzer dan LED sebagai peringatan
           digitalWrite(LED_BUILTIN, HIGH);
           digitalWrite(buzzerPin, HIGH);
        } else {
           // Mematikan buzzer dan LED jika perangkat ditemukan
           digitalWrite(LED_BUILTIN, LOW);
           digitalWrite(buzzerPin, LOW);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));  // Memberikan jeda 5 detik sebelum scanning berikutnya
     }
  }
  ```

Aplikasi Blynk digunakan sebagai antarmuka utama untuk kontrol dan pemantauan sistem secara real-time, dengan memanfaatkan **virtual pins** untuk komunikasi antara perangkat keras dan aplikasi.

## Test Results and Performance Evaluation

Pengujian sistem dilakukan untuk memastikan semua fitur berjalan sesuai dengan desain. Berikut ini merupakan hasil pengujian untuk setiap fitur utama.

| No | Fitur                       | Kondisi Uji                                              | Status   |
|----|-----------------------------|----------------------------------------------------------|----------|
| 1  | Notifikasi & Pemberian Makan| Notifikasi muncul saat jadwal makan, mekanisme bekerja baik | Berhasil |
| 2  | Pengaturan Pencahayaan      | LED menyala otomatis berdasarkan hasil baca photoresistor atau manual sesuai input pengguna | Berhasil |
| 3  | Pemantauan Suhu Kandang     | Servo motor aktif secara otomatis ketika hasil baca suhu dari DHT11/DHT22 tinggi | Berhasil |
| 4  | Pelacakan Lokasi Hewan      | Buzzer dan LED built-in aktif saat hewan tidak terdeteksi | Berhasil |

**Hasil Uji Sistem di Serial Monitor**

![Initialization Testing](https://github.com/user-attachments/assets/cc9d4dc0-cbb1-434e-909f-ee1b917c4568)

Koneksi ke WiFi dan Blynk berhasil dilakukan. LED berhasil diinisialisasi dan dalam keadaan mati.

![DHT Testing](https://github.com/user-attachments/assets/5d1f852f-0516-4064-a237-27b6c8dd5549)

DHT11 berhasil membaca suhu dan kelembaban di lingkungan sekitar hewan peliharaan.

![LED Testing](https://github.com/user-attachments/assets/7ff1e5ef-9943-4b57-a1e0-e28696d30514)

Photoresistor berhasil membaca intensitas cahaya di lingkungan sekitar hewan peliharaan, mengontrol LED untuk dihidup/matikan, serta mengatur intensitas cahaya LED. 

![Feed Testing 1](https://github.com/user-attachments/assets/91c34281-7e0a-4de2-8354-3c83eff93658)

Pengingat jadwal makan hewan bekerja dengan baik. Pemilik hewan tidak bisa memberi makan hewan di luar jam makan. 

![Feed Testing 2](https://github.com/user-attachments/assets/a8c2cb37-5763-49da-91fe-c628398c44fa)

Pemilik hewan bisa memberi makan hewan ketika sudah memasuki jam makan hewan dan memberi makan hewan maksimal lima kali tekan tombol `Feed`.

![BLE Testing](https://github.com/user-attachments/assets/daea0f8c-c55b-4181-9fe6-c4c20941d4ab)

Lokasi hewan berhasil dilacak dan memberi notifikasi ke pengguna bila hewan berada di luar jangkauan dalam lima menit terakhir. 

**Hasil Uji Sistem di Blynk**

![Blynk Testing](https://github.com/user-attachments/assets/4c890683-e3c2-4287-a2dd-b2e4a4ca4b97)

Tombol `Feed` dan `LED` bekerja dengan baik untuk memberi makan hewan dan mengatur pencahayaan di kandang hewan. Slider intensitas cahaya berhasil mengatur terang cahaya LED ketika dinyalakan. Display value juga dapat menampilkan data suhu dan kelembaban dengan baik. 

## Conclusion and Future Work

**Pet Smart Home** telah berhasil memberikan solusi inovatif untuk memudahkan perawatan hewan peliharaan. Sistem ini mengintegrasikan hardware dan software secara optimal, memungkinkan kontrol jarak jauh, otomatisasi, dan pemantauan real-time melalui aplikasi Blynk.

Meskipun sistem telah diuji dan berjalan dengan baik, beberapa penyempurnaan masih diperlukan, seperti penambahan casing pelindung, optimalisasi BLE scanner, dan pengembangan sistem node dalam memperluas cakupan wilayah BLE scanner. Dengan pengembangan lanjutan, **Pet Smart Home** berpotensi menjadi solusi yang lebih andal untuk pemilik hewan peliharaan di masa depan.
