#include <ArduinoMotorCarrier.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <ArduinoMDNS.h>
#include "secrets.h"

// ---- WiFi ----
//char ssid[] = "SSID";
//char pass[] = "PASSWORD";
WiFiServer server(80);

// ---- mDNS ----
WiFiUDP udp;
MDNS mdns(udp);

// ---- Logging ----
String logBuffer = "";

void logMsg(String msg) {
  logBuffer += msg + "\n";

  // prevent memory blowup
  if (logBuffer.length() > 2000) {
    logBuffer = logBuffer.substring(logBuffer.length() - 1500);
  }
}

// ---- Pen ----
#define PEN_DOWN 150
#define PEN_UP   90

#define STEP_DURATION 80
#define STEP_SETTLE   50

void touchBlackPen() {
  servo3.setAngle(PEN_DOWN);
  logMsg("Pen DOWN");
}

void liftPen() {
  servo3.setAngle(PEN_UP);
  logMsg("Pen UP");
}

// ---- Step ----
void stepMotor(int motor, int speed, int durationMs) {
  speed = constrain(speed, -100, 100);

  if (motor == 1) M1.setDuty(speed);
  if (motor == 2) M2.setDuty(speed);

  delay(durationMs);

  M1.setDuty(0);
  M2.setDuty(0);

  delay(STEP_SETTLE);
}

// ---- HTML page ----
String makeHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<title>Plotter</title>";
  html += "<style>body{font-family:monospace;background:#111;color:#0f0;}</style>";
  html += "</head><body>";
  html += "<h2>Plotter Log</h2>";
  html += "<pre>" + logBuffer + "</pre>";
  html += "</body></html>";
  return html;
}

// ---- Setup ----
void setup() {

  if (!controller.begin()) {
    while (1); // no serial fallback anymore
  }

  controller.reboot();
  delay(500);

  // ---- WiFi connect (with timeout!) ----
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
    logMsg("WiFi connected");
    uint32_t ip = WiFi.localIP();

String ipStr = String((ip >> 24) & 0xFF) + "." +
               String((ip >> 16) & 0xFF) + "." +
               String((ip >> 8) & 0xFF) + "." +
               String(ip & 0xFF);

logMsg("IP: " + ipStr);

    if (mdns.begin(WiFi.localIP())) {
      mdns.setName("plotter");
      logMsg("mDNS: plotter.local");
    } else {
      logMsg("mDNS failed");
    }

    server.begin();
  } else {
    logMsg("WiFi failed");
  }
}

// ---- Loop ----
void loop() {

  float batteryVoltage = battery.getConverted();

  if (batteryVoltage < 11) {
    logMsg("LOW BATTERY - STOP");
    M1.setDuty(0);
    M2.setDuty(0);
    delay(500);
    return;
  }

  WiFiClient client = server.available();

  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    logMsg(req);

    handleRequest(req);

    // ---- Serve HTML on root ----
    if (req.indexOf("GET / ") != -1) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println(makeHTML());
    } else {
      // API response
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("OK");
    }

    client.stop();
  }

  controller.ping();
  mdns.run();
}

// ---- Request handler ----
void handleRequest(String req) {

  if (req.indexOf("/x?") != -1) {
    int speed = getParam(req, "speed");
    M1.setDuty(constrain(speed, -100, 100));
    logMsg("X speed " + String(speed));
  }

  else if (req.indexOf("/y?") != -1) {
    int speed = getParam(req, "speed");
    M2.setDuty(constrain(speed, -100, 100));
    logMsg("Y speed " + String(speed));
  }

  else if (req.indexOf("/stepx?") != -1) {
    int val = getParam(req, "val");
    stepMotor(1, val, STEP_DURATION);
    logMsg("Step X " + String(val));
  }

  else if (req.indexOf("/stepy?") != -1) {
    int val = getParam(req, "val");
    stepMotor(2, val, STEP_DURATION);
    logMsg("Step Y " + String(val));
  }

  else if (req.indexOf("/stop") != -1) {
    M1.setDuty(0);
    M2.setDuty(0);
    logMsg("STOP");
  }

  else if (req.indexOf("/down") != -1) {
    touchBlackPen();
  }

  else if (req.indexOf("/up") != -1) {
    liftPen();
  }
}

// ---- Param parser ----
int getParam(String req, String key) {
  int start = req.indexOf(key + "=");
  if (start == -1) return 0;

  start += key.length() + 1;
  int end = req.indexOf(' ', start);

  return req.substring(start, end).toInt();
}
