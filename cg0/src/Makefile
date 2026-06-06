# 
# Additional make targets:
# 1. Release optimized by speed (make release) 
# 2. Release optimized by size (make srelease)
#

# --- Universal Makefile for Examples/Tests (Sub-directory) ---
# This Makefile compiles all .cpp files in the directory into a single binary.
AUI_ROOT   := ../../AUI
BUILD_MODE ?= debug
TARGET_NAME := cg0

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
# Using += allows users to override this from the command line if needed
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

AUI_LIB  := $(AUI_LIB_DIR)/libAUI.a
OBJ_DIR  := $(BUILD_DIR)/obj
DEP_DIR  := $(BUILD_DIR)/deps
SRC_DIR  := .
TARGET   := $(BUILD_DIR)/$(TARGET_NAME)

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

CPPFLAGS := -I$(AUI_ROOT)
LIBS     := -lAUI -lX11 -lX11-xcb -lxcb -lxcb-cursor -lXrender -lwayland-client

# Source and Object discovery
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

# --- Rules ---

# Main target: Build the single binary
all: $(TARGET)

# Helper targets for easy switching from command line
# These allow running 'make release' or 'make srelease' directly
release srelease:
	@$(MAKE) BUILD_MODE=$@ all

# Linking rule: Builds the final binary from all object files.
# It depends on both local objects AND the library-checking rule (build_aui_lib).
$(TARGET): $(OBJS) build_aui_lib | $(BUILD_DIR)
	@echo "Linking binary [$(BUILD_MODE)]: $@"
	@rm -f $@
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS) || (rm -f $@ && exit 1)
ifneq ($(BUILD_MODE), debug)
	@$(STRIP) --strip-all $@
endif

# Recursive call to the root Makefile to ensure libAUI.a is up to date.
# We mark it .PHONY to force a check of the library source files every time.
build_aui_lib:
	@echo "--- Ensuring AUI Library [Mode: $(BUILD_MODE)] ---"
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) all

# Compilation rule: Compile each .cpp into an .o file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) $(DEP_DIR)
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d -c $< -o $@

# Ensure build directories exist before compilation starts
$(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR):
	@mkdir -p $@

# Include automatically generated dependency files (.d)
-include $(DEPS)

# Clean only the current mode
clean:
	@echo "--- Cleaning Sub-project [$(BUILD_MODE)] ---"
	rm -rf $(BUILD_DIR)
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) clean

.PHONY: all clean release srelease build_aui_lib
