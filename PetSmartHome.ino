#define BLYNK_TEMPLATE_ID "TMPL6eoylgD75"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_AUTH_TOKEN "MSUyjFk_K4mH34NTpq5t7DwFfOmdbdY7"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>
#include <ESP32Servo.h>
//#include <BLEDevice.h>
//#include <BLEScan.h>
//#include <BLEAdvertisedDevice.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#define PHOTORESISTOR_PIN 32
#define LED_PIN 15
#define DHT_PIN 35
#define SERVO_PIN 22
#define FEED_BUTTON_PIN V2      // Tombol di Blynk
#define FEED_NOTIFICATION_PIN V5 // Virtual pin untuk notifikasi
#define RINGBUF_TYPE_NOSPLIT 0 // Gantikan enum dengan nilai default


DHTesp dht;
Servo servoMotor;
//BLEScan *pBLEScan;
const char *targetName = "My Pet";
unsigned long lastSeenTime = 0;
const unsigned long WARNING_INTERVAL = 300; // 5 menit dalam detik
typedef int ringbuf_type_t; // Tambahkan ini jika tidak ditemukan definisi.

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

bool lampStatus = false;         // Status ON/OFF lampu
int brightness = 0;              // Intensitas cahaya (0-5)

bool fanState = false;
int fanSpeed = 10;
int currentAngle = 0;
bool rotatingForward = true;

// Variabel untuk fitur makan
int feedButtonPressCount = 0; // Jumlah tekanan tombol
bool isFeedingTime = false;   // Status waktu makan

// RTC untuk waktu makan
WidgetRTC rtc;

void updateLampStatus() {
  int ldrValue = analogRead(PHOTORESISTOR_PIN); // Baca nilai dari photoresistor
  int lightValue = map(ldrValue, 0, 4095, 255, 0); // Konversi ke rentang 0-255

  // Jika lampStatus ON, gunakan nilai dari photoresistor
  if (lampStatus) {
    analogWrite(LED_PIN, lightValue); // Atur intensitas LED sesuai cahaya ruangan
    Serial.printf("Lamp ON - LDR Value: %d, LED Intensity: %d\n", ldrValue, lightValue);
  } else {
    analogWrite(LED_PIN, 0); // Matikan LED jika lampStatus OFF
    Serial.println("Lamp OFF");
  }

  // Perbarui status di Blynk
  Blynk.virtualWrite(V0, lampStatus ? "ON" : "OFF");
  Blynk.virtualWrite(V1, brightness);
}

// Task untuk membaca data dari DHT22
void readDHT(void *pvParameters) {
  while (true) {
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Gagal membaca data dari DHT22!");
    } else {
      Serial.printf("Suhu: %.1f°C, Kelembapan: %.1f%%\n", temperature, humidity);
      Blynk.virtualWrite(V3, temperature);
      Blynk.virtualWrite(V4, humidity);

      if (temperature >= 30.0) {
        fanState = true;
      } else {
        fanState = false;
      }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Task untuk memutar kipas
void rotateFan(void *pvParameters) {
  while (true) {
    if (fanState) {
      if (rotatingForward) {
        currentAngle += 1;
        if (currentAngle >= 180) {
          rotatingForward = false;
        }
      } else {
        currentAngle -= 1;
        if (currentAngle <= 0) {
          rotatingForward = true;
        }
      }
      servoMotor.write(currentAngle);
    } else {
      servoMotor.write(0);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

BLYNK_WRITE(V0) {
  lampStatus = param.asInt();
  updateLampStatus();
}

BLYNK_WRITE(V1) {
  brightness = param.asInt();
  updateLampStatus();
}

// Fungsi untuk memproses tekanan tombol
BLYNK_WRITE(V2) {
  int buttonState = param.asInt(); // Baca status tombol dari Blynk
  Serial.printf("Tombol Feeder ditekan: %d\n", buttonState); // Debugging

  if (buttonState == 1) { // Jika tombol ditekan
    if (isFeedingTime) { // Periksa apakah sedang waktu makan
      if (feedButtonPressCount < 5) {
        feedButtonPressCount++;
        Serial.printf("Tombol ditekan %d kali\n", feedButtonPressCount);

        // Gerakkan servo untuk memberi makan
        servoMotor.write(90);
        delay(2000);
        servoMotor.write(0); // Kembalikan posisi servo

        Blynk.virtualWrite(FEED_NOTIFICATION_PIN, String("Makanan diberikan! Sisa pencetan: ") + (5 - feedButtonPressCount));
      } else {
        Blynk.virtualWrite(FEED_NOTIFICATION_PIN, "Makanan sudah diberikan 5 kali. Tidak dapat memberi makan lagi!");
        Serial.println("Notifikasi: Sudah memenuhi batas pemberian makan.");
      }
    } else {
      Blynk.virtualWrite(FEED_NOTIFICATION_PIN, "Tidak dalam waktu makan. Tunggu jadwal makan berikutnya!");
      Serial.println("Notifikasi: Tombol ditekan di luar waktu makan.");
    }
  }
}


// Fungsi untuk mengatur waktu makan
void checkFeedingTime() {
  int currentHour = hour(); // Mengambil waktu jam dari RTC Blynk

  if (currentHour == 8 || currentHour == 18) { // Contoh jadwal makan
    if (!isFeedingTime) {
      isFeedingTime = true;
      feedButtonPressCount = 0;
      Serial.println("Notifikasi: Waktunya memberi makan hewan!");
    }
  } else {
    if (isFeedingTime){
    isFeedingTime = false;
    feedButtonPressCount = 0; // Reset hitungan tombol di luar waktu makan
    Serial.println("Notifikasi: Di luar waktu makan, hitungan tombol direset.");
    }
  }
}

BLYNK_CONNECTED() {
  Blynk.syncAll(); // Sinkronisasi semua virtual pin, termasuk RTC
}

void setup() {
  Serial.begin(115200);

  Serial.print("\nConnecting to WiFi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  Serial.print("\nConnecting to Blynk");
  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");
  while (!Blynk.connected()) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected to Blynk!");

  dht.setup(DHT_PIN, DHTesp::DHT22);
  servoMotor.attach(SERVO_PIN);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);
  Serial.println("LED initialized!");

  //timer.setInterval(1000L, checkFeedingTime); //cek waktu makan setiap detik

  updateLampStatus();

  rtc.begin(); // Inisialisasi RTC

  xTaskCreate(readDHT, "ReadDHT", 4096, NULL, 1, NULL);
  xTaskCreate(rotateFan, "RotateFan", 2048, NULL, 1, NULL);
  //xTaskCreate(bleScanTask, "BLE Scan Task", 4096, NULL, 1, NULL);

  Serial.println("\n----------------");
  Serial.println("|Pet Smart Home|");
  Serial.println("----------------");
}

void loop() {
  Blynk.run();
  checkFeedingTime();
  delay(500);
}
