#include <WiFi.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// DHT configuration
#define DHTPIN 4             // GPIO pin for DHT sensor
#define DHTTYPE DHT11        // Type of DHT sensor (DHT11 or DHT22)
DHT dht(DHTPIN, DHTTYPE);

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2); // Address 0x27, 16x2 LCD

// Wi-Fi credentials
const char* ssid = "Flybox_7728";
const char* password = "G3aYC7dCRG7F"; // No password for this SSID

WiFiServer server(80);

// LED configuration
#define LEDPIN 2 // GPIO pin for LED

// Fan configuration
#define FANPIN 5 // GPIO pin for Fan (connected to a relay)

// Soil moisture sensor configuration
#define SOIL_MOISTURE_PIN 34 // Analog pin for Soil Moisture Sensor

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();

  // Initialize LCD
  lcd.init(); // Initialize LCD
  lcd.backlight(); // Turn on the LCD backlight

  // Initialize LED
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW); // Ensure LED is off initially

  // Initialize Fan
  pinMode(FANPIN, OUTPUT);
  digitalWrite(FANPIN, LOW); // Ensure Fan is off initially

  // Display startup message
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  // Display Wi-Fi connection info on Serial Monitor
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Display Wi-Fi connection info on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  delay(2000); // Show IP for 2 seconds

  lcd.clear();

  // Start server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Read DHT sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read Soil Moisture Sensor data
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  float soilMoisturePercent = map(soilMoistureValue, 0, 4095, 0, 100); // Convert to percentage

  // Display data on LCD
  lcd.clear();
  if (isnan(temperature) || isnan(humidity)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    Serial.println("DHT sensor error");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature, 1); // Display temperature with 1 decimal
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("H: ");
    lcd.print(humidity, 1); // Display humidity with 1 decimal
    lcd.print("% S: ");
    lcd.print(soilMoisturePercent, 1); // Display soil moisture percentage
    lcd.print("%");

    // Log to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" \u00B0C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Control LED and Fan based on temperature
    if (temperature > 20) {
      digitalWrite(LEDPIN, HIGH); // Turn LED on
      digitalWrite(FANPIN, HIGH); // Turn Fan on
    } else {
      digitalWrite(LEDPIN, LOW); // Turn LED off
      digitalWrite(FANPIN, LOW); // Turn Fan off
    }
  }

  // Log soil moisture data to Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");

  // Display IP address on Serial Monitor periodically
  Serial.print("Device IP: ");
  Serial.println(WiFi.localIP());

  // Serve data over web
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    String request = client.readStringUntil('\r');
    client.flush();

    // Generate HTML response
    String html = "<!DOCTYPE html><html>";
    html += "<head><title>DHT Web Server</title>";
    html += "<style>body { font-family: Arial; background-color: #f4f4f4; color: #333; }";
    html += "h1 { color: red; text-align: center; }";
    html += "table { border-collapse: collapse; width: 50%; margin: 20px auto; }";
    html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }";
    html += "th { background-color: #f2f2f2; color: #333; }";
    html += "tr:nth-child(even) { background-color: #f9f9f9; }";
    html += "tr:hover { background-color: #ddd; }";
    html += ".green { color: green; }";
    html += "</style></head>";
    html += "<body><h1>ESP32 DHT Web Server</h1>";
    html += "<table><tr><th>Parameter</th><th>Value</th></tr>";
    if (isnan(temperature) || isnan(humidity)) {
      html += "<tr><td colspan='2'>Error reading sensor data!</td></tr>";
    } else {
      html += "<tr><td>Temperature</td><td class='green'>" + String(temperature, 1) + " \u00B0C</td></tr>";
      html += "<tr><td>Humidity</td><td class='green'>" + String(humidity, 1) + " %</td></tr>";
    }
    html += "<tr><td>Soil Moisture</td><td class='green'>" + String(soilMoisturePercent, 1) + " %</td></tr>";
    html += "</table></body></html>";

    // Send HTTP response
    client.print("HTTP/1.1 200 OK\r\n");
    client.print("Content-Type: text/html\r\n");
    client.print("Connection: close\r\n");
    client.print("\r\n");
    client.print(html);
    client.stop();
    Serial.println("Client disconnected");
  }

  delay(2000); // Add a delay for stability
}



