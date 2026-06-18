# --- AUI Library Makefile (Root) ---
SRC_DIR := .
BUILD_MODE ?= debug

# Detect number of CPU cores
NPROCS := 1
OS := $(shell uname -s)
ifeq ($(OS),Linux)
  NPROCS := $(shell nproc)
endif
ifeq ($(OS),Darwin)
  NPROCS := $(shell sysctl -n hw.ncpu)
endif

# Enable multithreading
ifeq ($(filter -j%,$(MAKEFLAGS)),)
  MAKEFLAGS += -j$(NPROCS)
endif

# Directory logic: debug stays in Custom, others in Custom/mode
ifeq ($(BUILD_MODE), debug)
  LIB_DIR := Custom
else
  LIB_DIR := Custom/$(BUILD_MODE)
endif

OBJ_DIR := $(LIB_DIR)/obj

CXX := g++
AR  := ar

# Wayland Protocol Scanner config
WAYLAND_SCANNER   := wayland-scanner
PROTOCOLS_DIR     := $(shell pkg-config --variable=pkgdatadir wayland-protocols 2>/dev/null)
ifeq ($(PROTOCOLS_DIR),)
  PROTOCOLS_DIR   := /usr/share/wayland-protocols
endif

# XML Paths
XDG_SHELL_XML     := $(PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml
DECORATION_XML    := $(PROTOCOLS_DIR)/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
SCALE_XML         := $(PROTOCOLS_DIR)/staging/fractional-scale/fractional-scale-v1.xml

ifeq ($(wildcard $(XDG_SHELL_XML)),)
  $(error Error: $(XDG_SHELL_XML) not found. Install wayland-protocols package)
endif

# Generated files
GEN_SRCS          := $(OBJ_DIR)/xdg-shell-protocol.c \
                     $(OBJ_DIR)/xdg-decoration-protocol.c \
                     $(OBJ_DIR)/fractional-scale-protocol.c

GEN_HDRS          := $(OBJ_DIR)/xdg-shell-client-protocol.h \
                     $(OBJ_DIR)/xdg-decoration-unstable-v1-client-protocol.h \
                     $(OBJ_DIR)/fractional-scale-v1-client-protocol.h

GEN_OBJS          := $(patsubst $(OBJ_DIR)/%.c, $(OBJ_DIR)/%.o, $(GEN_SRCS))

# Strict compiler flags
BASE_FLAGS := -fmax-errors=25 -std=c++26 -pedantic -pedantic-errors -Wall -Wextra -Werror \
              -Wcast-align -Wdisabled-optimization -Wlogical-op \
              -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept \
              -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo \
              -Wstrict-null-sentinel -Wswitch-default -Wundef -Wfloat-equal \
              -Wconversion -Wsign-conversion -MMD -MP
              
GIT_VERSION := $(shell git describe --always --dirty --tags 2>/dev/null || echo "unknown")
BASE_FLAGS  += -DAUI_GIT_VERSION=\"$(GIT_VERSION)\"              
BASE_FLAGS  += -I$(OBJ_DIR)

# Optimization logic based on BUILD_MODE
ifeq ($(BUILD_MODE), release)
  CXXFLAGS := $(BASE_FLAGS) -O3 -ffunction-sections -fdata-sections -DFT2_BUILD_LIBRARY
else ifeq ($(BUILD_MODE), srelease)
  CXXFLAGS := $(BASE_FLAGS) -Os -ffunction-sections -fdata-sections -DFT2_BUILD_LIBRARY
else
  CXXFLAGS := $(BASE_FLAGS) -O0 -g3 -DBUILDING_MY_STATIC_LIB -DFT2_BUILD_LIBRARY
endif
CXXFLAGS += -I$(SRC_DIR)/freetype2/include -I$(SRC_DIR)/freetype2/include/freetype/config

# Source files and base objects
SRC_FILES  := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES  := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
OBJ_FILES  += $(GEN_OBJS)

# ------------------------------------------------------------------
# Font embedding – appended after base objects
# ------------------------------------------------------------------
FONT_FILE := $(SRC_DIR)/fonts/DejaVuSans-Bold.ttf 
FONT_OBJ  := $(OBJ_DIR)/font_data.o

ifneq ($(wildcard $(FONT_FILE)),)
  OBJ_FILES += $(FONT_OBJ)
  CXXFLAGS += -DAUI_EMBEDDED_FONT
else
  $(warning Font file $(FONT_FILE) not found – using system font fallback)
endif

LIB_NAME   := libAUI.a
STATIC_LIB := $(LIB_DIR)/$(LIB_NAME)

# --- Rules ---

all: $(STATIC_LIB)

release srelease:
	+@$(MAKE) BUILD_MODE=$@ all

# Convert font to object
$(FONT_OBJ): $(FONT_FILE) | $(OBJ_DIR)
	@echo "Converting font to object: $<"
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

# Archive the library
$(STATIC_LIB): $(OBJ_FILES)
	@echo "--- Archiving AUI Library [Mode: $(BUILD_MODE)] ---"
	$(AR) rcs $@ $(OBJ_FILES)

# Compile C++ objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling AUI [$(BUILD_MODE)]: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generate Wayland protocol sources and headers
$(OBJ_DIR)/xdg-shell-protocol.c: $(XDG_SHELL_XML) | $(OBJ_DIR)
	@echo "Generating Wayland protocol source: $@"
	$(WAYLAND_SCANNER) private-code $< $@
$(OBJ_DIR)/xdg-shell-client-protocol.h: $(XDG_SHELL_XML) | $(OBJ_DIR)
	@echo "Generating Wayland protocol header: $@"
	$(WAYLAND_SCANNER) client-header $< $@

$(OBJ_DIR)/xdg-decoration-protocol.c: $(DECORATION_XML) | $(OBJ_DIR)
	@echo "Generating Wayland protocol source: $@"
	$(WAYLAND_SCANNER) private-code $< $@
$(OBJ_DIR)/xdg-decoration-unstable-v1-client-protocol.h: $(DECORATION_XML) | $(OBJ_DIR)
	@echo "Generating Wayland protocol header: $@"
	$(WAYLAND_SCANNER) client-header $< $@

$(OBJ_DIR)/fractional-scale-protocol.c: $(SCALE_XML) | $(OBJ_DIR)
	@echo "Generating Wayland protocol source: $@"
	$(WAYLAND_SCANNER) private-code $< $@
$(OBJ_DIR)/fractional-scale-v1-client-protocol.h: $(SCALE_XML) | $(OBJ_DIR)
	@echo "Generating Wayland protocol header: $@"
	$(WAYLAND_SCANNER) client-header $< $@

# Dependencies to avoid race conditions
$(OBJ_DIR)/xdg-shell-protocol.o: $(OBJ_DIR)/xdg-shell-protocol.c $(OBJ_DIR)/xdg-shell-client-protocol.h
$(OBJ_DIR)/xdg-decoration-protocol.o: $(OBJ_DIR)/xdg-decoration-protocol.c $(OBJ_DIR)/xdg-decoration-unstable-v1-client-protocol.h
$(OBJ_DIR)/fractional-scale-protocol.o: $(OBJ_DIR)/fractional-scale-protocol.c $(OBJ_DIR)/fractional-scale-v1-client-protocol.h

# Compile generated C files (Wayland) with relaxed warnings
$(GEN_OBJS): $(OBJ_DIR)/%.o: $(OBJ_DIR)/%.c
	@echo "Compiling generated Wayland code [$(BUILD_MODE)]: $<"
	$(CXX) $(CXXFLAGS) -Wno-pedantic -Wno-cast-qual -Wno-attributes -c $< -o $@

# Create directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Include auto-generated dependencies
-include $(OBJ_FILES:.o=.d)

# Clean
clean:
	@echo "--- Cleaning AUI Library [$(BUILD_MODE)] ---"
	@if [ "$(BUILD_MODE)" = "debug" ]; then \
		rm -rf $(OBJ_DIR) $(STATIC_LIB) && rmdir $(LIB_DIR) 2>/dev/null || true; \
	else \
		rm -rf $(LIB_DIR); \
	fi

.PHONY: all clean release srelease

