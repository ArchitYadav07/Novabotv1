#include <WiFi.h>
#include <FirebaseESP32.h>

// --- Pin Definitions ---
const int L_FOR = 25; 
const int L_REV = 26; 
const int R_FOR = 27; 
const int R_REV = 14; 
const int LED_PIN = 2; 

// --- Wi-Fi Credentials ---
const char* WIFI_SSID = "NOVATECH1";      // <--- Put your actual WiFi name
const char* WIFI_PASSWORD = "123456789@"; // <--- Put your actual WiFi password

// --- Firebase Credentials ---
#define API_KEY "Fk0eUhJhyOGEeJAoFuGEnd1XnXyKqx1Wu5IlSb5q6bE"
// VERY IMPORTANT: Firebase library requires the URL WITHOUT https:// and without a trailing slash!
#define DATABASE_URL "novabot-298e9-default-rtdb.firebaseio.com"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// --- Drive Logic ---
void drive(int lf, int lr, int rf, int rr, String msg) {
  analogWrite(L_FOR, lf);
  analogWrite(L_REV, lr);
  analogWrite(R_FOR, rf);
  analogWrite(R_REV, rr);
  Serial.println(">>> MOTOR ACTION: " + msg);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(L_FOR, OUTPUT); pinMode(L_REV, OUTPUT);
  pinMode(R_FOR, OUTPUT); pinMode(R_REV, OUTPUT);

  // Bootup Blink
  for(int i=0; i<3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(200);
    digitalWrite(LED_PIN, LOW); delay(200);
  }

  Serial.println("\n--- NOVA BOT CLOUD STARTING ---");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWi-Fi [CONNECTED]");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Crucial for test mode if Firebase rules are set to public without specific Auth
  config.signer.test_mode = true; 

  // Optimize SSL memory to prevent "Failed to initialize the SSL layer"
  fbdo.setBSSLBufferSize(1024, 1024);

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  digitalWrite(LED_PIN, HIGH);
  Serial.println("SYSTEM READY: Listening for Netlify Commands...");
}

void loop() {
  // 1. Fetch commands from Firebase
  if (Firebase.getString(fbdo, "/commands/command")) {
    String cmd = fbdo.stringData();
    
    // 2. Prevent repeating the same logic if nothing changed
    static String lastCmd = "";
    if (cmd != "" && cmd != lastCmd) {
      lastCmd = cmd;
      Serial.print("CLOUD SIGNAL RECEIVED: ");
      Serial.println(cmd);
      
      // --- MOTOR MAPPING ---
      if (cmd == "N")        { drive(0, 220, 0, 220, "FORWARD"); }
      else if (cmd == "S")   { drive(220, 0, 220, 0, "BACKWARD"); }
      else if (cmd == "W")   { drive(0, 220, 220, 0, "LEFT TURN"); }
      else if (cmd == "E")   { drive(220, 0, 0, 220, "RIGHT TURN"); }
      else if (cmd == "NW")  { drive(0, 220, 0, 110, "NORTH-WEST"); }
      else if (cmd == "NE")  { drive(0, 110, 0, 220, "NORTH-EAST"); }
      else if (cmd == "SW")  { drive(220, 0, 110, 0, "SOUTH-WEST"); }
      else if (cmd == "SE")  { drive(110, 0, 220, 0, "SOUTH-EAST"); }
      else if (cmd == "STOP"){ drive(0, 0, 0, 0, "STOPPED"); }
    }
  } else {
    // Print out the exact error from Firebase if it fails
    // This helps us debug if it's an auth/permissions issue
    // Serial.println("Firebase Error: " + fbdo.errorReason());
  }

  // 4. Update Heartbeat status
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 4000) {
    lastUpdate = millis();
    Firebase.setString(fbdo, "/botStatus/status", "Online");
  }
}
