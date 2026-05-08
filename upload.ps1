$Project = Join-Path $PSScriptRoot "plotter"
$Board = "arduino:samd:mkr1000"
$InstallDir = $PSScriptRoot
$CLI = "$InstallDir\arduino-cli.exe"

# Detect port automatically
$BoardInfo = & $CLI board list | Select-String "arduino:"
$Port = ($BoardInfo -split '\s+')[0]

Write-Host "Detected port: $Port"

Set-Location $Project

Write-Host "Compiling..."
& $CLI compile --fqbn $Board

Write-Host "Uploading..."
& $CLI upload -p $Port --fqbn $Board

Write-Host "Done."