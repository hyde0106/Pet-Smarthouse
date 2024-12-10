# Pet Smart Home
## Introduction to the Problem and the Solution

Dalam era modern yang serba cepat, banyak pemilik hewan peliharaan menghadapi tantangan dalam merawat hewan kesayangan mereka. Kesibukan sehari-hari membuat sulit untuk menjaga jadwal makan, memantau kondisi lingkungan, serta memastikan keamanan hewan peliharaan. Masalah ini dapat berdampak pada kesehatan dan kesejahteraan hewan.

**Pet Smart Home** hadir sebagai solusi berbasis IoT untuk mempermudah perawatan hewan peliharaan. Sistem ini mengintegrasikan berbagai teknologi pintar untuk otomatisasi jadwal makan, pengontrolan pencahayaan, pemantauan suhu kandang, dan pelacakan lokasi hewan. Sistem ini dirancang agar dapat dioperasikan secara mudah melalui aplikasi **Blynk**, memungkinkan kontrol jarak jauh, pemberitahuan real-time, serta pemantauan kondisi lingkungan hewan.

## Hardware Design and Implementation Details

Desain hardware Pet Smart Home bertujuan untuk mendukung semua fitur utama dengan komponen elektronik berikut:

1. **ESP32**  
   Mikrokontroler utama dengan konektivitas Wi-Fi dan BLE untuk mengintegrasikan fitur perangkat keras dengan aplikasi Blynk.

2. **Sensor DHT11**  
   Digunakan untuk memantau suhu dan kelembapan kandang hewan. Data dari sensor ini mengontrol kipas secara otomatis untuk menjaga kenyamanan suhu kandang.

3. **Servo Motor**  
   Mengoperasikan mekanisme pemberian makan otomatis dengan menggerakkan tempat makan saat jadwal makan tiba.

4. **Photoresistor**  
   Mendeteksi tingkat pencahayaan di kandang dan mengontrol LED untuk pencahayaan otomatis atau manual melalui aplikasi.

5. **LED**  
   Memberikan pencahayaan di kandang yang dapat diatur intensitasnya secara otomatis atau manual menggunakan slider pada aplikasi.

6. **BLE Beacon**  
   Dipasang pada tubuh hewan (misalnya, di kalung) untuk memantau posisi hewan. Jika hewan keluar dari radius tertentu, sistem akan memberikan peringatan.

7. **Buzzer**  
   Digunakan sebagai peringatan tambahan saat hewan tidak terdeteksi dalam radius tertentu selama periode waktu tertentu.

## Software Implementation Details

Pengembangan perangkat lunak Pet Smart Home bertujuan untuk mengintegrasikan fungsi hardware dengan sistem IoT berbasis aplikasi **Blynk**. Proses utama meliputi:

- **Notifikasi dan Kontrol Jadwal Makan**  
  Sistem memantau jadwal makan secara real-time dan mengirimkan notifikasi ke aplikasi. Pengguna dapat memberi makan hewan dengan mengaktifkan mekanisme servo motor.

- **Pengaturan Pencahayaan**  
  Menggunakan photoresistor untuk mendeteksi intensitas cahaya. Pengguna dapat mengontrol pencahayaan melalui aplikasi atau mengandalkan sistem otomatis.

- **Pemantauan Suhu Kandang**  
  Data dari sensor DHT11 digunakan untuk mengaktifkan kipas secara otomatis saat suhu melebihi ambang batas tertentu.

- **Pelacakan Lokasi Hewan**  
  Menggunakan BLE beacon untuk melacak keberadaan hewan dan memberikan peringatan jika hewan keluar dari radius aman.

Aplikasi Blynk digunakan sebagai antarmuka utama untuk kontrol dan pemantauan real-time, memanfaatkan **virtual pins** untuk komunikasi antara perangkat keras dan aplikasi.

## Test Results and Performance Evaluation

Pengujian sistem dilakukan untuk memastikan semua fitur berjalan sesuai dengan desain. Berikut hasil pengujian untuk setiap fitur utama:

| No | Fitur                       | Kondisi Uji                                              | Status   |
|----|-----------------------------|----------------------------------------------------------|----------|
| 1  | Notifikasi & Pemberian Makan| Indikasi muncul saat jadwal makan, mekanisme bekerja baik| Berhasil |
| 2  | Pengaturan Pencahayaan      | LED menyala otomatis/manual sesuai input pengguna        | Berhasil |
| 3  | Pemantauan Suhu             | Kipas aktif saat suhu panas                              | Berhasil |
| 4  | Pelacakan Posisi Hewan      | Buzzer aktif saat hewan tidak terdeteksi                | Berhasil |

Evaluasi tambahan:
- Membuat casing untuk melindungi komponen elektronik.
- Integrasi BLE scanner dengan kontroler utama untuk deteksi lebih andal.
- Implementasi sistem node untuk memperluas cakupan BLE scanner.

## Conclusion and Future Work

**Pet Smart Home** telah berhasil memberikan solusi inovatif untuk memudahkan perawatan hewan peliharaan. Sistem ini mengintegrasikan hardware dan software secara optimal, memungkinkan kontrol jarak jauh, otomatisasi, dan pemantauan real-time melalui aplikasi Blynk.

Meski sistem telah diuji dan berjalan dengan baik, beberapa penyempurnaan masih diperlukan, seperti penambahan casing pelindung, optimalisasi BLE scanner, dan pengembangan sistem node untuk meningkatkan performa. Dengan pengembangan lanjutan, Pet Smart Home berpotensi menjadi solusi yang lebih andal untuk pemilik hewan peliharaan di masa depan.
