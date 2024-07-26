#include <DHT.h>
#include <SoftwareSerial.h>
#include <ThingSpeak.h>
#define DHTPIN 2
#define DHTTYPE DHT11

#define MOISTURE_SENSOR A0
#define LDR_SENSOR A1
#define FAN_PIN 5
#define PUMP_PIN 6

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial espSerial(10, 11); // RX, TX for ESP01

// WiFi settings
const char *ssid = "saro";
const char *password = "12345678";

// ThingSpeak settings
const char *thingSpeakApiKey = "2X3IO4LI23TFX6CS";
const unsigned long updateInterval = 5000; // Update every 5 seconds (adjust as needed)
unsigned long lastConnectionTime = 0;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);
  pinMode(MOISTURE_SENSOR, INPUT);
  pinMode(LDR_SENSOR, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  dht.begin();

  // Connect to WiFi
  if (!connectToWiFi()) {
    Serial.println("Failed to connect to WiFi!");
  }
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int moistureValue = analogRead(MOISTURE_SENSOR);
  int ldrValue = analogRead(LDR_SENSOR);

  // Display sensor values on Serial Monitor
  Serial.println("Sensor Readings:");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C   Humidity: ");
  Serial.print(humidity);
  Serial.print(" %   Moisture: ");
  Serial.print(moistureValue);
  Serial.print("   LDR: ");
  Serial.println(ldrValue);

  // Control DC fan based on temperature
  if (temperature > 33) {
    digitalWrite(FAN_PIN, HIGH); // Turn on fan
    Serial.println("Fan is ON (Temperature threshold exceeded)");
  } else {
    digitalWrite(FAN_PIN, LOW); // Turn off fan
    Serial.println("Fan is OFF");
  }

  // Control DC water pump based on soil moisture
  if (moistureValue > 800) {
    digitalWrite(PUMP_PIN, HIGH); // Turn on pump
    Serial.println("Water Pump is ON (Moisture threshold exceeded)");
  } else {
    digitalWrite(PUMP_PIN, LOW); // Turn off pump
    Serial.println("Water Pump is OFF");
  }

  // Send data to ThingSpeak
  if (millis() - lastConnectionTime > updateInterval) {
    updateThingSpeak(temperature, humidity, moistureValue, ldrValue);
  }

  Serial.println(); // Separate each iteration in the Serial Monitor

  delay(5000); // Delay for 5 seconds (adjust as needed)
}

bool connectToWiFi() {
  Serial.println("Connecting to WiFi");

  espSerial.println("AT+RST"); // Reset ESP module
  delay(2000);

  espSerial.println("AT+CWMODE=1"); // Set ESP mode to Station mode
  delay(1000);

  espSerial.print("AT+CWJAP=\"");
  espSerial.print(ssid);
  espSerial.print("\",\"");
  espSerial.print(password);
  espSerial.println("\""); // Connect to WiFi
  delay(5000);

  if (espSerial.find("OK")) {
    Serial.println("Connected to WiFi");
    return true;
  } else {
    Serial.println("Failed to connect to WiFi!");
    return false;
  }
}

void updateThingSpeak(float temperature, float humidity, int moistureValue, int ldrValue) {
  Serial.println("Updating ThingSpeak");

  String data = "GET /update?api_key=" + String(thingSpeakApiKey) +
                "&field1=" + String(temperature) +
                "&field2=" + String(humidity) +
                "&field3=" + String(moistureValue) +
                "&field4=" + String(ldrValue);

  espSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80");
  delay(2000);
  espSerial.println("AT+CIPSEND=" + String(data.length() + 4));
  delay(2000);
  espSerial.println(data);
  delay(5000);

  lastConnectionTime = millis();
}
