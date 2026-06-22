# 
# Additional make targets:
# 1. Release optimized by speed (make release) 
# 2. Release optimized by size (make srelease)
#

# --- Universal Makefile for Examples/Tests (Sub-directory) ---
# This version builds EACH .cpp file into its OWN separate binary.
AUI_ROOT   := ..
BUILD_MODE ?= debug

# Detect number of CPU cores for parallel build
NPROCS := 1
OS := $(shell uname -s)
ifeq ($(OS),Linux)
  NPROCS := $(shell nproc)
endif
ifeq ($(OS),Darwin)
  NPROCS := $(shell sysctl -n hw.ncpu)
endif

# Enable multithreading by default
MAKEFLAGS += -j$(NPROCS)

# Automatically delete target if a rule fails to prevent corrupted binaries
.DELETE_ON_ERROR:

# Define library and build directory paths based on BUILD_MODE
# Synchronize library path logic with AUI root Makefile
ifeq ($(BUILD_MODE), debug)
  AUI_LIB_DIR := $(AUI_ROOT)/Custom
  BUILD_DIR   := bin
else
  AUI_LIB_DIR := $(AUI_ROOT)/Custom/$(BUILD_MODE)
  BUILD_DIR   := bin/$(BUILD_MODE)
endif

AUI_LIB := $(AUI_LIB_DIR)/libAUI.a
DEP_DIR := $(BUILD_DIR)/deps
SRC_DIR := .

# FIX: Always use the freetype_amalgamation.o from the main Custom/obj directory,
# because it is built only once and not copied to mode‑specific subfolders.
FREETYPE_OBJ := $(AUI_ROOT)/Custom/obj/freetype_amalgamation.o

CXX      := g++
STRIP    := strip

# Compilation flags
BASE_FLAGS := -std=c++26 -MMD -pedantic -pedantic-errors -Wall -Wextra -Werror \
              -Wcast-align -Wdisabled-optimization -Wlogical-op -Wmissing-include-dirs \
              -Wfloat-equal -Wconversion -Wsign-conversion

# Optimization logic based on BUILD_MODE
ifeq ($(BUILD_MODE), release)
  CXXFLAGS := $(BASE_FLAGS) -O3
  LDFLAGS  := -L$(AUI_LIB_DIR)
else ifeq ($(BUILD_MODE), srelease)
  CXXFLAGS := $(BASE_FLAGS) -Os -ffunction-sections -fdata-sections
  LDFLAGS  := -L$(AUI_LIB_DIR) -Wl,--gc-sections
else
  CXXFLAGS := $(BASE_FLAGS) -O0 -g3
  LDFLAGS  := -L$(AUI_LIB_DIR)
endif

# Include paths synchronized with the root Makefile to fix #include errors
CPPFLAGS := -I$(AUI_ROOT) \
            -I$(AUI_LIB_DIR)/obj \
            -I$(AUI_ROOT)/freetype2/include \
            -I$(AUI_ROOT)/freetype2/include/freetype/config

LIBS     := -lAUI -lX11 -lX11-xcb -lxcb -lxcb-cursor -lxcb-image -lXrender -lwayland-client -lxkbcommon -lxcb-keysyms -lxkbcommon-x11 -lwayland-cursor -lxcb-cursor -lpng -lz

# Source and Target discovery
# Every .cpp file becomes a separate executable in $(BUILD_DIR)
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
EXES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

# --- Rules ---

# Main target: Build all separate binaries
all: $(EXES)

# Helper targets for easy switching from command line
release srelease:
	@$(MAKE) BUILD_MODE=$@ all

# Force make to always look into the root directory to check if libAUI.a is up to date
$(AUI_LIB): build_aui_lib
	@:

# Recursive call to the root Makefile to ensure libAUI.a is up to date.
# Marked .PHONY to ensure the library is checked before any app is linked.
.PHONY: build_aui_lib
build_aui_lib:
	@echo "--- Checking AUI Library [Mode: $(BUILD_MODE)] ---"
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) all

# Executables now link against $(AUI_LIB) and the separate $(FREETYPE_OBJ) file
$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp $(AUI_LIB) | $(BUILD_DIR) $(DEP_DIR)
	@echo "Building application [$(BUILD_MODE)]: $<"
	@rm -f $@
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d $< -o $@ $(LDFLAGS) $(FREETYPE_OBJ) $(LIBS)
ifneq ($(BUILD_MODE), debug)
	@$(STRIP) --strip-all $@
endif

# Ensure build directories exist before compilation starts
$(BUILD_DIR) $(DEP_DIR):
	@mkdir -p $@

# Include automatically generated dependency files (.d)
-include $(DEPS)

# Clean only the current mode
clean:
	@echo "--- Cleaning Sub-project [$(BUILD_MODE)] ---"
	rm -rf $(BUILD_DIR)
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) clean

.PHONY: all clean release srelease

