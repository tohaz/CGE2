CXX = g++
CXXFLAGS = -fmax-errors=15 -std=c++26 -O2 -g3 \
           -DFT2_BUILD_LIBRARY \
           -DFT_CONFIG_OPTION_USE_PNG \
           -Iinclude \
           -Wno-conversion -Wno-sign-conversion -Wno-missing-declarations

OBJ_DIR = ../Custom/obj
OUTPUT_NODE = $(OBJ_DIR)/freetype_amalgamation.o

all: $(OUTPUT_NODE)

$(OUTPUT_NODE):
	@mkdir -p $(OBJ_DIR)
	# Base modules
	
	
	
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftbdf.c -o $(OBJ_DIR)/ftbdf.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afindic.c -o $(OBJ_DIR)/afindic.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/ft-hb.c -o $(OBJ_DIR)/ft-hb.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afadjust.c -o $(OBJ_DIR)/afadjust.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afblue.c -o $(OBJ_DIR)/afblue.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afcjk.c -o $(OBJ_DIR)/afcjk.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/aflatin.c -o $(OBJ_DIR)/aflatin.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afdummy.c -o $(OBJ_DIR)/afdummy.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/psnames/psmodule.c -o $(OBJ_DIR)/psmodule.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/pshinter/pshalgo.c -o $(OBJ_DIR)/pshalgo.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/pshinter/pshrec.c -o $(OBJ_DIR)/pshrec.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/pshinter/pshglob.c -o $(OBJ_DIR)/pshglob.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/pshinter/pshmod.c -o $(OBJ_DIR)/pshmod.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afshaper.c -o $(OBJ_DIR)/afshaper.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afranges.c -o $(OBJ_DIR)/afranges.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afloader.c -o $(OBJ_DIR)/afloader.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afhints.c -o $(OBJ_DIR)/afhints.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afglobal.c -o $(OBJ_DIR)/afglobal.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/autofit/afmodule.c -o $(OBJ_DIR)/afmodule.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/raster/raster.c -o $(OBJ_DIR)/raster.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/cff/cff.c -o $(OBJ_DIR)/cff.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/gzip/ftgzip.c -o $(OBJ_DIR)/ftgzip.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftinit.c -o $(OBJ_DIR)/ftinit.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftbase.c -o $(OBJ_DIR)/ftbase.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftbbox.c -o $(OBJ_DIR)/ftbbox.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftsystem.c -o $(OBJ_DIR)/ftsystem.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftbitmap.c -o $(OBJ_DIR)/ftbitmap.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftdebug.c -o $(OBJ_DIR)/ftdebug.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftmm.c -o $(OBJ_DIR)/ftmm.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/base/ftglyph.c -o $(OBJ_DIR)/ftglyph.o
	# TrueType, SFNT, smooth
	$(CXX) $(CXXFLAGS) -xc++ -c src/truetype/truetype.c -o $(OBJ_DIR)/truetype.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/sfnt/sfnt.c -o $(OBJ_DIR)/sfnt.o
	$(CXX) $(CXXFLAGS) -xc++ -c src/sfnt/pngshim.c -o $(OBJ_DIR)/pngshim.o   # <--- PNG support
	$(CXX) $(CXXFLAGS) -xc++ -c src/smooth/smooth.c -o $(OBJ_DIR)/smooth.o
	# Cache module
	$(CXX) $(CXXFLAGS) -xc++ -c src/cache/ftcache.c -o $(OBJ_DIR)/ftcache.o

	# Link everything (including pngshim.o)
	ld -r $(OBJ_DIR)/ftinit.o $(OBJ_DIR)/cff.o \
	      $(OBJ_DIR)/ftgzip.o $(OBJ_DIR)/ftbase.o \
	      $(OBJ_DIR)/ftbbox.o $(OBJ_DIR)/ftsystem.o \
	      $(OBJ_DIR)/ftbitmap.o $(OBJ_DIR)/ftdebug.o \
	      $(OBJ_DIR)/ftmm.o $(OBJ_DIR)/ftglyph.o \
	      $(OBJ_DIR)/truetype.o $(OBJ_DIR)/sfnt.o \
	      $(OBJ_DIR)/raster.o \
	      $(OBJ_DIR)/afmodule.o \
	      $(OBJ_DIR)/afglobal.o \
	      $(OBJ_DIR)/afhints.o \
	      $(OBJ_DIR)/afloader.o \
	      $(OBJ_DIR)/afranges.o \
	      $(OBJ_DIR)/afshaper.o \
	      $(OBJ_DIR)/pshmod.o \
	      $(OBJ_DIR)/pshrec.o \
	      $(OBJ_DIR)/pshglob.o \
	      $(OBJ_DIR)/pshalgo.o \
	      $(OBJ_DIR)/psmodule.o \
	      $(OBJ_DIR)/afdummy.o \
	      $(OBJ_DIR)/aflatin.o \
	      $(OBJ_DIR)/afcjk.o \
	      $(OBJ_DIR)/afblue.o \
	      $(OBJ_DIR)/afadjust.o \
	      $(OBJ_DIR)/ft-hb.o \
	      $(OBJ_DIR)/afindic.o \
	      $(OBJ_DIR)/pngshim.o \
	      $(OBJ_DIR)/smooth.o \
	      $(OBJ_DIR)/ftbdf.o \
	      $(OBJ_DIR)/ftcache.o -o $@

	# Cleanup
	@rm $(OBJ_DIR)/ftinit.o $(OBJ_DIR)/cff.o  $(OBJ_DIR)/ftgzip.o $(OBJ_DIR)/ftbase.o \
	    $(OBJ_DIR)/ftbbox.o $(OBJ_DIR)/ftsystem.o \
	    $(OBJ_DIR)/ftbitmap.o $(OBJ_DIR)/ftdebug.o \
	    $(OBJ_DIR)/ftmm.o $(OBJ_DIR)/ftglyph.o \
	    $(OBJ_DIR)/truetype.o $(OBJ_DIR)/sfnt.o \
	    $(OBJ_DIR)/pngshim.o \
	    $(OBJ_DIR)/smooth.o \
	    $(OBJ_DIR)/raster.o \
	    $(OBJ_DIR)/afmodule.o \
	    $(OBJ_DIR)/ftcache.o