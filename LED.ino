#define BLYNK_TEMPLATE_ID "TMPL6eoylgD75"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_AUTH_TOKEN "MSUyjFk_K4mH34NTpq5t7DwFfOmdbdY7"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define LED_PIN 15

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

bool lampStatus = false;         // Status ON/OFF lampu
int brightness = 0;              // Intensitas cahaya (0-5)


void updateLampStatus() {
  if (lampStatus) {
    int pwmValue = map(brightness, 0, 5, 0, 255);
    analogWrite(LED_PIN, pwmValue);
  } else {
    analogWrite(LED_PIN, 0);
  }

  Blynk.virtualWrite(V0, lampStatus ? "ON" : "OFF");
  Blynk.virtualWrite(V1, brightness);

  Serial.printf("Lamp Status: %s, Brightness: %d\n", lampStatus ? "ON" : "OFF", brightness);
}


BLYNK_WRITE(V0) {
  lampStatus = param.asInt();   
  updateLampStatus();
}

BLYNK_WRITE(V1) {
  brightness = param.asInt();
  updateLampStatus();
}

void setup() {
  Serial.begin(115200);

  Serial.println("Connecting to WiFi...");
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  Serial.println("Connected to WiFi and Blynk!");

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);
  Serial.println("LED initialized!");

  updateLampStatus();
}

void loop() {
  Blynk.run();
}
