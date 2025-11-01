# PowerShell script to create a single-file self-extracting installer
# This bundles the executable and all DLLs into one installer

param(
    [string]$AppName = "PF-Lab-Project",
    [string]$Version = "1.0"
)

$OutputInstaller = "$AppName-$Version-Setup.exe"
$DIST_DIR = "dist"

Write-Host "========================================" -ForegroundColor Green
Write-Host "Creating Single-File Installer" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# Check if dist folder exists
if (-not (Test-Path $DIST_DIR)) {
    Write-Host "Error: dist folder not found!" -ForegroundColor Red
    Write-Host "Run 'make dist' or '.\bundle-dlls.ps1' first!" -ForegroundColor Yellow
    exit 1
}

# Check if 7-Zip is installed
$7zipPaths = @(
    "C:\Program Files\7-Zip\7z.exe",
    "C:\Program Files (x86)\7-Zip\7z.exe",
    "$env:ProgramFiles\7-Zip\7z.exe"
)

$7zip = $null
foreach ($path in $7zipPaths) {
    if (Test-Path $path) {
        $7zip = $path
        break
    }
}

if ($null -eq $7zip) {
    Write-Host "Error: 7-Zip not found!" -ForegroundColor Red
    Write-Host "Please install 7-Zip from: https://www.7-zip.org/" -ForegroundColor Yellow
    Write-Host "" -ForegroundColor Yellow
    Write-Host "Alternative: Create a ZIP file instead?" -ForegroundColor Cyan
    $createZip = Read-Host "Create ZIP file? (Y/N)"
    
    if ($createZip -eq "Y" -or $createZip -eq "y") {
        $ZipFile = "$AppName-$Version-Portable.zip"
        Compress-Archive -Path "$DIST_DIR\*" -DestinationPath $ZipFile -Force
        Write-Host "`n✅ Created: $ZipFile" -ForegroundColor Green
        Write-Host "Users can extract and run main.exe" -ForegroundColor Cyan
    }
    exit 0
}

Write-Host "Found 7-Zip at: $7zip" -ForegroundColor Cyan

# Create SFX config file
$SfxConfig = @"
;!@Install@!UTF-8!
Title="$AppName $Version Setup"
BeginPrompt="This will install $AppName $Version.\n\nClick OK to continue."
ExtractDialogText="Extracting files..."
ExtractPathText="Installation folder:"
ExtractTitle="Installing $AppName..."
GUIFlags="8+32+64+256+4096"
GUIMode="1"
OverwriteMode="2"
ExecuteFile="main.exe"
;!@InstallEnd@!
"@

$ConfigFile = "sfx-config.txt"
$SfxConfig | Out-File -FilePath $ConfigFile -Encoding ASCII

Write-Host "Creating self-extracting archive..." -ForegroundColor Cyan

# Create 7z archive
$ArchiveFile = "temp-archive.7z"
& $7zip a -t7z -mx9 $ArchiveFile "$DIST_DIR\*" | Out-Null

if (-not (Test-Path $ArchiveFile)) {
    Write-Host "Error: Failed to create archive!" -ForegroundColor Red
    Remove-Item $ConfigFile -ErrorAction SilentlyContinue
    exit 1
}

# Find 7z SFX module
$SfxModule = Join-Path (Split-Path $7zip) "7zSD.sfx"
if (-not (Test-Path $SfxModule)) {
    Write-Host "Error: 7z SFX module not found at: $SfxModule" -ForegroundColor Red
    Write-Host "Creating simple ZIP instead..." -ForegroundColor Yellow
    
    $ZipFile = "$AppName-$Version-Portable.zip"
    Compress-Archive -Path "$DIST_DIR\*" -DestinationPath $ZipFile -Force
    
    # Cleanup
    Remove-Item $ArchiveFile -ErrorAction SilentlyContinue
    Remove-Item $ConfigFile -ErrorAction SilentlyContinue
    
    Write-Host "`n✅ Created: $ZipFile" -ForegroundColor Green
    exit 0
}

# Combine SFX module + config + archive
Write-Host "Building self-extracting installer..." -ForegroundColor Cyan

# Read files as bytes
$sfxBytes = [System.IO.File]::ReadAllBytes($SfxModule)
$configBytes = [System.IO.File]::ReadAllBytes($ConfigFile)
$archiveBytes = [System.IO.File]::ReadAllBytes($ArchiveFile)

# Write combined file
$outputStream = [System.IO.File]::OpenWrite($OutputInstaller)
$outputStream.Write($sfxBytes, 0, $sfxBytes.Length)
$outputStream.Write($configBytes, 0, $configBytes.Length)
$outputStream.Write($archiveBytes, 0, $archiveBytes.Length)
$outputStream.Close()

# Cleanup temporary files
Remove-Item $ArchiveFile -ErrorAction SilentlyContinue
Remove-Item $ConfigFile -ErrorAction SilentlyContinue

# Get file size
$FileSize = (Get-Item $OutputInstaller).Length / 1MB

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "✅ SUCCESS!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "`nCreated: $OutputInstaller" -ForegroundColor Cyan
Write-Host "Size: $([math]::Round($FileSize, 2)) MB" -ForegroundColor Cyan
Write-Host "`nThis is a SINGLE executable file that:" -ForegroundColor Yellow
Write-Host "  • Contains your app and all DLLs" -ForegroundColor White
Write-Host "  • Extracts files to user-chosen folder" -ForegroundColor White
Write-Host "  • Automatically launches main.exe after extraction" -ForegroundColor White
Write-Host "  • Works on any Windows computer (no install required)" -ForegroundColor White
Write-Host "`nJust send '$OutputInstaller' to users!" -ForegroundColor Green
