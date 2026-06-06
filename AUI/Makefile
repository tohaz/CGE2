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
  $(error Ошибка: Файл $(XDG_SHELL_XML) не найден. Установите пакет wayland-protocols)
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
BASE_FLAGS += -I$(OBJ_DIR)
              

# Optimization logic based on BUILD_MODE
ifeq ($(BUILD_MODE), release)
  CXXFLAGS := $(BASE_FLAGS) -O3 -ffunction-sections -fdata-sections -DFT2_BUILD_LIBRARY
else ifeq ($(BUILD_MODE), srelease)
  CXXFLAGS := $(BASE_FLAGS) -Os -ffunction-sections -fdata-sections -DFT2_BUILD_LIBRARY
else
  CXXFLAGS := $(BASE_FLAGS) -O0 -g3 -DBUILDING_MY_STATIC_LIB -DFT2_BUILD_LIBRARY
endif
CXXFLAGS += -I$(SRC_DIR)/freetype2/include -I$(SRC_DIR)/freetype2/include/freetype/config

SRC_FILES  := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES  := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
# Add wayland generated object to the final library list
OBJ_FILES  += $(GEN_OBJS)

LIB_NAME   := libAUI.a
STATIC_LIB := $(LIB_DIR)/$(LIB_NAME)

# --- Rules ---

# Default target: The library is the main goal
all: $(STATIC_LIB)

# Mode switching targets
release srelease:
	+@$(MAKE) BUILD_MODE=$@ all

# Archive the library: All OBJ_FILES must be finished first
$(STATIC_LIB): $(OBJ_FILES)
	@echo "--- Archiving AUI Library [Mode: $(BUILD_MODE)] ---"
	$(AR) rcs $@ $(OBJ_FILES)

# Compile objects:
# Uses order-only dependency '| $(OBJ_DIR)' to ensure directory exists 
# BEFORE compilation starts, without relying on timestamps.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling AUI [$(BUILD_MODE)]: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Specific triggers to match target names with their XML sources
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

# Precise dependencies mapping to prevent race condition during parallel build
$(OBJ_DIR)/xdg-shell-protocol.o: $(OBJ_DIR)/xdg-shell-protocol.c $(OBJ_DIR)/xdg-shell-client-protocol.h
$(OBJ_DIR)/xdg-decoration-protocol.o: $(OBJ_DIR)/xdg-decoration-protocol.c $(OBJ_DIR)/xdg-decoration-unstable-v1-client-protocol.h
$(OBJ_DIR)/fractional-scale-protocol.o: $(OBJ_DIR)/fractional-scale-protocol.c $(OBJ_DIR)/fractional-scale-v1-client-protocol.h

# Static pattern rule to compile all generated C files safely
$(GEN_OBJS): $(OBJ_DIR)/%.o: $(OBJ_DIR)/%.c
	@echo "Compiling generated Wayland code [$(BUILD_MODE)]: $<"
	$(CXX) $(CXXFLAGS) -Wno-pedantic -Wno-cast-qual -Wno-attributes -c $< -o $@

# Create mode-specific directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Include auto-generated dependencies
-include $(OBJ_FILES:.o=.d)

# Clean only the current mode and remove empty Custom directory in debug mode
clean:
	@echo "--- Cleaning AUI Library [$(BUILD_MODE)] ---"
	@if [ "$(BUILD_MODE)" = "debug" ]; then \
		rm -rf $(OBJ_DIR) $(STATIC_LIB) && rmdir $(LIB_DIR) 2>/dev/null || true; \
	else \
		rm -rf $(LIB_DIR); \
	fi

.PHONY: all clean release srelease

