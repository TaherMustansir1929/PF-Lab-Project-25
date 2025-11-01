#!/bin/bash
# Script to bundle all required DLLs with the executable for Windows distribution

# Create distribution directory
DIST_DIR="dist"
mkdir -p "$DIST_DIR"

# Copy the executable
echo "Copying executable..."
cp main.exe "$DIST_DIR/"

# Function to recursively copy DLL dependencies
copy_dependencies() {
    local file="$1"
    local already_copied="$2"
    
    # Get list of DLLs this file depends on
    dependencies=$(ldd "$file" 2>/dev/null | grep -i "msys" | awk '{print $3}')
    
    for dll in $dependencies; do
        # Skip if already copied
        if [[ "$already_copied" == *"$(basename "$dll")"* ]]; then
            continue
        fi
        
        # Skip if DLL doesn't exist
        if [ ! -f "$dll" ]; then
            continue
        fi
        
        echo "Copying: $(basename "$dll")"
        cp "$dll" "$DIST_DIR/"
        already_copied="$already_copied $(basename "$dll")"
        
        # Recursively copy dependencies of this DLL
        copy_dependencies "$dll" "$already_copied"
    done
}

# Copy all dependencies
echo "Analyzing and copying DLL dependencies..."
copy_dependencies "main.exe" ""

# Copy GTK runtime files (themes, icons, etc.)
echo "Copying GTK runtime files..."
MINGW_PREFIX="/mingw64"
if [ -d "$MINGW_PREFIX/share/glib-2.0/schemas" ]; then
    mkdir -p "$DIST_DIR/share/glib-2.0"
    cp -r "$MINGW_PREFIX/share/glib-2.0/schemas" "$DIST_DIR/share/glib-2.0/"
fi

if [ -d "$MINGW_PREFIX/share/icons" ]; then
    mkdir -p "$DIST_DIR/share"
    cp -r "$MINGW_PREFIX/share/icons" "$DIST_DIR/share/"
fi

if [ -d "$MINGW_PREFIX/lib/gdk-pixbuf-2.0" ]; then
    mkdir -p "$DIST_DIR/lib"
    cp -r "$MINGW_PREFIX/lib/gdk-pixbuf-2.0" "$DIST_DIR/lib/"
fi

echo ""
echo "Distribution bundle created in '$DIST_DIR' directory"
echo "Copy the entire '$DIST_DIR' folder to another computer to run the application"
echo ""
echo "To run: Navigate to the dist folder and run main.exe"
