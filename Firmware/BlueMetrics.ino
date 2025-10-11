#define BLYNK_TEMPLATE_ID "TMPL5lxVUTLtd"
#define BLYNK_TEMPLATE_NAME "Blue Metrics"
#define BLYNK_AUTH_TOKEN "zI-f-w4M7m3FCquwKjqMpgJMVIhvkJ56"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- WiFi credentials ---
char ssid[] = "YOUR_WIFI_SSID";         // <-- Change this
char pass[] = "YOUR_WIFI_PASSWORD";     // <-- Change this

// --- Sensor pin configuration ---
#define PH_PIN 34          // Analog input for pH sensor
#define TURBIDITY_PIN 35   // Analog input for turbidity sensor
#define ONE_WIRE_BUS 4     // Digital pin for DS18B20 temperature sensor

// --- Initialize OneWire and DS18B20 ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- Variables for sensor readings ---
float phValue = 0.0;
float turbidityValue = 0.0;
float temperatureC = 0.0;

// --- Virtual Pin Assignments (Blynk Dashboard) ---
// V0 = pH Gauge + Chart
// V1 = Temperature Gauge + Chart
// V2 = Turbidity Gauge + Chart
// V3 = pH Status Label
// V4 = Temperature Status Label
// V5 = Turbidity Status Label

BlynkTimer timer;

// --- Function to read pH sensor ---
float readPH() {
  int analogValue = analogRead(PH_PIN);
  float voltage = analogValue * (3.3 / 4095.0);  // Convert to voltage (ESP32: 12-bit ADC)
  float ph = 7 + ((2.5 - voltage) / 0.18);       // Calibration formula (approx.)
  return ph;
}

// --- Function to read turbidity sensor ---
float readTurbidity() {
  int sensorValue = analogRead(TURBIDITY_PIN);
  float voltage = sensorValue * (3.3 / 4095.0);
  float turbidity = 100 - (voltage / 3.3) * 100;  // Inverse relation to clarity
  return turbidity;
}

// --- Function to read temperature ---
float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

// --- Function to send sensor data to Blynk ---
void sendSensorData() {
  phValue = readPH();
  turbidityValue = readTurbidity();
  temperatureC = readTemperature();

  // Send to Blynk gauges & charts
  Blynk.virtualWrite(V0, phValue);
  Blynk.virtualWrite(V1, temperatureC);
  Blynk.virtualWrite(V2, turbidityValue);

  // --- pH status ---
  String phStatus;
  if (phValue >= 6.5 && phValue <= 8.5) phStatus = "Safe";
  else phStatus = "Unsafe";
  Blynk.virtualWrite(V3, "pH Status: " + phStatus);

  // --- Temperature status ---
  String tempStatus;
  if (temperatureC >= 20 && temperatureC <= 30) tempStatus = "Safe";
  else tempStatus = "Unsafe";
  Blynk.virtualWrite(V4, "Temp Status: " + tempStatus);

  // --- Turbidity status ---
  String turbStatus;
  if (turbidityValue >= 0 && turbidityValue <= 50) turbStatus = "Clear";
  else turbStatus = "Murky";
  Blynk.virtualWrite(V5, "Turbidity: " + turbStatus);

  // Debug output
  Serial.print("pH: "); Serial.print(phValue);
  Serial.print(" | Temp: "); Serial.print(temperatureC);
  Serial.print("°C | Turbidity: "); Serial.println(turbidityValue);
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(5000L, sendSensorData); // Send data every 5 seconds
}

void loop() {
  Blynk.run();
  timer.run();
}
