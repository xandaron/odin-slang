#!/bin/bash

# Build script for slangc wrapper (Linux/macOS)

set -e

# Default values
BUILD_TYPE="Release"
CLEAN=false
INSTALL=false
HELP=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -h|--help)
            HELP=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Show help
if [ "$HELP" = true ]; then
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -t, --type <Debug|Release>  Build configuration (default: Release)"
    echo "  -c, --clean                 Clean build directory"
    echo "  -i, --install               Install after building"
    echo "  -h, --help                  Show this help"
    echo ""
    echo "Examples:"
    echo "  $0                          # Build Release with CMake"
    echo "  $0 -t Debug                 # Build Debug"
    echo "  $0 -c -i                    # Clean, build, and install"
    exit 0
fi

# Set variables
BUILD_DIR="build"
BIN_DIR="bin"
SOURCE_FILE="slangc.cpp"
HEADER_FILE="slangc.h"
LIB_DIR="libs"
INCLUDE_DIR="include"

echo "Building slangc wrapper..."
echo "Build Type: $BUILD_TYPE"
echo "Using: CMake"

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directories..."
    rm -rf "$BUILD_DIR" "$BIN_DIR"
    echo "Clean complete."
    if [ "$INSTALL" = false ]; then
        exit 0
    fi
fi

# Check dependencies
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 not found. Please install $1."
        exit 1
    fi
}

check_command cmake
check_command ninja || check_command make

# Build with CMake
echo "Building with CMake..."

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Determine generator
GENERATOR=""
if command -v ninja &> /dev/null; then
    GENERATOR="-G Ninja"
fi

# Configure
cmake .. $GENERATOR -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build
cmake --build . --config "$BUILD_TYPE" --parallel

cd ..

# Copy binaries to bin directory
mkdir -p "$BIN_DIR"
find "$BUILD_DIR" -name "*.so" -o -name "*.a" -o -name "*.dylib" | while read -r file; do
    cp "$file" "$BIN_DIR/"
done

echo "Build completed successfully!"

# Install if requested
if [ "$INSTALL" = true ]; then
    echo "Installing..."
    
    # Determine install prefix
    if [ "$(uname)" = "Darwin" ]; then
        PREFIX="/usr/local"
    else
        PREFIX="/usr/local"
    fi
    
    # Check if we need sudo
    if [ ! -w "$PREFIX/lib" ] || [ ! -w "$PREFIX/include" ]; then
        SUDO="sudo"
    else
        SUDO=""
    fi
    
    # Install files
    $SUDO mkdir -p "$PREFIX/lib" "$PREFIX/include"
    
    # Install libraries
    find "$BIN_DIR" -name "*.so" -o -name "*.a" -o -name "*.dylib" | while read -r file; do
        $SUDO cp "$file" "$PREFIX/lib/"
    done
    
    # Install header
    $SUDO cp "$HEADER_FILE" "$PREFIX/include/"
    
    # Update library cache on Linux
    if [ "$(uname)" = "Linux" ] && command -v ldconfig &> /dev/null; then
        $SUDO ldconfig
    fi
    
    echo "Installation completed to: $PREFIX"
fi

echo "Done!"
