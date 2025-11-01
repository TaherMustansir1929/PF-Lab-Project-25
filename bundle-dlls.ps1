# PowerShell script to bundle all required DLLs with the executable for Windows distribution

$DIST_DIR = "dist"

# Detect MSYS2 installation paths
$possiblePaths = @(
    "C:\msys64\mingw64\bin",
    "C:\msys64\ucrt64\bin",
    "C:\msys64\clang64\bin",
    "C:\msys64\usr\bin"
)

$MSYS_BIN = $null
foreach ($path in $possiblePaths) {
    if (Test-Path "$path\libglib-2.0-0.dll") {
        $MSYS_BIN = $path
        Write-Host "Found MSYS2 binaries at: $MSYS_BIN" -ForegroundColor Cyan
        break
    }
}

if ($null -eq $MSYS_BIN) {
    Write-Host "Error: Could not find MSYS2 installation!" -ForegroundColor Red
    Write-Host "Please ensure MSYS2 is installed at C:\msys64\" -ForegroundColor Yellow
    exit 1
}

# Create distribution directory (preserve existing files)
Write-Host "Preparing distribution directory..." -ForegroundColor Green
if (-not (Test-Path $DIST_DIR)) {
    New-Item -ItemType Directory -Path $DIST_DIR | Out-Null
    Write-Host "Created new dist directory" -ForegroundColor Cyan
} else {
    Write-Host "Using existing dist directory" -ForegroundColor Cyan
}

# Detect executable name
$exeName = $null
if (Test-Path "main.exe") {
    $exeName = "main.exe"
} elseif (Test-Path "astrogon.exe") {
    $exeName = "astrogon.exe"
} else {
    # Find any .exe in current directory
    $exeFiles = Get-ChildItem -Filter "*.exe" -File | Where-Object { $_.Name -notlike "*Setup*" -and $_.Name -notlike "*-Portable*" }
    if ($exeFiles.Count -gt 0) {
        $exeName = $exeFiles[0].Name
    }
}

if ($null -eq $exeName) {
    Write-Host "Error: No executable found!" -ForegroundColor Red
    Write-Host "Please build the application first using: make" -ForegroundColor Yellow
    exit 1
}

# Copy the executable
Write-Host "Copying executable: $exeName" -ForegroundColor Green
Copy-Item $exeName -Destination "$DIST_DIR\" -Force

# List of common GTK3 and dependency DLLs from MSYS2/MinGW
$requiredDLLs = @(
    # Core GLib libraries
    "libgio-2.0-0.dll",
    "libglib-2.0-0.dll",
    "libgobject-2.0-0.dll",
    "libgmodule-2.0-0.dll",
    
    # GTK3 libraries
    "libgtk-3-0.dll",
    "libgdk-3-0.dll",
    "libcairo-2.dll",
    "libcairo-gobject-2.dll",
    "libpango-1.0-0.dll",
    "libpangocairo-1.0-0.dll",
    "libpangowin32-1.0-0.dll",
    "libpangoft2-1.0-0.dll",
    "libatk-1.0-0.dll",
    "libgdk_pixbuf-2.0-0.dll",
    "libepoxy-0.dll",
    
    # Support libraries
    "libffi-8.dll",
    "libintl-8.dll",
    "libiconv-2.dll",
    "libpcre2-8-0.dll",
    "zlib1.dll",
    "libpng16-16.dll",
    "libharfbuzz-0.dll",
    "libfreetype-6.dll",
    "libfontconfig-1.dll",
    "libexpat-1.dll",
    "libbz2-1.dll",
    "libbrotlidec.dll",
    "libbrotlicommon.dll",
    "libgraphite2.dll",
    "libpixman-1-0.dll",
    "libfribidi-0.dll",
    "libdatrie-1.dll",
    "libthai-0.dll",
    
    # Image format libraries
    "libjpeg-8.dll",
    "libtiff-6.dll",
    "libwebp-7.dll",
    "libsharpyuv-0.dll",
    "libjbig-0.dll",
    "libLerc.dll",
    "libdeflate.dll",
    "liblzma-5.dll",
    
    # Application dependencies
    "libcurl-4.dll",
    "libcjson-1.dll",
    "libsqlite3-0.dll",
    
    # Runtime libraries
    "libwinpthread-1.dll",
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    
    # SSL/Crypto
    "libssl-3-x64.dll",
    "libcrypto-3-x64.dll",
    
    # UUID and network
    "libuuid-1.dll",
    "libssh2-1.dll",
    "libnghttp2-14.dll",
    "libnghttp3-9.dll",
    "libngtcp2-16.dll",
    "libngtcp2_crypto_ossl-0.dll",
    "libidn2-0.dll",
    "libunistring-5.dll",
    "libpsl-5.dll",
    "libzstd.dll"
)

$copiedCount = 0
$missingCount = 0
$missingDLLs = @()

Write-Host "`nCopying required DLLs from: $MSYS_BIN" -ForegroundColor Green
foreach ($dll in $requiredDLLs) {
    $sourcePath = Join-Path $MSYS_BIN $dll
    if (Test-Path $sourcePath) {
        Copy-Item $sourcePath -Destination "$DIST_DIR\" -Force -ErrorAction SilentlyContinue
        Write-Host "  ✓ Copied: $dll" -ForegroundColor Cyan
        $copiedCount++
    } else {
        Write-Host "  ✗ Missing: $dll" -ForegroundColor Yellow
        $missingCount++
        $missingDLLs += $dll
    }
}

# Copy GTK runtime files
Write-Host "`nCopying GTK runtime files..." -ForegroundColor Green

$GTK_SHARE = Join-Path (Split-Path $MSYS_BIN -Parent) "share"
$GTK_LIB = Join-Path (Split-Path $MSYS_BIN -Parent) "lib"

# Copy schemas
if (Test-Path "$GTK_SHARE\glib-2.0\schemas") {
    New-Item -ItemType Directory -Force -Path "$DIST_DIR\share\glib-2.0" | Out-Null
    Copy-Item "$GTK_SHARE\glib-2.0\schemas" -Destination "$DIST_DIR\share\glib-2.0\" -Recurse -Force
    Write-Host "  ✓ Copied: glib schemas" -ForegroundColor Cyan
}

# Copy icons (only Adwaita - the default theme)
if (Test-Path "$GTK_SHARE\icons\Adwaita") {
    New-Item -ItemType Directory -Force -Path "$DIST_DIR\share\icons" | Out-Null
    Copy-Item "$GTK_SHARE\icons\Adwaita" -Destination "$DIST_DIR\share\icons\" -Recurse -Force
    Write-Host "  ✓ Copied: Adwaita icons" -ForegroundColor Cyan
}

# Copy gdk-pixbuf loaders
if (Test-Path "$GTK_LIB\gdk-pixbuf-2.0") {
    New-Item -ItemType Directory -Force -Path "$DIST_DIR\lib" | Out-Null
    Copy-Item "$GTK_LIB\gdk-pixbuf-2.0" -Destination "$DIST_DIR\lib\" -Recurse -Force
    Write-Host "  ✓ Copied: gdk-pixbuf loaders" -ForegroundColor Cyan
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Summary" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Executable: $exeName" -ForegroundColor Cyan
Write-Host "DLLs copied: $copiedCount" -ForegroundColor Cyan
if ($missingCount -gt 0) {
    Write-Host "DLLs missing: $missingCount" -ForegroundColor Yellow
    Write-Host "`nMissing DLLs (may not be needed):" -ForegroundColor Yellow
    foreach ($dll in $missingDLLs) {
        Write-Host "  - $dll" -ForegroundColor Gray
    }
}

# Get total size
$totalSize = (Get-ChildItem -Path $DIST_DIR -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB

Write-Host "`nTotal size: $([math]::Round($totalSize, 2)) MB" -ForegroundColor Cyan
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "✅ Distribution bundle created successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "`nLocation: $DIST_DIR folder" -ForegroundColor Cyan
Write-Host "`nTo test locally:" -ForegroundColor Yellow
Write-Host "  cd $DIST_DIR" -ForegroundColor White
Write-Host "  .\$exeName" -ForegroundColor White
Write-Host "`nTo distribute:" -ForegroundColor Yellow
Write-Host "  1. Copy the entire '$DIST_DIR' folder to another computer" -ForegroundColor White
Write-Host "  2. Run $exeName from that folder" -ForegroundColor White
Write-Host "`nThe target computer does NOT need GTK, MSYS2, or any development tools!" -ForegroundColor Green
.\bundle-dlls.ps1