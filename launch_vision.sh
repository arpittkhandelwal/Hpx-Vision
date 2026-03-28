#!/bin/bash

# HPX-Vision 1-Click Launch Script
echo "🚀 Launching HPX-Vision Engine & Dashboard..."

DATA_DIR="dashboard/public"
BUILD_DIR="build"

# Ensure directories exist
mkdir -p "$DATA_DIR"
mkdir -p "$BUILD_DIR"

# 1. Compile the Simulation Engine
echo "📦 Compiling C++ Simulation Engine..."
g++ -O3 -std=c++20 \
    -I"./include" \
    -I"./mock_hpx" \
    "examples/vision_simulation.cpp" \
    -o "$BUILD_DIR/vision_simulation"

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

# 2. Start the Simulation in the background
echo "⚡ Starting C++ Engine (writing to $DATA_DIR/vision_data.json)..."
./"$BUILD_DIR/vision_simulation" &
SIM_PID=$!

# 3. Start the Dashboard (if not already running)
echo "🌐 Starting Dashbaord Dev Server..."
cd dashboard
npm run dev &
DASH_PID=$!

cd ..

echo "✅ HPX-Vision is now ACTIVE!"
echo "👉 Dashboard: http://localhost:5175/"
echo "👉 Data Export: dashboard/public/vision_data.json"
echo ""
echo "Press Ctrl+C to stop both processes."

# Wait for both to finish
trap "kill $SIM_PID $DASH_PID; exit" INT
wait $SIM_PID $DASH_PID
