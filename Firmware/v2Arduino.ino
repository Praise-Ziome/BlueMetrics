/***************************************************
 *  BlueMetrics: IoT Water Quality Monitoring System
 *  Sends pH, Temperature, and Turbidity data to MySQL
 *  via PHP API running on localhost (XAMPP)
 ***************************************************/

#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---- WiFi Credentials ----
const char* ssid = "Admin";            // your WiFi name
const char* password = "LHATil@pi@+2020";  // your WiFi password

// ---- Local Server URL ----
// Use your PC's local IP (XAMPP machine IP). Find it using `ipconfig`
// Example: 192.168.0.105 — replace with yours
String serverName = "http://192.168.1.182/bluemetrics/insert_data.php";

// ---- Sensor Pins ----
#define PH_PIN 34
#define TURB_PIN 35
#define TEMP_PIN 32

// ---- OneWire Setup for DS18B20 ----
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  sensors.requestTemperatures();
  
  float phValue = analogRead(PH_PIN) * (14.0 / 4095.0);
  float turbValue = analogRead(TURB_PIN) / 100.0; // normalize
  float tempC = sensors.getTempCByIndex(0);

  Serial.print("📊 pH: "); Serial.print(phValue);
  Serial.print(" | 🌡 Temp: "); Serial.print(tempC);
  Serial.print(" °C | 🌫 Turbidity: "); Serial.println(turbValue);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = serverName + "?ph=" + String(phValue, 2) + 
                 "&temperature=" + String(tempC, 2) + 
                 "&turbidity=" + String(turbValue, 2);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("✅ Server response: " + payload);
    } else {
      Serial.println("❌ Error sending data.");
    }

    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected!");
  }

  delay(5000); // send every 5 seconds
}
