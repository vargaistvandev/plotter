Setup
1. Install libraries
ArduinoMotorCarrier
WiFi101
ArduinoMDNS
2. Flash Motor Carrier firmware

Use the official firmware updater example from:

Arduino Motor Carrier Firmware Update Guide

Run the firmware updater once.

3. Upload plotter firmware

Upload the main sketch.

4. Configure Wi-Fi

Create secrets.h:

#define WIFI_SSID "your_wifi"
#define WIFI_PASS "your_password"

const char SHARED_SECRET[] = "shared_secret_value";
5. Connect
Web UI:
http://plotter.local
TCP control:
plotter.local:5000