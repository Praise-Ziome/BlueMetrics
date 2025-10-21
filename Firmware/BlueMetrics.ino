/**************************************************************
 * Smart AquaSense System - Complete Code
 * ESP32 + Blynk + pH, Temperature (DS18B20), Turbidity Sensors
 **************************************************************/

#define BLYNK_TEMPLATE_ID "TMPL5lxVUTLtd"
#define BLYNK_TEMPLATE_NAME "Blue Metrics"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ----------- Replace with your Blynk credentials -----------
char auth[] = "zI-f-w4M7m3FCquwKjqMpgJMVIhvkJ56";
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ----------- Sensor Pin Definitions -----------
#define PH_PIN 34
#define TEMP_PIN 4
#define TURBIDITY_PIN 35
#define LED_PIN 2

// ----------- DS18B20 Setup -----------
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

// ----------- Thresholds -----------
float safePHMin = 6.5;
float safePHMax = 8.5;
float safeTempMin = 20.0;
float safeTempMax = 30.0;
float safeTurbidityMax = 1000; // Adjust depending on sensor calibration

// ----------- Counters for stability -----------
int unsafeCount = 0;
const int UNSAFE_LIMIT = 5;  // LED only lights up after 5 consecutive unsafe readings

// ----------- Setup -----------
void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Blynk.begin(auth, ssid, pass);
  sensors.begin();
  Serial.println("Smart AquaSense System Initializing...");
}

// ----------- Main Loop -----------
void loop()
{
  Blynk.run();
  sensors.requestTemperatures();

  // ---------- Read Sensor Values ----------
  int phValue = analogRead(PH_PIN);
  int turbidityValue = analogRead(TURBIDITY_PIN);
  float temperature = sensors.getTempCByIndex(0);

  // ---------- Convert Raw Readings ----------
  float voltagePH = phValue * (3.3 / 4095.0);
  float pH = 7 + ((2.5 - voltagePH) / 0.18); // Approximate calibration

  float turbidity = map(turbidityValue, 0, 4095, 0, 3000); // Simulated NTU range

  // ---------- Display Raw Readings ----------
  Serial.print("pH: "); Serial.print(pH);
  Serial.print(" | Temp: "); Serial.print(temperature);
  Serial.print("°C | Turbidity: "); Serial.println(turbidity);

  // ---------- Send Readings to Blynk Gauges ----------
  Blynk.virtualWrite(V0, pH);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, turbidity);

  // ---------- Update Safe/Unsafe Labels ----------
  String phStatus = (pH >= safePHMin && pH <= safePHMax) ? "Safe" : "Unsafe";
  String tempStatus = (temperature >= safeTempMin && temperature <= safeTempMax) ? "Safe" : "Unsafe";
  String turbStatus = (turbidity <= safeTurbidityMax) ? "Safe" : "Unsafe";

  Blynk.virtualWrite(V3, phStatus);
  Blynk.virtualWrite(V4, tempStatus);
  Blynk.virtualWrite(V5, turbStatus);

  // ---------- LED Alert Logic ----------
  if (phStatus == "Unsafe" || tempStatus == "Unsafe" || turbStatus == "Unsafe")
  {
    unsafeCount++;
    if (unsafeCount >= UNSAFE_LIMIT)
    {
      digitalWrite(LED_PIN, HIGH);
    }
  }
  else
  {
    unsafeCount = 0;
    digitalWrite(LED_PIN, LOW);
  }

  delay(5000); // 5-second interval between readings
}
