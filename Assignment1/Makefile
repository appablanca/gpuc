# Convenience wrapper around CMake.
#   make          -> same as make build
#   make build    -> configure (if needed) and compile C++ + shaders
#   make run      -> build, then run from this folder (shader paths are relative to it)
#   make clean    -> delete the build folder
#   make rebuild  -> clean + build
#   make reset    -> forget the cached GPU choice (asks again on next run)

BUILD_DIR  ?= build
BUILD_TYPE ?= Release
TARGET     := $(BUILD_DIR)/Assignment1

.PHONY: all build run clean rebuild reset

all: build

$(BUILD_DIR)/CMakeCache.txt: CMakeLists.txt
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) -j

run: build
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build

reset:
	rm -f device_selection_cache
