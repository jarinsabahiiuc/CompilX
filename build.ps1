$ErrorActionPreference = "Stop"

Write-Host "`nBuilding CompilX..." -ForegroundColor Cyan

$sourceFiles = @(".\main.cpp")

$sourceFiles += Get-ChildItem -Path ".\src" -Filter "*.cpp" -File |
    ForEach-Object { $_.FullName }

& g++ -std=c++17 -Wall -Wextra -g -I ".\include" @sourceFiles -o ".\CompilX.exe"

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "`nBuild successful!" -ForegroundColor Green
Write-Host "Executable: CompilX.exe" -ForegroundColor Green
