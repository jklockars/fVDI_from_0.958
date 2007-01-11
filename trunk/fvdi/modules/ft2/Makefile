#
# to build from the builds/freemint directory call
#
#  make
#
# Your programs source code should start with this
# (uncomment the parts you do not need to keep the program small):
# ---8<---
#define FT_USE_AUTOHINT // autohinter
#define FT_USE_RASTER   // monochrome rasterizer
#define FT_USE_SMOOTH   // anti-aliasing rasterizer
#define FT_USE_TT       // truetype font driver
#define FT_USE_T1       // type1 font driver
#define FT_USE_T42      // type42 font driver
#define FT_USE_T1CID    // cid-keyed type1 font driver
#define FT_USE_CFF      // opentype font driver
#define FT_USE_BDF      // bdf bitmap font driver
#define FT_USE_PCF      // pcf bitmap font driver
#define FT_USE_PFR      // pfr font driver
#define FT_USE_WINFNT   // windows .fnt|.fon bitmap font driver
#include "FT:src/base/ftinit.c"
# ---8<---
#
# link your programs with libft2.a and either ftsystem.o or ftsystempure.o
# (and either ftdebug.o or ftdebugpure.o if you enabled FT_DEBUG_LEVEL_ERROR or
# FT_DEBUG_LEVEL_TRACE in include/freetype/config/ftoption.h).

TARGET = libft2.a

all:	$(TARGET)

top_srcdir  = ../..
srcdir      = .
subdir      = ft2

include $(top_srcdir)/CONFIGVARS

ifneq ($(stdlib_srcdir),)
CFLAGS = -m68$(CPU) -DFT2_BUILD_LIBRARY $(OPTS) -I$(srcdir)/include $(FT2_DEBUG_OPTS) -I$(ft2_srcdir)/src -I$(ft2_srcdir)/include -I$(stdlib_srcdir)/include $(M68K_ATARI_MINT_CFLAGS)
else
CFLAGS = -m68$(CPU) -DFT2_BUILD_LIBRARY $(OPTS) -I$(srcdir)/include -I$(top_srcdir)/modules/include $(FT2_DEBUG_OPTS) -I$(ft2_srcdir)/src -I$(ft2_srcdir)/include $(M68K_ATARI_MINT_CFLAGS)
endif

#
# FreeType2 library base
#
ftbase.o: $(ft2_srcdir)/src/base/ftbase.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftinit.o: $(ft2_srcdir)/src/base/ftinit.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftsystem.o: $(ft2_srcdir)/src/base/ftsystem.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftdebug.o: $(ft2_srcdir)/src/base/ftdebug.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library base extensions
#
ftglyph.o: $(ft2_srcdir)/src/base/ftglyph.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftbbox.o: $(ft2_srcdir)/src/base/ftbbox.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftmm.o: $(ft2_srcdir)/src/base/ftmm.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftsynth.o: $(ft2_srcdir)/src/base/ftsynth.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftbitmap.o: $(ft2_srcdir)/src/base/ftbitmap.c
	$(CC) -c $(CFLAGS) -o $@ $<

ftstroke.o: $(ft2_srcdir)/src/base/ftstroke.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library autohinting module
#
#autohint.o: $(ft2_srcdir)/src/autohint/autohint.c
#	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library autohinting module extensions
#
autofit.o: $(ft2_srcdir)/src/autofit/autofit.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library postscript hinting module
#
pshinter.o: $(ft2_srcdir)/src/pshinter/pshinter.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library PS support module
#
psaux.o: $(ft2_srcdir)/src/psaux/psaux.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library PS glyph names module
#
psnames.o: $(ft2_srcdir)/src/psnames/psnames.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library monochrome raster module
#
raster.o: $(ft2_srcdir)/src/raster/raster.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library anti-aliasing raster module
#
smooth.o: $(ft2_srcdir)/src/smooth/smooth.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library 'sfnt' module
#
sfnt.o: $(ft2_srcdir)/src/sfnt/sfnt.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library glyph and image caching system (still experimental)
#
ftcache.o: $(ft2_srcdir)/src/cache/ftcache.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library OpenType font driver
#
cff.o: $(ft2_srcdir)/src/cff/cff.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library TrueType font driver
#
truetype.o: $(ft2_srcdir)/src/truetype/truetype.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library Type1 font driver
#
type1.o: $(ft2_srcdir)/src/type1/type1.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library Type42 font driver
#
type42.o: $(ft2_srcdir)/src/type42/type42.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library CID-keyed Type1 font driver
#
type1cid.o: $(ft2_srcdir)/src/cid/type1cid.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library BDF bitmap font driver
#
bdf.o: $(ft2_srcdir)/src/bdf/bdf.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library PCF bitmap font driver
#
pcf.o: $(ft2_srcdir)/src/pcf/pcf.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library PFR font driver
#
pfr.o: $(ft2_srcdir)/src/pfr/pfr.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# FreeType2 library Windows FNT/FON bitmap font driver
#
winfnt.o: $(ft2_srcdir)/src/winfonts/winfnt.c
	$(CC) -c $(CFLAGS) -o $@ $<

BASEOBJ = ftinit.o ftbase.o ftglyph.o ftbbox.o ftmm.o ftsynth.o ftbitmap.o ftstroke.o

#AHINTOBJ = autohint.o autofit.o
AHINTOBJ = autofit.o

PSOBJ = psaux.o psnames.o pshinter.o

RASTEROBJ = raster.o smooth.o

FONTDOBJ = cff.o type1.o type42.o type1cid.o truetype.o bdf.o pfr.o winfnt.o
# pcf.o  - needs GZLib and ZLib

OBJS = $(BASEOBJ) $(AHINTOBJ) $(PSOBJ) $(RASTEROBJ) sfnt.o ftcache.o $(FONTDOBJ)

$(TARGET): $(OBJS) 
	$(AR) cr $@ $(OBJS); $(RANLIB) $@

clean:
	rm -rf $(OBJS)
	rm $(TARGET)

