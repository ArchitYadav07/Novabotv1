#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- Configuration ---
const char* ssid = "NovaBot-AP";
const char* password = ""; // Open network for easy access

AsyncWebServer server(80);

// --- HTML Dashboard (Sleek Dark Mode) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Nova Bot Control Center</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600;700&display=swap');
        
        :root {
            --primary-bg: #0a0a0c;
            --card-bg: rgba(255, 255, 255, 0.05);
            --accent-glow: #00f2ff;
            --text-main: #ffffff;
            --text-dim: #a0a0a0;
        }

        body {
            margin: 0;
            font-family: 'Inter', sans-serif;
            background-color: var(--primary-bg);
            background-image: 
                radial-gradient(circle at 20% 20%, rgba(0, 242, 255, 0.05) 0%, transparent 40%),
                radial-gradient(circle at 80% 80%, rgba(188, 0, 255, 0.05) 0%, transparent 40%);
            color: var(--text-main);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-between;
            min-height: 100vh;
            text-align: center;
            overflow: hidden;
        }

        .header {
            padding: 40px 20px;
            width: 100%;
        }

        .status-badge {
            display: inline-block;
            padding: 8px 20px;
            background: rgba(0, 242, 255, 0.1);
            border: 1px solid rgba(0, 242, 255, 0.3);
            border-radius: 50px;
            color: var(--accent-glow);
            font-size: 0.9rem;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 1px;
            box-shadow: 0 0 15px rgba(0, 242, 255, 0.2);
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { opacity: 0.8; }
            50% { opacity: 1; box-shadow: 0 0 25px rgba(0, 242, 255, 0.4); }
            100% { opacity: 0.8; }
        }

        .main-content {
            flex-grow: 1;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            width: 90%;
            max-width: 400px;
        }

        .clock {
            font-size: 5rem;
            font-weight: 700;
            margin: 0;
            background: linear-gradient(135deg, #fff 0%, var(--accent-glow) 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            text-shadow: 0 0 40px rgba(0, 242, 255, 0.3);
        }

        .welcome-text {
            font-size: 1.8rem;
            font-weight: 600;
            margin-top: 10px;
            letter-spacing: -0.5px;
        }

        .robot-icon {
            width: 120px;
            height: 120px;
            margin-bottom: 20px;
            filter: drop-shadow(0 0 20px var(--accent-glow));
        }

        .footer {
            padding: 30px;
            font-size: 0.8rem;
            color: var(--text-dim);
            letter-spacing: 0.5px;
            text-transform: uppercase;
            border-top: 1px solid rgba(255, 255, 255, 0.05);
            width: 100%;
        }

        /* Responsive */
        @media (max-height: 600px) {
            .clock { font-size: 3.5rem; }
            .robot-icon { width: 80px; height: 80px; }
            .header { padding: 20px 10px; }
        }
    </style>
</head>
<body>
    <div class="header">
        <div class="status-badge" id="mode">Interaction Mode</div>
    </div>

    <div class="main-content">
        <svg class="robot-icon" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M12 2C6.47715 2 2 6.47715 2 12C2 17.5228 6.47715 22 12 22C17.5228 22 22 17.5228 22 12C22 6.47715 17.5228 2 12 2Z" stroke="#00f2ff" stroke-width="1.5"/>
            <path d="M8 11V13M16 11V13" stroke="#00f2ff" stroke-width="2" stroke-linecap="round"/>
            <path d="M9 16C9.85148 16.6305 10.8841 17 12 17C13.1159 17 14.1485 16.6305 15 16" stroke="#00f2ff" stroke-width="1.5" stroke-linecap="round"/>
        </svg>
        <div class="clock" id="time">00:00</div>
        <div class="welcome-text">Welcome to Nova Bot</div>
    </div>

    <div class="footer">
        Developed by NovaTech Robo Pvt Ltd
    </div>

    <script>
        function updateTime() {
            const now = new Date();
            const hours = String(now.getHours()).padStart(2, '0');
            const minutes = String(now.getMinutes()).padStart(2, '0');
            document.getElementById('time').textContent = `${hours}:${minutes}`;
        }
        
        setInterval(updateTime, 1000);
        updateTime();

        // Optional: Interaction Mode toggle status visually
        document.body.onclick = () => {
             const badge = document.getElementById('mode');
             if(badge.textContent === "Interaction Mode") {
                 badge.textContent = "Processing Mode";
                 badge.style.color = "#ff0080";
                 badge.style.borderColor = "rgba(255, 0, 128, 0.3)";
                 badge.style.background = "rgba(255, 0, 128, 0.1)";
                 badge.style.boxShadow = "0 0 15px rgba(255, 0, 128, 0.2)";
             } else {
                 badge.textContent = "Interaction Mode";
                 badge.style.color = "#00f2ff";
                 badge.style.borderColor = "rgba(0, 242, 255, 0.3)";
                 badge.style.background = "rgba(0, 242, 255, 0.1)";
                 badge.style.boxShadow = "0 0 15px rgba(0, 242, 255, 0.2)";
             }
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);

    // Set up Access Point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.begin();
    Serial.println("Server started");
}

void loop() {
    // Nothing here for AsyncWebServer
}
