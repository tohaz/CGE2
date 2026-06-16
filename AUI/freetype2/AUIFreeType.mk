CXX = g++
CXXFLAGS = -fmax-errors=15 -std=c++26 -O2 -g3 -DFT2_BUILD_LIBRARY -Iinclude \
           -Wno-conversion -Wno-sign-conversion -Wno-missing-declarations

OBJ_DIR = ../Custom/obj
OUTPUT_NODE = $(OBJ_DIR)/freetype_amalgamation.o

all: $(OUTPUT_NODE)

$(OUTPUT_NODE):
	@mkdir -p $(OBJ_DIR)
	# Base modules (including ftglyph.c)
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftinit.c -o $(OBJ_DIR)/ftinit.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftbase.c -o $(OBJ_DIR)/ftbase.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftbbox.c -o $(OBJ_DIR)/ftbbox.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftsystem.c -o $(OBJ_DIR)/ftsystem.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftbitmap.c -o $(OBJ_DIR)/ftbitmap.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftdebug.c -o $(OBJ_DIR)/ftdebug.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftmm.c -o $(OBJ_DIR)/ftmm.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftglyph.c -o $(OBJ_DIR)/ftglyph.o   # ← ADDED
	# TrueType, SFNT, smooth
	$(CXX) $(CXXFLAGS) -xc++ -c src/truetype/truetype.c -o $(OBJ_DIR)/truetype.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/sfnt/sfnt.c -o $(OBJ_DIR)/sfnt.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/smooth/smooth.c -o $(OBJ_DIR)/smooth.o
	# Cache module
	$(CXX) $(CXXFLAGS) -xc++ -c src/cache/ftcache.c -o $(OBJ_DIR)/ftcache.o

	# Link everything (including ftglyph.o)
	ld -r $(OBJ_DIR)/ftinit.o $(OBJ_DIR)/ftbase.o $(OBJ_DIR)/ftbbox.o $(OBJ_DIR)/ftsystem.o \
	      $(OBJ_DIR)/ftbitmap.o $(OBJ_DIR)/ftdebug.o $(OBJ_DIR)/ftmm.o $(OBJ_DIR)/ftglyph.o \
	      $(OBJ_DIR)/truetype.o $(OBJ_DIR)/sfnt.o $(OBJ_DIR)/smooth.o \
	      $(OBJ_DIR)/ftcache.o -o $@

	# Cleanup
	@rm $(OBJ_DIR)/ftinit.o $(OBJ_DIR)/ftbase.o $(OBJ_DIR)/ftbbox.o $(OBJ_DIR)/ftsystem.o \
	    $(OBJ_DIR)/ftbitmap.o $(OBJ_DIR)/ftdebug.o $(OBJ_DIR)/ftmm.o $(OBJ_DIR)/ftglyph.o \
	    $(OBJ_DIR)/truetype.o $(OBJ_DIR)/sfnt.o $(OBJ_DIR)/smooth.o \
	    $(OBJ_DIR)/ftcache.o
	    
	    