# PowerShell script to bundle all required DLLs with the executable for Windows distribution

$DIST_DIR = "dist"
$MSYS_BIN = "C:\msys64\mingw64\bin"

# Create distribution directory
Write-Host "Creating distribution directory..." -ForegroundColor Green
New-Item -ItemType Directory -Force -Path $DIST_DIR | Out-Null

# Copy the executable
Write-Host "Copying executable..." -ForegroundColor Green
Copy-Item "main.exe" -Destination "$DIST_DIR\" -Force

# List of common GTK3 and dependency DLLs from MSYS2/MinGW
$requiredDLLs = @(
    "libgio-2.0-0.dll",
    "libglib-2.0-0.dll",
    "libgobject-2.0-0.dll",
    "libgmodule-2.0-0.dll",
    "libgtk-3-0.dll",
    "libgdk-3-0.dll",
    "libcairo-2.dll",
    "libcairo-gobject-2.dll",
    "libpango-1.0-0.dll",
    "libpangocairo-1.0-0.dll",
    "libpangowin32-1.0-0.dll",
    "libatk-1.0-0.dll",
    "libgdk_pixbuf-2.0-0.dll",
    "libepoxy-0.dll",
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
    "libcurl-4.dll",
    "libcjson-1.dll",
    "libsqlite3-0.dll",
    "libwinpthread-1.dll",
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libssl-3-x64.dll",
    "libcrypto-3-x64.dll",
    "libuuid-1.dll",
    "libssh2-1.dll",
    "libnghttp2-14.dll",
    "libidn2-0.dll",
    "libunistring-5.dll",
    "libpsl-5.dll",
    "libzstd.dll"
)

Write-Host "Copying required DLLs..." -ForegroundColor Green
foreach ($dll in $requiredDLLs) {
    $sourcePath = Join-Path $MSYS_BIN $dll
    if (Test-Path $sourcePath) {
        Copy-Item $sourcePath -Destination "$DIST_DIR\" -Force
        Write-Host "  Copied: $dll" -ForegroundColor Cyan
    } else {
        Write-Host "  Warning: $dll not found" -ForegroundColor Yellow
    }
}

# Copy GTK runtime files
Write-Host "`nCopying GTK runtime files..." -ForegroundColor Green

$GTK_SHARE = "C:\msys64\mingw64\share"
$GTK_LIB = "C:\msys64\mingw64\lib"

# Copy schemas
if (Test-Path "$GTK_SHARE\glib-2.0\schemas") {
    New-Item -ItemType Directory -Force -Path "$DIST_DIR\share\glib-2.0" | Out-Null
    Copy-Item "$GTK_SHARE\glib-2.0\schemas" -Destination "$DIST_DIR\share\glib-2.0\" -Recurse -Force
    Write-Host "  Copied: glib schemas" -ForegroundColor Cyan
}

# Copy icons
if (Test-Path "$GTK_SHARE\icons") {
    New-Item -ItemType Directory -Force -Path "$DIST_DIR\share" | Out-Null
    Copy-Item "$GTK_SHARE\icons" -Destination "$DIST_DIR\share\" -Recurse -Force
    Write-Host "  Copied: icons" -ForegroundColor Cyan
}

# Copy gdk-pixbuf loaders
if (Test-Path "$GTK_LIB\gdk-pixbuf-2.0") {
    New-Item -ItemType Directory -Force -Path "$DIST_DIR\lib" | Out-Null
    Copy-Item "$GTK_LIB\gdk-pixbuf-2.0" -Destination "$DIST_DIR\lib\" -Recurse -Force
    Write-Host "  Copied: gdk-pixbuf loaders" -ForegroundColor Cyan
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Distribution bundle created successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "`nLocation: $DIST_DIR folder" -ForegroundColor Cyan
Write-Host "`nTo distribute:" -ForegroundColor Yellow
Write-Host "  1. Copy the entire '$DIST_DIR' folder to another computer" -ForegroundColor White
Write-Host "  2. Navigate to the folder and run main.exe" -ForegroundColor White
Write-Host "`nNote: The target computer does NOT need GTK or MSYS2 installed!" -ForegroundColor Yellow
