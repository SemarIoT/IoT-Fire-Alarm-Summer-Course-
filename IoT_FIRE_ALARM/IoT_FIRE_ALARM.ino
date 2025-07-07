/*
==========================================
          SUMMER COURSE PROGRAM
IOT INTEGRATED FIRE ALARM DETECTION SYSTEM
          WITH THINGSBOARD INTEGRATION
==========================================

WHAT THIS PROGRAM DOES:
- Reads temperature from DHT11 sensor
- If temperature > 30°C → FIRE ALARM ON
- If temperature ≤ 30°C → NORMAL MODE
- Sends data to ThingsBoard Cloud every loop
*/

// ===== STEP 1: INCLUDE LIBRARIES =====
#include <DHT.h>        // Library for DHT11 sensor
#include <ESP8266WiFi.h>    // WiFi library
#include <PubSubClient.h>   // MQTT library for ThingsBoard

// ===== STEP 2: WIFI & THINGSBOARD CONFIG =====
const char* WIFI_SSID = "YOUR_SSID";     // Change this to your WiFi name
const char* WIFI_PASSWORD = "YOUR_PASSWORD"; // Change this to your WiFi password

// ThingsBoard configuration
const char* TB_SERVER = "thingsboard.cloud";
const int TB_PORT = 1883;
const char* TB_ACCESS_TOKEN = "YOUR_TOKEN"; // Get this from ThingsBoard device

// ===== STEP 3: DEFINE PINS =====
// Think of this like labeling wires
#define RED_LED D1        // Red LED for danger (D1 on NodeMCU)
#define GREEN_LED D2      // Green LED for normal (D2 on NodeMCU)
#define DHT_PIN D4        // DHT11 data pin (D4 on NodeMCU)
#define BUZZER D5        // Buzzer for alarm (D5 on NodeMCU)

// ===== STEP 4: SETUP SENSOR & CONNECTIVITY =====
DHT dht(DHT_PIN, DHT11);     // Create DHT sensor object
WiFiClient wifiClient;       // WiFi client
PubSubClient client(wifiClient); // MQTT client for ThingsBoard

// ===== STEP 5: DEFINE VARIABLES =====
float temperature;        // Store temperature value
float humidity;          // Store humidity value
bool is_fire = false;    // Is there fire? true/false

// Settings you can change:
float DANGER_TEMP = 32.0;  // Temperature limit (°C)
int LOOP_DELAY = 1000;     // Delay 1 seconds each loop

// ===== STEP 6: SETUP FUNCTION (RUNS ONCE) =====
void setup() {
  // Start communication with computer
  Serial.begin(115200);
  
  // Print welcome message
  Serial.println("================================");
  Serial.println("    FIRE ALARM WITH THINGSBOARD ");
  Serial.println("================================");
  Serial.println("   Danger limit: " + String(DANGER_TEMP) + "°C");
  Serial.println("   Loop delay: " + String(LOOP_DELAY/1000) + " seconds");
  Serial.println("--------------------------------");
  
  // Setup pins (tell Arduino what each pin does)
  pinMode(GREEN_LED, OUTPUT);  // Green LED = output
  pinMode(RED_LED, OUTPUT);    // Red LED = output  
  pinMode(BUZZER, OUTPUT);     // Buzzer = output
  
  // Start the sensor
  dht.begin();
  
  // Set starting condition (normal/safe)
  digitalWrite(GREEN_LED, HIGH);  // Green LED ON
  digitalWrite(RED_LED, LOW);     // Red LED OFF
  digitalWrite(BUZZER, LOW);      // Buzzer OFF
  
  // Connect to WiFi and ThingsBoard
  connectToWiFi();
  connectToThingsBoard();
  
  Serial.println(" System ready! Starting monitoring...");
  Serial.println("================================");
}

// ===== STEP 7: MAIN LOOP (RUNS FOREVER) =====
void loop() {
  // Keep WiFi and ThingsBoard connected
  if (!WiFi.isConnected()) {
    connectToWiFi();
  }
  if (!client.connected()) {
    connectToThingsBoard();
  }
  client.loop(); // Keep MQTT connection alive
  
  // Every loop: Check temperature and send data
  Serial.println(" Starting new loop cycle...");
  
  // 1. Read sensor data
  readTemperature();
  
  // 2. Check if there's danger
  checkForFire();
  
  // 3. Control LEDs and buzzer
  controlOutputs();
  
  // 4. Show information
  showStatus();
  
  // 5. Send data to ThingsBoard
  sendToThingsBoard();
  
  // 6. Wait 1 seconds before next loop
  Serial.println("========================================");
  delay(LOOP_DELAY);
}

// ===== STEP 8: CONNECTIVITY FUNCTIONS =====

// Connect to WiFi
void connectToWiFi() {
  Serial.println(" Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println(" WiFi Connected!");
  Serial.println(" IP Address: " + WiFi.localIP().toString());
}

// Connect to ThingsBoard
void connectToThingsBoard() {
  Serial.println("Connecting to ThingsBoard...");
  client.setServer(TB_SERVER, TB_PORT);
  
  while (!client.connected()) {
    Serial.print(".");
    if (client.connect("ESP8266Device", TB_ACCESS_TOKEN, NULL)) {
      Serial.println("");
      Serial.println("ThingsBoard Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

// Send data to ThingsBoard
void sendToThingsBoard() {
  if (client.connected()) {
    // Create alarm message: "danger" or "ok"
    String alarmStatus = is_fire ? "danger" : "ok";
    
    // Create JSON payload
    String payload = "{";
    payload += "\"temperature\":" + String(temperature) + ",";
    payload += "\"humidity\":" + String(humidity) + ",";
    payload += "\"fire_alarm\":\"" + alarmStatus + "\"";
    payload += "}";
    
    // Send to ThingsBoard
    char msg[150];
    payload.toCharArray(msg, 150);
    client.publish("v1/devices/me/telemetry", msg);
    
    Serial.println("Data sent to ThingsBoard:");
    Serial.println("   " + payload);
  } else {
    Serial.println("ThingsBoard not connected - data not sent");
  }
}

// ===== STEP 9: ORIGINAL FUNCTIONS (NO CHANGES) =====

// Function 1: Read temperature from sensor
void readTemperature() {
  Serial.println("Reading sensor...");
  
  // Get data from DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // Check if sensor is working
  if (isnan(temperature)) {
    Serial.println("Sensor error! Check connections!");
    return;
  }
  
  Serial.println("Sensor data received!");
}

// Function 2: Decide if there's fire danger
void checkForFire() {
  Serial.println("Checking for danger...");
  
  if (temperature >= DANGER_TEMP) {
    is_fire = true;  // DANGER!
    Serial.println("DANGER: Temperature too high!");
  } else {
    is_fire = false; // SAFE
    Serial.println("SAFE: Temperature normal");
  }
}

// Function 3: Control LEDs and buzzer based on danger
void controlOutputs() {
  if (is_fire == true) {
    // DANGER MODE
    Serial.println("Activating danger mode...");
    digitalWrite(GREEN_LED, LOW);   // Turn OFF green LED
    digitalWrite(RED_LED, HIGH);    // Turn ON red LED
    digitalWrite(BUZZER, HIGH);     // Turn ON buzzer
  } else {
    // NORMAL MODE  
    Serial.println("Activating normal mode...");
    digitalWrite(GREEN_LED, HIGH);  // Turn ON green LED
    digitalWrite(RED_LED, LOW);     // Turn OFF red LED
    digitalWrite(BUZZER, LOW);      // Turn OFF buzzer
  }
}

// Function 4: Show current status
void showStatus() {
  Serial.println("");
  Serial.println("=== STATUS REPORT ===");
  Serial.println("Temperature: " + String(temperature) + "°C");
  Serial.println("Humidity: " + String(humidity) + "%");
  Serial.println("Danger limit: " + String(DANGER_TEMP) + "°C");
  
  if (is_fire) {
    Serial.println("STATUS: FIRE DANGER!");
    Serial.println("Red LED: ON");
    Serial.println("Buzzer: ON");
    Serial.println("Alarm Status: DANGER");
  } else {
    Serial.println("STATUS: NORMAL");
    Serial.println("Green LED: ON");  
    Serial.println("Buzzer: OFF");
    Serial.println("Alarm Status: OK");
  }
  
  Serial.println("Time: " + String(millis()/1000) + " seconds");
  Serial.println("WiFi: " + String(WiFi.isConnected() ? "Connected" : "Disconnected"));
  Serial.println("ThingsBoard: " + String(client.connected() ? "Connected" : "Disconnected"));
  Serial.println("========================");
  Serial.println("");
}
