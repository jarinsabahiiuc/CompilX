if (-not (Test-Path ".\CompilX.exe")) {
    Write-Host "Executable not found. Build first." -ForegroundColor Yellow
    exit 1
}

& ".\CompilX.exe"
