# Alternative: Use WinRAR or other tool to create a single executable
# This script creates a simple self-extracting ZIP using PowerShell

param(
    [string]$AppName = "PF-Lab-Project",
    [string]$Version = "1.0"
)

$DIST_DIR = "dist"
$OutputFile = "$AppName-$Version-Portable.zip"

Write-Host "========================================" -ForegroundColor Green
Write-Host "Creating Portable Package" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# Check if dist folder exists
if (-not (Test-Path $DIST_DIR)) {
    Write-Host "Error: dist folder not found!" -ForegroundColor Red
    Write-Host "Run 'make dist' or '.\bundle-dlls.ps1' first!" -ForegroundColor Yellow
    exit 1
}

Write-Host "Creating ZIP archive..." -ForegroundColor Cyan
Compress-Archive -Path "$DIST_DIR\*" -DestinationPath $OutputFile -Force

if (Test-Path $OutputFile) {
    $FileSize = (Get-Item $OutputFile).Length / 1MB
    
    Write-Host "`n========================================" -ForegroundColor Green
    Write-Host "✅ SUCCESS!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "`nCreated: $OutputFile" -ForegroundColor Cyan
    Write-Host "Size: $([math]::Round($FileSize, 2)) MB" -ForegroundColor Cyan
    Write-Host "`nTo use:" -ForegroundColor Yellow
    Write-Host "  1. Send this ZIP file to users" -ForegroundColor White
    Write-Host "  2. Users extract to any folder" -ForegroundColor White
    Write-Host "  3. Users run main.exe from extracted folder" -ForegroundColor White
    Write-Host "`nNo installation or dependencies needed!" -ForegroundColor Green
    
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "Want a TRUE single .exe file?" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Install 7-Zip and run: .\create-installer.ps1" -ForegroundColor White
    Write-Host "Download: https://www.7-zip.org/" -ForegroundColor Yellow
} else {
    Write-Host "Error: Failed to create archive!" -ForegroundColor Red
    exit 1
}
