#define BLYNK_TEMPLATE_ID "TMPL6jOIj6_Iz"
#define BLYNK_TEMPLATE_NAME "Pet Smart Home"
#define BLYNK_AUTH_TOKEN "7EnhwJjWybAldOHv3ylDhFw6n9GJq3hZ"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>
#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


#define PHOTORESISTOR_PIN 32
#define LED_PIN 12
#define DHT_PIN 33
#define SERVO_PIN 13

DHTesp dht;
Servo servoMotor;
BLEScan* pBLEScan;
const char* targetName = "My Pet"; 
unsigned long lastSeenTime = 0; 
const unsigned long WARNING_INTERVAL = 300; // 5 menit dalam detik

bool fanState = false;
int fanSpeed = 10; 
int currentAngle = 0; 
bool rotatingForward = true;

//define class buat callback advertise
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // memeriksa sama dengan targetName
    if (advertisedDevice.getName() == targetName) {
      unsigned long currentTime = millis() / 1000; //mendapat waktu sekarang dalam ms
      lastSeenTime = currentTime; // update lastseen

      Serial.print("Found Pet: ");
      Serial.println(advertisedDevice.toString().c_str());
    }
  }
};

//Task buat scanning BLE
void bleScanTask(void* parameter) {
  while (true) {
    pBLEScan->start(5, false); // Scan ble selama 5 detik

    // cek apakah pet tidak ditemukan dalam 5 menit terakhir
    unsigned long currentTime = millis() / 1000; // mendapat waktu sekarang dalam ms
    if ((currentTime - lastSeenTime) > WARNING_INTERVAL) {
      Serial.println("WARNING: Pet hanst been found in the last 5 minutes!");
    }

    vTaskDelay(pdMS_TO_TICKS(5000)); 
  }
}

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

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);

  xTaskCreate(readDHT, "ReadDHT", 4096, NULL, 1, NULL);
  xTaskCreate(rotateFan, "RotateFan", 2048, NULL, 1, NULL);
  xTaskCreate(bleScanTask, "BLE Scan Task", 4096, NULL, 1, NULL);

  Serial.println("\n----------------");
  Serial.println("|Pet Smart Home|");
  Serial.println("----------------");
}

void loop() {
  Blynk.run();
}
