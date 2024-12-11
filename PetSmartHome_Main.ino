#define BLYNK_TEMPLATE_ID "TMPL6eoylgD75"                    // ID template Blynk
#define BLYNK_TEMPLATE_NAME "PetSmartHome"                   // Nama template Blynk
#define BLYNK_AUTH_TOKEN "MSUyjFk_K4mH34NTpq5t7DwFfOmdbdY7"  // Token autentikasi Blynk

// Library yang digunakan
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>
#include <ESP32Servo.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// Defini pin digital dan virtual
#define PHOTORESISTOR_PIN 32      // Pin untuk sensor photoresistor
#define LED_PIN 13                // Pin untuk LED
#define DHT_PIN 23                // Pin untuk sensor DHT11
#define SERVO_PIN 22              // Pin untuk motor servo
#define FEED_BUTTON_PIN V2        // Virtual pin untuk tombol feed di Blynk
#define FEED_NOTIFICATION_PIN V5  // Virtual pin untuk notifikasi beri makan

// Variabel global untuk perangkat
DHTesp dht;                                  // Objek untuk sensor DHT
Servo servoMotor;                            // Objek untuk servo motor
const char *targetName = "My Pet";           // Nama hewan yang dipantau
unsigned long lastSeenTime = 0;              // Waktu terakhir hewan terlihat
const unsigned long WARNING_INTERVAL = 300;  // Interval peringatan (5 menit)

// Variabel untuk fitur lampu
bool lampStatus = false;  // Status ON/OFF lampu
int brightness = 0;       // Intensitas cahaya (0-5)

// Variabel untuk fitur kipas
bool fanState = false;        // Status ON/OFF kipas
int fanSpeed = 10;            // Kecepatan kipas
int currentAngle = 0;         // Sudut saat ini untuk servo motor
bool rotatingForward = true;  // Arah putaran servo motor

// Variabel untuk fitur makan
int feedButtonPressCount = 0;  // Jumlah tekanan tombol
bool isFeedingTime = false;    // Status waktu makan
bool lastButtonState = false;  // Status tombol sebelumnya

WidgetRTC rtc;  // RTC untuk waktu makan

// Fungsi untuk memperbarui status lampu berdasarkan sensor cahaya
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

// Task untuk membaca data dari sensor DHT11
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

// Task untuk mengontrol putaran kipas
void rotateFan(void *pvParameters) {
  while (true) {
    if (fanState) {  // Jika kipas aktif
      if (rotatingForward) {
        currentAngle += 1;  // Putar maju
        if (currentAngle >= 180) {
          rotatingForward = false;
        }
      } else {
        currentAngle -= 1;  // Putar mundur
        if (currentAngle <= 0) {
          rotatingForward = true;
        }
      }
      servoMotor.write(currentAngle);  // Simpan nilai sudut ke servo motor
    } else {
      servoMotor.write(0);  // Posisi awal jika kipas mati
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Tunda 1 detik
  }
}

// Terima status lampu yang dikirim pengguna melalui Blynk
BLYNK_WRITE(V0) {
  lampStatus = param.asInt();
  updateLampStatus();
}

// Terima intensitas lampu yang dikirim pengguna melalui Blynk
BLYNK_WRITE(V1) {
  brightness = param.asInt();
  updateLampStatus();
}

// Fungsi untuk memproses banyakan tekanan ke tombol makan
BLYNK_WRITE(V2) {
  bool buttonState = param.asInt();  // Membaca status tombol

  if (buttonState == true && lastButtonState == false) {  // Deteksi perubahan status tombol
    if (isFeedingTime) {                                  // Jika sudah waktu makan
      if (feedButtonPressCount < 5) {                     // Cek batas banyaknya tekanan
        feedButtonPressCount++;                           // Tambah jumlah tekanan tombol
        Serial.printf("Tombol Feeder ditekan: %d kali\n", feedButtonPressCount);
        int sisaPencetan = 5 - feedButtonPressCount;
        Blynk.virtualWrite(FEED_NOTIFICATION_PIN, String("Makanan diberikan! Sisa pencetan: ") + sisaPencetan);
      } else {  // Jika mencapai batas pencetan
        Serial.println("Makanan sudah diberikan 5 kali. Tidak dapat memberi makan lagi!");
        Blynk.virtualWrite(FEED_NOTIFICATION_PIN, "Makanan sudah diberikan 5 kali. Tidak dapat memberi makan lagi!");
      }
    } else {  // Jika belum waktu makan
      Serial.println("Tidak dalam waktu makan. Tombol ditekan di luar waktu makan.");
      Blynk.virtualWrite(FEED_NOTIFICATION_PIN, "Tidak dalam waktu makan. Tunggu jadwal makan berikutnya!");
    }
  }
  lastButtonState = buttonState;  // Simpan status tombol untuk iterasi berikutnya
}

// Fungsi untuk mengecek waktu makan
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

// Callback untuk Blynk saat terkoneksi
BLYNK_CONNECTED() {
  Blynk.syncAll();  // Sinkronisasi semua virtual pin, termasuk RTC
}

void setup() {
  Serial.begin(115200);  // Mulai serial monitor

  // Mulai koneksi dengan WiFi
  Serial.print("\nConnecting to WiFi");
  WiFi.begin("Ini WiFi", "IniPassword", 6);
  while (WiFi.status() != WL_CONNECTED) {  // Cek status koneksi WiFi
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Mulai koneksi dengan Blynk
  Serial.print("\nConnecting to Blynk");
  Blynk.begin(BLYNK_AUTH_TOKEN, "Ini WiFi", "IniPassword");
  while (!Blynk.connected()) {  // Cek status koneksi Blynk
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected to Blynk!");

  dht.setup(DHT_PIN, DHTesp::DHT11);  // Inisialisasi sensor DHT
  servoMotor.attach(SERVO_PIN);       // Inisialisasi servo motor
  pinMode(LED_PIN, OUTPUT);           // Atur pin LED sebagai output
  analogWrite(LED_PIN, 0);            // Mulai LED dalam keadaan mati
  Serial.println("LED initialized!");

  updateLampStatus();

  rtc.begin();  // Inisialisasi RTC

  xTaskCreate(readDHT, "ReadDHT", 4096, NULL, 1, NULL);      // Task untuk membaca DHT
  xTaskCreate(rotateFan, "RotateFan", 2048, NULL, 1, NULL);  // Task untuk mengatur kipas

  Serial.println("\n----------------");
  Serial.println("|Pet Smart Home|");
  Serial.println("----------------");
}

void loop() {
  Blynk.run();         // Jalankan Blynk
  checkFeedingTime();  // Cek waktu makan
  delay(500);          // Tunda untuk mengurangi beban loop
}
