#include <WiFi.h>
#include <FirebaseESP32.h>

// --- Pin Definitions (From your local code) ---
const int L_FOR = 25; 
const int L_REV = 26; 
const int R_FOR = 27; 
const int R_REV = 14; 
const int LED_PIN = 2; 

// --- Cloud Credentials ---
const char* WIFI_SSID = "NOVATECH1";
const char* WIFI_PASSWORD = "123456789@";
#define API_KEY "Fk0eUhJhyOGEeJAoFuGEnd1XnXyKqx1Wu5IlSb5q6bE"
#define DATABASE_URL "https://novabot-298e9-default-rtdb.firebaseio.com/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// --- Drive Logic ---
void drive(int lf, int lr, int rf, int rr, String msg) {
  analogWrite(L_FOR, lf);
  analogWrite(L_REV, lr);
  analogWrite(R_FOR, rf);
  analogWrite(R_REV, rr);
  Serial.println(">>> ACTION: " + msg);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(L_FOR, OUTPUT); pinMode(L_REV, OUTPUT);
  pinMode(R_FOR, OUTPUT); pinMode(R_REV, OUTPUT);

  // Blink showing bootup
  for(int i=0; i<3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(200);
    digitalWrite(LED_PIN, LOW); delay(200);
  }

  Serial.println("\n--- NOVA BOT CLOUD INITIALIZING ---");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  digitalWrite(LED_PIN, HIGH); // LED stays on when cloud is ready
  Serial.println("System Ready. Controlling via Netlify.");
}

void loop() {
  // Listen for commands from the Netlify Website
  if (Firebase.getJSON(fbdo, "/commands")) {
    String jsonString = fbdo.jsonString();
    
    if (jsonString.indexOf("command") != -1) {
      int startIdx = jsonString.indexOf("\"command\":\"") + 11;
      int endIdx = jsonString.indexOf("\"", startIdx);
      String cmd = jsonString.substring(startIdx, endIdx);
      
      if (cmd != "") {
        Serial.println("CLOUD COMMAND: " + cmd);
        
        // --- MOTOR MAPPING ---
        if (cmd == "N")   { drive(220, 0, 220, 0, "FORWARD"); }
        else if (cmd == "S")   { drive(0, 220, 0, 220, "BACK"); }
        else if (cmd == "W")   { drive(0, 220, 220, 0, "LEFT"); }
        else if (cmd == "E")   { drive(220, 0, 0, 220, "RIGHT"); }
        else if (cmd == "NW")  { drive(150, 0, 220, 0, "NW_TURN"); }
        else if (cmd == "NE")  { drive(220, 0, 150, 0, "NE_TURN"); }
        else if (cmd == "STOP"){ drive(0, 0, 0, 0, "STOP"); }
      }
    }
  }

  // Sync status back to dashboard
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    lastUpdate = millis();
    Firebase.setString(fbdo, "/botStatus/status", "Online");
    Firebase.setString(fbdo, "/botStatus/mode", "Cloud Manual");
  }
}
