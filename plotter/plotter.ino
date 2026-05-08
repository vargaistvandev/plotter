#include <ArduinoMotorCarrier.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <ArduinoMDNS.h>
#include "secrets.h"

// ---------------- WiFi ----------------
WiFiServer server(80);          // web page
WiFiServer controlServer(5000); // raw TCP control

// ---------------- mDNS ----------------
WiFiUDP udp;
MDNS mdns(udp);

// ---------------- Pen ----------------
#define PEN_DOWN 150
#define PEN_UP   90

void moveToTarget(long targetX, long targetY) {

  const int speed = 30;
  const long tolerance = 60;

  while (true) {

    long currentX = readMotor1Count();
    long currentY = readMotor2Count();

    long errorX = targetX - currentX;
    long errorY = targetY - currentY;

    bool doneX = labs(errorX) <= tolerance;
    bool doneY = labs(errorY) <= tolerance;

    // ---- Motor X ----
    if (!doneX) {
      if (errorX > 0) {
        M1.setDuty(speed);
      } else {
        M1.setDuty(-speed);
      }
    } else {
      M1.setDuty(0);
    }

    // ---- Motor Y ----
    if (!doneY) {
      if (errorY > 0) {
        M2.setDuty(speed);
      } else {
        M2.setDuty(-speed);
      }
    } else {
      M2.setDuty(0);
    }

    // ---- Finished? ----
    if (doneX && doneY) {
      break;
    }

    delay(10);

    // keep controller alive
    controller.ping();
    mdns.run();
  }

  // safety stop
  M1.setDuty(0);
  M2.setDuty(0);
}

void touchBlackPen() {
  servo3.setAngle(PEN_DOWN);
}

void liftPen() {
  servo3.setAngle(PEN_UP);
}

// ---------------- Helpers ----------------
float readBatteryVoltage() {
  return battery.getConverted();
}

int batteryPercentFromVoltage(float v) {
  // Simple estimate. Adjust these values for your battery pack.
  const float FULL_V  = 12.6;
  const float EMPTY_V = 9.0;

  float pct = (v - EMPTY_V) * 100.0 / (FULL_V - EMPTY_V);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return (int)(pct + 0.5);
}

long readMotor1Count() {
  return encoder1.getRawCount();   // change if your library uses a different name
}

long readMotor2Count() {
  return encoder2.getRawCount();   // change if your library uses a different name
}

String makeStatusJSON() {
  float v = readBatteryVoltage();
  int pct = batteryPercentFromVoltage(v);

  String json = "{";
  json += "\"batteryVoltage\":" + String(v, 2) + ",";
  json += "\"motor1\":" + String(readMotor1Count()) + ",";
  json += "\"batteryPercent\":" + String(pct) + ",";
  json += "\"motor2\":" + String(readMotor2Count()) + ",";
  json += "\"lowBattery\":" + String(v < 11.0 ? "true" : "false");
  json += "}";
  return json;
}

String makeHTML() {
  float v = readBatteryVoltage();
  int pct = batteryPercentFromVoltage(v);
  long c1 = readMotor1Count();
  long c2 = readMotor2Count();
  bool low = (v < 11.0);

  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='1'>";
  html += "<title>Plotter Status</title>";
  html += "<style>";
  html += "body{font-family:monospace;background:#111;color:#0f0;margin:0;padding:20px;}";
  html += ".card{max-width:520px;border:1px solid #0f0;padding:16px;border-radius:10px;}";
  html += ".row{display:flex;justify-content:space-between;gap:12px;padding:8px 0;border-bottom:1px solid #033;}";
  html += ".row:last-child{border-bottom:none;}";
  html += ".label{opacity:.8;}";
  html += ".value{font-weight:bold;}";
  html += ".warn{color:#ff8080;}";
  html += "</style></head><body>";
  html += "<div class='card'>";
  html += "<h2>Plotter Status</h2>";
  html += "<div class='row'><span class='label'>Battery</span><span class='value'>";
  html += String(pct) + "%</span></div>";
  html += "<div class='row'><span class='label'>Voltage</span><span class='value'>";
  html += String(v, 2) + " V</span></div>";
  html += "<div class='row'><span class='label'>Motor 1 counter</span><span class='value'>";
  html += String(c1) + "</span></div>";
  html += "<div class='row'><span class='label'>Motor 2 counter</span><span class='value'>";
  html += String(c2) + "</span></div>";
  html += "<div class='row'><span class='label'>Status</span><span class='value ";
  html += (low ? "warn" : "") + String("'>");
  html += (low ? "LOW BATTERY" : "OK");
  html += "</span></div>";
  html += "</div></body></html>";

  return html;
}

// ---------------- TCP control ----------------
void handleTcpCommand(String cmd, WiFiClient &client) {

  cmd.trim();

  // ---- Check shared secret ----
  int firstSpace = cmd.indexOf(' ');

  if (firstSpace == -1) {
    client.println("AUTH ERR");
    return;
  }

  String secret = cmd.substring(0, firstSpace);
  String actualCmd = cmd.substring(firstSpace + 1);

  if (secret != SHARED_SECRET) {
    client.println("AUTH ERR");
    return;
  }

  actualCmd.trim();
  actualCmd.toUpperCase();

  // ---- Commands ----
if (actualCmd.startsWith("MOVE ")) {

  int split = actualCmd.indexOf(' ', 5);

  if (split == -1) {
    client.println("ERR");
    return;
  }

  long targetX = actualCmd.substring(5, split).toInt();
  long targetY = actualCmd.substring(split + 1).toInt();

  moveToTarget(targetX, targetY);

  client.println("OK");
} else if (actualCmd == "PEN DOWN") {
  touchBlackPen();
  client.println("OK");
} else if (actualCmd == "PEN UP") {
  liftPen();
  client.println("OK");
} 


else if (actualCmd == "STOP") {
  M1.setDuty(0);
  M2.setDuty(0);
  client.println("OK");
}

else if (actualCmd == "STATUS") {
  client.print(makeStatusJSON());
}

else {
  client.println("ERR");
}

}

// ---------------- Setup ----------------
void setup() {
  if (!controller.begin()) {
    while (1) { }
  }

  controller.reboot();
  delay(500);

  unsigned long start = millis();
  bool wifiConnected = false;

  while (millis() - start < 10000) {
    if (WiFi.begin(WIFI_SSID, WIFI_PASS) == WL_CONNECTED) {
      wifiConnected = true;
      break;
    }
    delay(1000);
  }

  if (wifiConnected) {
    if (mdns.begin(WiFi.localIP())) {
      mdns.setName("plotter");
    }

    server.begin();
    controlServer.begin();
  }
}

// ---------------- Loop ----------------
void loop() {
  // Raw TCP control port for Python
  WiFiClient controlClient = controlServer.available();
  if (controlClient) {
    String cmd = controlClient.readStringUntil('\n');
    handleTcpCommand(cmd, controlClient);
    controlClient.stop();
  }

  // Web page
  WiFiClient webClient = server.available();
  if (webClient) {
    String req = webClient.readStringUntil('\r');
    webClient.flush();

    if (req.indexOf("GET / ") != -1 || req.indexOf("GET /HTTP") != -1) {
      webClient.println("HTTP/1.1 200 OK");
      webClient.println("Content-Type: text/html");
      webClient.println("Connection: close");
      webClient.println();
      webClient.println(makeHTML());
    }
    else if (req.indexOf("GET /status") != -1) {
      webClient.println("HTTP/1.1 200 OK");
      webClient.println("Content-Type: application/json");
      webClient.println("Connection: close");
      webClient.println();
      webClient.println(makeStatusJSON());
    }
    else {
      webClient.println("HTTP/1.1 200 OK");
      webClient.println("Content-Type: text/plain");
      webClient.println("Connection: close");
      webClient.println();
      webClient.println("OK");
    }

    webClient.stop();
  }

  // Safety cutoff
  float batteryVoltage = readBatteryVoltage();
  if (batteryVoltage < 11.0) {
    M1.setDuty(0);
    M2.setDuty(0);
  }

  controller.ping();
  mdns.run();
}