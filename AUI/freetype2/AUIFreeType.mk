CXX = g++
CXXFLAGS = -fmax-errors=15 -std=c++26 -O2 -g3 \
           -DFT2_BUILD_LIBRARY \
           -DFT_CONFIG_OPTION_USE_PNG \
           -Iinclude \
           -Wno-conversion -Wno-sign-conversion -Wno-missing-declarations

OBJ_DIR = ../Custom/obj
OUTPUT_NODE = $(OBJ_DIR)/freetype_amalgamation.o

# ==============================================================================
# EXACT PATH CONFIGURATION
# Format: object_name.o|path/to/source.c
# ==============================================================================
FILE_MAP = \
    ftbdf.o|src/base/ftbdf.c \
    afindic.o|src/autofit/afindic.c \
    ft-hb.o|src/autofit/ft-hb.c \
    afadjust.o|src/autofit/afadjust.c \
    afblue.o|src/autofit/afblue.c \
    afcjk.o|src/autofit/afcjk.c \
    aflatin.o|src/autofit/aflatin.c \
    afdummy.o|src/autofit/afdummy.c \
    psmodule.o|src/psnames/psmodule.c \
    pshalgo.o|src/pshinter/pshalgo.c \
    pshrec.o|src/pshinter/pshrec.c \
    pshglob.o|src/pshinter/pshglob.c \
    pshmod.o|src/pshinter/pshmod.c \
    afshaper.o|src/autofit/afshaper.c \
    afranges.o|src/autofit/afranges.c \
    afloader.o|src/autofit/afloader.c \
    afhints.o|src/autofit/afhints.c \
    afglobal.o|src/autofit/afglobal.c \
    afmodule.o|src/autofit/afmodule.c \
    raster.o|src/raster/raster.c \
    cff.o|src/cff/cff.c \
    ftgzip.o|src/gzip/ftgzip.c \
    ftinit.o|src/base/ftinit.c \
    ftbase.o|src/base/ftbase.c \
    ftbbox.o|src/base/ftbbox.c \
    ftsystem.o|src/base/ftsystem.c \
    ftbitmap.o|src/base/ftbitmap.c \
    ftdebug.o|src/base/ftdebug.c \
    ftmm.o|src/base/ftmm.c \
    ftglyph.o|src/base/ftglyph.c \
    truetype.o|src/truetype/truetype.c \
    sfnt.o|src/sfnt/sfnt.c \
    pngshim.o|src/sfnt/pngshim.c \
    smooth.o|src/smooth/smooth.c \
    ftcache.o|src/cache/ftcache.c

# ==============================================================================
# BUILD LOGIC (Do not modify below this line)
# ==============================================================================

# Extract all target objects with the directory prefix
OBJS = $(foreach item,$(FILE_MAP),$(OBJ_DIR)/$(word 1,$(subst |, ,$(item))))

.PHONY: all clean

all: $(OUTPUT_NODE)

# Link the final combined node
$(OUTPUT_NODE): $(OBJS)
	@mkdir -p $(@D)
	ld -r $(OBJS) -o $@

# Metaprogramming loop: Dynamically generates explicit rules for each map entry
define GENERATE_RULE
OBJ_NAME := $$(word 1,$$(subst |, ,$(1)))
SRC_PATH := $$(word 2,$$(subst |, ,$(1)))

$$(OBJ_DIR)/$$(OBJ_NAME): $$(SRC_PATH)
	@mkdir -p $$(@D)
	$$(CXX) $$(CXXFLAGS) -xc++ -c $$< -o $$@
endef

# Evaluate the rules for every pair in the map
$(foreach item,$(FILE_MAP),$(eval $(call GENERATE_RULE,$(item))))

clean:
	rm -rf $(OBJ_DIR)
	
	