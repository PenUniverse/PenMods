#!/bin/bash

WORKING_DIR=/home/xenoneko/Desktop/projects/PenMods
BUILD_ARCH=arm64-v8a
BUILD_PLAT=linux
VNC_PORT=5900
BUILD_MODE=$1
EMULATOR_DEVICE=$2
EMULATOR_PATH=$WORKING_DIR/../DictPen
MOD_PATH=$EMULATOR_PATH
BUILD_PATH=$WORKING_DIR/build/$BUILD_PLAT/$BUILD_ARCH/$BUILD_MODE

rm "$MOD_PATH/libPenMods.so"
cp "$BUILD_PATH/libPenMods.so" "$MOD_PATH/libPenMods.so"

if [ "$EMULATOR_DEVICE" = "YDP02X" ]; then
    EMULATOR_PATH=$EMULATOR_PATH/ydp02x_oem
else
    echo 'Unsupported device!'
    exit 1
fi

CURRENT_PATH=$(pwd)
cd "$EMULATOR_PATH"

bash run.sh &
RUN_PID=$!

cd $CURRENT_PATH
sleep 5s

remmina --no-tray-icon -c vnc://127.0.0.1:$VNC_PORT &
REMMINA_PID=$!

cleanup() {
    killall qemu-aarch64-static
    killall remmina
}

trap cleanup SIGINT

wait $RUN_PID
wait $REMMINA_PID
