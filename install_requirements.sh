#!/bin/bash

set -e

VERSION="1.3.1"

curl -fsSL \
https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh \
| sudo BINDIR=/usr/local/bin sh -s ${VERSION}

arduino-cli version

arduino-cli core update-index
arduino-cli core install arduino:samd
arduino-cli lib install "ArduinoMotorCarrier@2.0.3"
arduino-cli lib install "WiFiNINA@2.0.1"
arduino-cli lib install "ArduinoMDNS@1.0.1"
arduino-cli lib install "WiFi101"
