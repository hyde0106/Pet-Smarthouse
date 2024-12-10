#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define buzzerPin 25

BLEScan *pBLEScan;
const char *targetName = "My_pet";
unsigned long lastSeenTime = 0;
int lastRSSI = 0;

const unsigned long WARNING_INTERVAL = 10; // 5 menit dalam detik

// define class buat callback advertise
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // memeriksa sama dengan targetName
        if (advertisedDevice.getName() == targetName)
        {
            unsigned long currentTime = millis() / 1000;
            lastSeenTime = currentTime;
            lastRSSI = advertisedDevice.getRSSI();

            Serial.print("Found Pet: ");
            Serial.println(advertisedDevice.toString().c_str());
        }
    }
};

// Task buat scanning BLE
void bleScanTask(void *parameter)
{
    while (true)
    {
        pBLEScan->start(5, false); // Scan ble selama 5 detik

        // cek apakah pet tidak ditemukan dalam 5 menit terakhir
        unsigned long currentTime = millis() / 1000; // mendapat waktu sekarang dalam ms
        if ((currentTime - lastSeenTime) > WARNING_INTERVAL || lastRSSI < -80)
        {
            Serial.println("WARNING: Pet hasn't been found in the last 5 minutes or out of range!");
            Serial.println("RSSI: " + String(lastRSSI));    
            // Turn on the buzzer
            digitalWrite(LED_BUILTIN, HIGH);
            digitalWrite(buzzerPin, HIGH);
        }else{
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(buzzerPin, LOW);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup()
{
    Serial.begin(115200);
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);

    pinMode(buzzerPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    // Membuat task scanning BLE
    xTaskCreate(bleScanTask, "BLE Scan Task", 4096, NULL, 1, NULL);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000)); // Idle delay
}
