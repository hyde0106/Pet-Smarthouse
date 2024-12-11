#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define buzzerPin 25  // Pin untuk buzzer

BLEScan *pBLEScan;  // Objek BLE Scanner

const char *targetName = "My_pet";          // Nama target perangkat BLE yang dicari
unsigned long lastSeenTime = 0;             // Variabel untuk melacak waktu terakhir perangkat ditemukan
int lastRSSI = 0;                           // Variabel untuk menyimpan nilai RSSI terakhir
const unsigned long WARNING_INTERVAL = 10;  // Interval peringatan (5 menit)

// Class untuk menangani callback dari perangkat BLE yang terdeteksi
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == targetName) {  // Memeriksa apakah hasil advertised sama dengan targetName
      unsigned long currentTime = millis() / 1000;   // Mendapatkan waktu saat ini dalam detik
      lastSeenTime = currentTime;                    // Memperbarui waktu terakhir perangkat ditemukan
      lastRSSI = advertisedDevice.getRSSI();         // Memperbarui nilai RSSI

      Serial.print("Found Pet: ");
      Serial.println(advertisedDevice.toString().c_str());
    }
  }
};

// Task untuk melakukan scanning BLE secara periodik
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

void setup() {
  Serial.begin(115200);  // Inisialisasi komunikasi serial untuk debugging

  BLEDevice::init("");                                                        // Inisialisasi perangkat BLE
  pBLEScan = BLEDevice::getScan();                                            // Mendapatkan objek scanner BLE
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());  // Mengatur callback untuk perangkat yang ditemukan
  pBLEScan->setActiveScan(true);                                              // Mengaktifkan mode aktif untuk scanning

  // Mengatur pin untuk buzzer dan LED
  pinMode(buzzerPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Membuat task scanning BLE
  xTaskCreate(bleScanTask, "BLE Scan Task", 4096, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));  // Memberikan jeda 1 detik
}
