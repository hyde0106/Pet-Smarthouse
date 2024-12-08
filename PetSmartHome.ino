#define BLYNK_TEMPLATE_ID "TMPL6jOIj6_Iz"
#define BLYNK_TEMPLATE_NAME "Pet Smart Home"
#define BLYNK_AUTH_TOKEN "7EnhwJjWybAldOHv3ylDhFw6n9GJq3hZ"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>
#include <ESP32Servo.h>

#define PHOTORESISTOR_PIN 32
#define LED_PIN 12
#define DHT_PIN 33
#define SERVO_PIN 13

DHTesp dht;
Servo servoMotor;
BlynkTimer timer;

bool fanState = false;
int fanSpeed = 10; 
int currentAngle = 0; 
bool rotatingForward = true; 

void readDHT() {
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Gagal membaca data dari DHT22!");
    return;
  }

  Serial.printf("Suhu: %.1f°C, Kelembapan: %.1f%%\n", temperature, humidity);
  Blynk.virtualWrite(V3, temperature);
  Blynk.virtualWrite(V4, humidity);

  if (temperature >= 30.0) {
    fanState = true;
  } else {
    fanState = false;
  }
}

void rotateFan() {
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
  while (!Blynk.connected()){
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected to Blynk!");

  dht.setup(DHT_PIN, DHTesp::DHT22);
  servoMotor.attach(SERVO_PIN);

  timer.setInterval(1000L, readDHT);
  timer.setInterval(10L, rotateFan);

  Serial.println("\n----------------");
  Serial.println("|Pet Smart Home|");
  Serial.println("----------------");
}

void loop() {
  Blynk.run();
  timer.run();
}