$PythonProject = $PSScriptRoot

$PythonExe = Join-Path $PythonProject "\python\.venv\Scripts\python.exe"
$PythonScript = Join-Path $PythonProject "example.py"

Write-Host "Running Python script..."

& $PythonExe $PythonScript