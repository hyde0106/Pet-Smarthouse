#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// UUID untuk BLE service dan characteristic
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

String receivedValue;  // Variabel untuk menyimpan nilai yang diterima melalui BLE

/**
 * @brief Callback class for handling characteristic write events.
 *
 * This class inherits from BLECharacteristicCallbacks and overrides the onWrite method
 * to handle events when a new value is written to the characteristic.
 */
class MyCallbacks : public BLECharacteristicCallbacks {
  /**
   * @brief Called when a client writes a new value to the characteristic.
   *
   * @param pCharacteristic Pointer to the characteristic being written to.
   */
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();  // Membaca nilai dari karakteristik

    if (value.length() > 0) {  // Pastikan nilai tidak kosong
      receivedValue = "";      // Reset nilai yang diterima
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);     // Cetak karakter satu per satu
        receivedValue += value[i];  // Simpan nilai dalam variabel global
      }

      Serial.println();
      Serial.println("*********");
    }
  }
};

/**
 * @brief Callback class for handling server connection events.
 * 
 * This class inherits from BLEServerCallbacks and overrides the onConnect method
 * to handle events when a client connects to the server.
 */
class MyServerCallbacks : public BLEServerCallbacks {
  /**
   * @brief Called when a client connects to the server.
   * 
   * @param pServer Pointer to the BLE server.
  */
  void onConnect(BLEServer *pServer) {
    BLEDevice::startAdvertising();  // Mulai iklan BLE ulang saat client terhubung
    Serial.println("Connected to BLE client");
  };
};

/**
 * @brief Setup function for initializing BLE server and services.
 * 
 * This function initializes the BLE device, creates a server, a service,
 * and a characteristic, and starts advertising the BLE device.
 */
void setup() {
  Serial.begin(115200);

  BLEDevice::init("My_pet");                       // Inisialisasi perangkat BLE dengan nama "My_pet"
  BLEServer *pServer = BLEDevice::createServer();  // Buat server BLE
  pServer->setCallbacks(new MyServerCallbacks());  // Tambahkan callback untuk event server

  BLEService *pService = pServer->createService(SERVICE_UUID);                                                                                                     // Membuat service dengan UUID yang telah didefinisikan
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);  // Membuat characteristic dengan properti READ dan WRITE
  pCharacteristic->setCallbacks(new MyCallbacks());                                                                                                                // Menambahkan callback untuk event write

  pService->start();  // Memulai service

  // Memulai advertising BLE
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

/**
 * @brief Loop function for handling dynamic device name changes.
 * 
 * This function checks if a new value has been written to the characteristic.
 * If a new value is received, it stops advertising, reinitializes the BLE device
 * with the new name, and starts advertising again.
 */
void loop() {
  if (receivedValue != "") {       // Jika ada nilai baru yang diterima
    BLEDevice::stopAdvertising();  // Hentikan advertising BLE
    BLEDevice::deinit();           // Deinisialisasi perangkat BLE

    BLEDevice::init(receivedValue);                  // Gunakan tipe String langsung
    BLEServer *pServer = BLEDevice::createServer();  // Buat ulang server BLE
    pServer->setCallbacks(new MyServerCallbacks());

    // Membuat ulang service dan characteristic
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pCharacteristic->setCallbacks(new MyCallbacks());

    pService->start();  // Mulai service ulang

    // Mulai kembali advertising
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();

    receivedValue = "";  // Reset nilai yang diterima

    delay(1000);  // Jeda 1 detik sebelum iterasi berikutnya
  }
}
