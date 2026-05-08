$Project = Join-Path $PSScriptRoot "python"

Write-Host "Creating virtual environment..."

python -m venv "$Project\.venv"

Write-Host "Activating virtual environment..."

& "$Project\.venv\Scripts\Activate.ps1"

Write-Host "Upgrading pip..."

python -m pip install --upgrade pip

Write-Host "Installing dependencies..."

pip install requests


Write-Host ""
Write-Host "Done."
Write-Host ""
Write-Host "To activate later:"
Write-Host "$Project\.venv\Scripts\Activate.ps1"