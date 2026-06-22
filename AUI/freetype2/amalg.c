#define FT2_BUILD_LIBRARY

/* 1. Base Core & Memory Hooks */
#include "src/base/ftbase.c"
#include "src/base/ftinit.c"
#include "src/base/ftsystem.c"

/* 2. Core Layout Tables (Handles SFNT structure required by Emojis) */
#include "src/sfnt/sfnt.c"         // Internally includes ttcolr.c & ttcpal.c
#include "src/truetype/truetype.c"
#include "src/cff/cff.c"

/* 3. Essential Hinters and Utilities */
#include "src/autofit/autofit.c"
#include "src/pshinter/pshinter.c"
#include "src/psnames/psnames.c"

/* 4. Rasterizers & Bitmap Format Handlers */
#include "src/smooth/smooth.c"     // Handles anti-aliasing layers
#include "src/raster/raster.c"     // Normal fallback pipeline

/* 5. CRITICAL EMOJI ADDITIONS */
#include "src/base/ftbdf.c"        // Handles raw bitmap boundaries
#include "src/gzip/ftgzip.c" 

