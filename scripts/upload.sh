BIN_NAME=libPenMods.so
BUILD_TYPE=debug
BUILD_PATH=../build/linux/arm64-v8a/$BUILD_TYPE/$BIN_NAME
REMOTE_PATH=/userdata/PenMods/$BIN_NAME
adb shell rm "$REMOTE_PATH"
sleep 1s
adb push "$BUILD_PATH" "$REMOTE_PATH"
