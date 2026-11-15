# Convenience wrapper around CMake.
#   make          -> same as make build
#   make build    -> configure (if needed) and compile C++ + shaders
#   make run      -> build, then run from this folder (shader paths are relative to it)
#   make clean    -> delete the build folder
#   make rebuild  -> clean + build
#   make reset    -> forget the cached GPU choice (asks again on next run)
#   make handin   -> zip the whole project except build output for ILIAS (Assignment1.zip)

BUILD_DIR  ?= build
BUILD_TYPE ?= Release
TARGET     := $(BUILD_DIR)/Assignment1

PROJECT        := $(notdir $(CURDIR))
HANDIN_ZIP     := $(PROJECT).zip
HANDIN_EXCLUDE := '$(PROJECT)/$(BUILD_DIR)/*' '*.zip' '*.spv' '*.o' \
                  '$(PROJECT)/Makefile' '*device_selection_cache' '*.DS_Store' '*.rdc'

.PHONY: all build run clean rebuild reset handin

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

handin:
	rm -f $(HANDIN_ZIP)
	cd .. && zip -rX $(PROJECT)/$(HANDIN_ZIP) $(PROJECT) -x $(HANDIN_EXCLUDE)
	@echo
	@unzip -l $(HANDIN_ZIP)
