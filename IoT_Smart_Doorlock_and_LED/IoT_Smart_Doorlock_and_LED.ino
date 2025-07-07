#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi
const char* ssid = "WIFI_USERNAME";
const char* password = "WIFI_PASSWORD";

// ThingsBoard
const char* tb_server = "thingsboard.cloud";
const int tb_port = 1883;
const char* tb_token = "TOKEN_THINGSBOARD";

// Pin Mapping
#define LED1_PIN D1  // GPIO5
#define LED2_PIN D2  // GPIO4
#define RELAY_PIN D3 // GPIO0

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi: Koneksi ke WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi: Koneksi ke ThingsBoard
void connectThingsBoard() {
  client.setServer(tb_server, tb_port);
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("ESP8266", tb_token, NULL)) {
      Serial.println(" connected!");
      // Subscribe ke kontrol LED dan Relay
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

// Fungsi: Callback jika ada perintah masuk dari ThingsBoard
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Received RPC call...");
  
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Payload: " + message);

  // Ekstrak perintah dari JSON
  if (message.indexOf("LED1") > 0) {
    if (message.indexOf("true") > 0) {
      digitalWrite(LED1_PIN, HIGH);
      Serial.println("LED1 ON");
    } else {
      digitalWrite(LED1_PIN, LOW);
      Serial.println("LED1 OFF");
    }
  } else if (message.indexOf("LED2") > 0) {
    if (message.indexOf("true") > 0) {
      digitalWrite(LED2_PIN, HIGH);
      Serial.println("LED2 ON");
    } else {
      digitalWrite(LED2_PIN, LOW);
      Serial.println("LED2 OFF");
    }
  } else if (message.indexOf("Relay") > 0) {
    if (message.indexOf("true") > 0) {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay ON");
    } else {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay OFF");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Setup GPIO sebagai output
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Default semua OFF
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  connectWiFi();
  client.setCallback(callback);
  connectThingsBoard();
}

void loop() {
  if (!client.connected()) {
    connectThingsBoard();
  }
  client.loop();
}
