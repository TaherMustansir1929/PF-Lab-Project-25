# PowerShell script to verify all DLL dependencies are bundled
# This uses objdump or dumpbin to check actual dependencies

param(
    [string]$ExePath = "dist\astrogon.exe"
)

if (-not (Test-Path $ExePath)) {
    Write-Host "Error: Executable not found at: $ExePath" -ForegroundColor Red
    exit 1
}

$distDir = Split-Path $ExePath -Parent

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Checking DLL Dependencies" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Executable: $ExePath" -ForegroundColor White
Write-Host ""

# Try to use objdump from MSYS2
$objdump = $null
$objdumpPaths = @(
    "C:\msys64\mingw64\bin\objdump.exe",
    "C:\msys64\ucrt64\bin\objdump.exe",
    "C:\msys64\clang64\bin\objdump.exe"
)

foreach ($path in $objdumpPaths) {
    if (Test-Path $path) {
        $objdump = $path
        break
    }
}

if ($null -eq $objdump) {
    Write-Host "Note: objdump not found. Cannot verify dependencies automatically." -ForegroundColor Yellow
    Write-Host "To manually test:" -ForegroundColor Cyan
    Write-Host "  cd $distDir" -ForegroundColor White
    Write-Host "  .\$(Split-Path $ExePath -Leaf)" -ForegroundColor White
    Write-Host ""
    Write-Host "If you get DLL errors, the missing DLLs will be shown." -ForegroundColor Yellow
    exit 0
}

Write-Host "Using objdump: $objdump" -ForegroundColor Green
Write-Host ""

# Get dependencies
$output = & $objdump -p $ExePath | Select-String "DLL Name:"

if ($null -eq $output) {
    Write-Host "Could not determine dependencies." -ForegroundColor Yellow
    exit 0
}

$dependencies = $output | ForEach-Object { ($_ -replace ".*DLL Name:\s*", "").Trim() }

Write-Host "Found $($dependencies.Count) DLL dependencies:" -ForegroundColor Cyan
Write-Host ""

$missing = @()
$found = @()
$system = @()

foreach ($dll in $dependencies) {
    $dllLower = $dll.ToLower()
    
    # Skip Windows system DLLs
    if ($dllLower -like "kernel32.dll" -or 
        $dllLower -like "user32.dll" -or 
        $dllLower -like "gdi32.dll" -or 
        $dllLower -like "advapi32.dll" -or 
        $dllLower -like "ws2_32.dll" -or 
        $dllLower -like "ole32.dll" -or 
        $dllLower -like "shell32.dll" -or 
        $dllLower -like "msvcrt.dll" -or
        $dllLower -like "bcrypt.dll" -or
        $dllLower -like "crypt32.dll" -or
        $dllLower -like "winmm.dll" -or
        $dllLower -like "imm32.dll" -or
        $dllLower -like "setupapi.dll" -or
        $dllLower -like "version.dll") {
        $system += $dll
        continue
    }
    
    # Check if DLL exists in dist folder
    $dllPath = Join-Path $distDir $dll
    if (Test-Path $dllPath) {
        Write-Host "  ✓ $dll" -ForegroundColor Green
        $found += $dll
    } else {
        Write-Host "  ✗ $dll (MISSING!)" -ForegroundColor Red
        $missing += $dll
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Found in dist: $($found.Count)" -ForegroundColor Green
Write-Host "System DLLs: $($system.Count)" -ForegroundColor Gray
if ($missing.Count -gt 0) {
    Write-Host "Missing: $($missing.Count)" -ForegroundColor Red
    Write-Host ""
    Write-Host "MISSING DLLs:" -ForegroundColor Red
    foreach ($dll in $missing) {
        Write-Host "  - $dll" -ForegroundColor Yellow
    }
    Write-Host ""
    Write-Host "Action required:" -ForegroundColor Yellow
    Write-Host "Copy these DLLs from C:\msys64\mingw64\bin\ to the dist folder:" -ForegroundColor White
    foreach ($dll in $missing) {
        $sourcePath = "C:\msys64\mingw64\bin\$dll"
        if (Test-Path $sourcePath) {
            Write-Host "  Copy-Item '$sourcePath' -Destination '$distDir\'" -ForegroundColor Cyan
        }
    }
} else {
    Write-Host ""
    Write-Host "✅ All required DLLs are present!" -ForegroundColor Green
    Write-Host ""
    Write-Host "The application is ready to distribute." -ForegroundColor Cyan
    Write-Host "Copy the entire '$distDir' folder to another computer." -ForegroundColor Cyan
}
