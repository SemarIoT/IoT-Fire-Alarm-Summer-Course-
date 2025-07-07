/*
==========================================
          SUMMER COURSE PROGRAM
IOT INTEGRATED FIRE ALARM DETECTION SYSTEM
==========================================

WHAT THIS PROGRAM DOES:
- Reads temperature from DHT11 sensor
- If temperature > 30°C → FIRE ALARM ON
- If temperature ≤ 30°C → NORMAL MODE
*/

// ===== STEP 1: INCLUDE LIBRARIES =====
#include <DHT.h>  // Library for DHT11 sensor

// ===== STEP 2: DEFINE PINS =====
// Think of this like labeling wires
#define RED_LED D1        // Red LED for danger (D1 on NodeMCU)
#define GREEN_LED D2      // Green LED for normal (D2 on NodeMCU)
#define DHT_PIN D4        // DHT11 data pin (D4 on NodeMCU)
#define BUZZER D5        // Buzzer for alarm (D5 on NodeMCU)

// ===== STEP 3: SETUP SENSOR =====
DHT dht(DHT_PIN, DHT11);  // Create DHT sensor object

// ===== STEP 4: DEFINE VARIABLES =====
float temperature;        // Store temperature value
float humidity;          // Store humidity value
bool is_fire = false;    // Is there fire? true/false

// Settings you can change:
float DANGER_TEMP = 32.0;  // Temperature limit (°C)
int CHECK_TIME = 2000;     // Check every 2 seconds

// ===== STEP 5: SETUP FUNCTION (RUNS ONCE) =====
void setup() {
  // Start communication with computer
  Serial.begin(115200);
  
  // Print welcome message
  Serial.println("================================");
  Serial.println("    FIRE ALARM                  ");
  Serial.println("================================");
  Serial.println("   Danger limit: " + String(DANGER_TEMP) + "°C");
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
  
  Serial.println("System ready! Starting monitoring...");
  Serial.println("================================");
}

// ===== STEP 6: MAIN LOOP (RUNS FOREVER) =====
void loop() {
  // 1. Read sensor data
  readTemperature();
  
  // 2. Check if there's danger
  checkForFire();
  
  // 3. Control LEDs and buzzer
  controlOutputs();
  
  // 4. Show information
  showStatus();
  
  // 5. Wait before next check
  delay(CHECK_TIME);
}

// ===== STEP 7: FUNCTIONS (LIKE MINI PROGRAMS) =====

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
  } else {
    Serial.println("STATUS: NORMAL");
    Serial.println("Green LED: ON");  
    Serial.println("Buzzer: OFF");
  }
  
  Serial.println("Time: " + String(millis()/1000) + " seconds");
  Serial.println("========================");
  Serial.println("");
}
