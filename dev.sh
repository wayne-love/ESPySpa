#!/bin/bash
# Quick helper script for common development tasks

set -e

DEVICE_IP="${ESPA_IP:-10.0.0.198}"

# Function to find the ESP32 serial port
find_esp_port() {
    # Look for usbmodem (most common for ESP32-S3)
    local port=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
    if [ -z "$port" ]; then
        # Fall back to any USB serial device
        port=$(ls /dev/cu.usbserial* 2>/dev/null | head -1)
    fi
    echo "$port"
}

# Function to clean serial port
clean_serial() {
    # Kill any process using USB serial ports
    lsof -t /dev/cu.usb* 2>/dev/null | xargs kill -9 2>/dev/null || true
    sleep 0.5
}

# Function to ensure clean build (fixes LDF mode issues)
ensure_clean_build() {
    # Check if we need a clean build (first time or after errors)
    if [ ! -f ".pio/.dev-build-ok" ]; then
        echo "First dev build or previous errors - doing complete clean..."
        rm -rf .pio
        mkdir -p .pio
        touch .pio/.dev-build-ok
    fi
}

case "$1" in
  build)
    echo "Building dev environment..."
    ensure_clean_build
    pio run -e espa-v1
    ;;
    
  clean)
    echo "Cleaning build completely..."
    rm -rf .pio
    echo "Clean complete. Next build will reinstall all dependencies."
    ;;
    
  upload)
    echo "Building and uploading firmware..."
    ensure_clean_build
    clean_serial
    PORT=$(find_esp_port)
    if [ -z "$PORT" ]; then
        echo "ERROR: No ESP32 device found. Please connect your device."
        exit 1
    fi
    echo "Using port: $PORT"
    pio run -e espa-v1 --target upload --upload-port "$PORT"
    ;;
    
  monitor)
    echo "Opening serial monitor..."
    PORT=$(find_esp_port)
    if [ -z "$PORT" ]; then
        echo "ERROR: No ESP32 device found. Please connect your device."
        exit 1
    fi
    echo "Using port: $PORT"
    pio device monitor --port "$PORT"
    ;;
    
  uploadfs)
    echo "Building and uploading filesystem..."
    ensure_clean_build
    clean_serial
    PORT=$(find_esp_port)
    if [ -z "$PORT" ]; then
        echo "ERROR: No ESP32 device found. Please connect your device."
        exit 1
    fi
    echo "Using port: $PORT"
    pio run -e espa-v1 --target uploadfs --upload-port "$PORT"
    ;;
    
  full)
    echo "Build, upload firmware+filesystem, and monitor..."
    ensure_clean_build
    clean_serial
    PORT=$(find_esp_port)
    if [ -z "$PORT" ]; then
        echo "ERROR: No ESP32 device found. Please connect your device."
        exit 1
    fi
    echo "Using port: $PORT"
    echo ""
    echo "Step 1: Uploading firmware..."
    pio run -e espa-v1 --target upload --upload-port "$PORT"
    echo ""
    echo "Step 2: Uploading filesystem..."
    pio run -e espa-v1 --target uploadfs --upload-port "$PORT"
    echo ""
    echo "Upload complete! Starting serial monitor (Press Ctrl+C to exit)..."
    echo ""
    sleep 1
    pio device monitor --port "$PORT"
    ;;
    
  test)
    echo "Testing script functionality..."
    echo ""
    echo "✓ Script is executable"
    PORT=$(find_esp_port)
    if [ -z "$PORT" ]; then
        echo "✗ No ESP32 device found"
        exit 1
    else
        echo "✓ ESP32 found at: $PORT"
    fi
    if [ -f ".pio/.dev-build-ok" ]; then
        echo "✓ Build marker exists (incremental builds enabled)"
    else
        echo "⚠ No build marker (next build will be clean)"
    fi
    echo ""
    echo "Script is ready to use!"
    ;;
    
  unpair)
    echo "Unpairing device at $DEVICE_IP..."
    curl -X POST "http://$DEVICE_IP/api/espa-control/unpair"
    echo ""
    ;;
    
  info)
    echo "Device info at $DEVICE_IP:"
    echo ""
    echo "Device ID:"
    curl -s "http://$DEVICE_IP/api/espa-control/device-id" | jq
    echo ""
    echo "Config:"
    curl -s "http://$DEVICE_IP/api/espa-control/config" | jq
    ;;
    
  *)
    echo "eSpa Development Helper"
    echo ""
    echo "Usage: $0 {command}"
    echo ""
    echo "Commands:"
    echo "  build     - Build dev environment"
    echo "  clean     - Clean build cache (fixes WiFi.h errors)"
    echo "  upload    - Build and upload firmware"
    echo "  uploadfs  - Build and upload filesystem"
    echo "  monitor   - Open serial monitor (Ctrl+C to exit)"
    echo "  full      - Build, upload firmware+filesystem, and monitor"
    echo "  test      - Test script functionality"
    echo "  unpair    - Clear pairing token"
    echo "  info      - Show device info (ID, config)"
    echo ""
    echo "Environment:"
    echo "  ESPA_IP   - Device IP (default: espa.local)"
    echo "            Example: ESPA_IP=192.168.1.50 $0 info"
    exit 1
    ;;
esac
