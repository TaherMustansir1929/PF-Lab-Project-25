# PowerShell script to find ALL DLL dependencies recursively
# This discovers missing transitive dependencies

param(
    [string]$ExePath = "main.exe",
    [string]$MSYSBin = "C:\msys64\mingw64\bin"
)

$objdump = Join-Path $MSYSBin "objdump.exe"

if (-not (Test-Path $objdump)) {
    Write-Host "Error: objdump not found at $objdump" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $ExePath)) {
    Write-Host "Error: Executable not found: $ExePath" -ForegroundColor Red
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Recursive DLL Dependency Scanner" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$allDlls = @{}
$toProcess = New-Object System.Collections.Queue
$toProcess.Enqueue($ExePath)

$systemDlls = @(
    "KERNEL32.dll", "USER32.dll", "GDI32.dll", "ADVAPI32.dll",
    "WS2_32.dll", "OLE32.dll", "SHELL32.dll", "msvcrt.dll",
    "COMDLG32.dll", "OLEAUT32.dll", "IMM32.dll", "WINMM.dll",
    "SETUPAPI.dll", "VERSION.dll", "bcrypt.dll", "CRYPT32.dll",
    "WINSPOOL.DRV", "COMCTL32.dll", "SHLWAPI.dll"
)

function Get-DllDependencies($filePath) {
    $output = & $objdump -p $filePath 2>$null | Select-String "DLL Name:"
    if ($null -eq $output) { return @() }
    
    $deps = $output | ForEach-Object { 
        ($_ -replace ".*DLL Name:\s*", "").Trim() 
    }
    return $deps
}

Write-Host "Scanning dependencies..." -ForegroundColor Yellow
$processed = @{}

while ($toProcess.Count -gt 0) {
    $current = $toProcess.Dequeue()
    $currentName = Split-Path $current -Leaf
    
    if ($processed.ContainsKey($currentName)) {
        continue
    }
    
    $processed[$currentName] = $true
    
    # Find the file
    $filePath = $null
    if (Test-Path $current) {
        $filePath = $current
    } elseif (Test-Path (Join-Path $MSYSBin $currentName)) {
        $filePath = Join-Path $MSYSBin $currentName
    } elseif (Test-Path (Join-Path "dist" $currentName)) {
        $filePath = Join-Path "dist" $currentName
    }
    
    if ($null -eq $filePath) {
        continue
    }
    
    $deps = Get-DllDependencies $filePath
    
    foreach ($dep in $deps) {
        $depLower = $dep.ToLower()
        
        # Skip system DLLs
        if ($systemDlls -contains $dep) {
            continue
        }
        
        if (-not $allDlls.ContainsKey($dep)) {
            $allDlls[$dep] = $true
            $toProcess.Enqueue($dep)
        }
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Found $($allDlls.Count) non-system DLLs" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$found = @()
$missing = @()

foreach ($dll in $allDlls.Keys | Sort-Object) {
    $inDist = Test-Path (Join-Path "dist" $dll)
    $inMSYS = Test-Path (Join-Path $MSYSBin $dll)
    
    if ($inDist) {
        Write-Host "  ✓ $dll" -ForegroundColor Green
        $found += $dll
    } elseif ($inMSYS) {
        Write-Host "  ✗ $dll (in MSYS2 but NOT in dist)" -ForegroundColor Red
        $missing += $dll
    } else {
        Write-Host "  ? $dll (not found anywhere)" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "In dist folder: $($found.Count)" -ForegroundColor Green
Write-Host "Missing from dist: $($missing.Count)" -ForegroundColor Red
Write-Host ""

if ($missing.Count -gt 0) {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "MISSING DLLs - Copy these to dist:" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host ""
    
    foreach ($dll in $missing) {
        $sourcePath = Join-Path $MSYSBin $dll
        Write-Host "Copy-Item '$sourcePath' -Destination 'dist\'" -ForegroundColor Yellow
    }
    
    Write-Host ""
    Write-Host "Or run this to copy all missing DLLs:" -ForegroundColor Cyan
    Write-Host ""
    $copyCmd = '@("' + ($missing -join '", "') + '") | ForEach-Object { Copy-Item "' + $MSYSBin + '\$_" -Destination "dist\" -Force }'
    Write-Host $copyCmd -ForegroundColor White
} else {
    Write-Host "✅ All required DLLs are in the dist folder!" -ForegroundColor Green
}
