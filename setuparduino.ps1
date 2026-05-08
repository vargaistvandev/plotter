$Version = "1.3.1"

# Directory where this script is located
$InstallDir = $PSScriptRoot

Write-Host "Downloading Arduino CLI..."

Invoke-WebRequest `
  -Uri "https://downloads.arduino.cc/arduino-cli/arduino-cli_${Version}_Windows_64bit.zip" `
  -OutFile "$InstallDir\arduino-cli.zip"

Expand-Archive `
  "$InstallDir\arduino-cli.zip" `
  -DestinationPath $InstallDir `
  -Force

$CLI = "$InstallDir\arduino-cli.exe"

& $CLI version

& $CLI core update-index
& $CLI core install arduino:samd

& $CLI lib install "ArduinoMotorCarrier@2.0.3"
& $CLI lib install "WiFiNINA@2.0.1"
& $CLI lib install "ArduinoMDNS@1.0.1"
& $CLI lib install "WiFi101"