#!/bin/bash

# 3D Game Development Build Script for macOS M1 Pro

set -e  # Exit on any error

echo "🎮 3D Game Development Build Script"
echo "===================================="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Please run this script from the project root directory"
    exit 1
fi

# Create build directory
echo "📁 Creating build directory..."
mkdir -p build
cd build

# Build the project
echo "🔨 Building project..."
cmake ..
make

echo ""
echo "✅ Build completed successfully!"
echo ""
echo "please go to directory bin/0.my_work to run program"
WORKLIST="../src/0.my_work/worklist.txt"

if [ -f "$WORKLIST" ]; then
  echo "📄 Worklist"
  # nl -ba -w2 -s'. ' "$WORKLIST"
  cat "$WORKLIST"
else
  echo "⚠️ Worklist file not found at $WORKLIST"
fi