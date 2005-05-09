#!make -f
#
# This software is licensed under the GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

#
# gcc >= 2.95.3 (sparemint) version
#

TARGET = fvdi_gnu.prg

all: $(TARGET)

top_srcdir = ..
srcdir     = .

include $(top_srcdir)/CONFIGVARS

ifneq ($(stdlib_srcdir),)
STDLIB_CFLAGS = -I$(stdlib_srcdir)/include $(M68K_ATARI_MINT_CFLAGS)
else
STDLIB_CFLAGS = -I$(top_srcdir)/modules/include $(M68K_ATARI_MINT_CFLAGS)
endif

CFLAGS  = -m68$(CPU) $(OPTS) -I$(top_srcdir)/include $(STDLIB_CFLAGS)
ASFLAGS = -m68$(CPU) -I$(top_srcdir)/include -I$(top_srcdir)/drivers/include -I$(top_srcdir)/drivers/1_plane

# FreeType2 things
ifneq ($(ft2_srcdir),)

ifneq ($(libkern_srcdir),)

LIBKERN = $(libkern_srcdir)/libkern$(CPU).a
#LIBS    = -lft2 -lkern$(CPU)
LIBS    = -lft2 -lkern020-60
LDFLAGS = -L$(top_srcdir)/modules/ft2 -L$(libkern_srcdir)
CFLAGS += -DUSE_LIBKERN
else
LIBKERN = 
LIBS    = -lft2
LDFLAGS = -L$(top_srcdir)/modules/ft2
endif

LIBFT2  = $(top_srcdir)/modules/ft2/libft2.a
LIBFILES= $(LIBFT2) $(LIBKERN)
CFLAGS += -DFT2 -I$(top_srcdir)/modules/ft2/include $(FT2_DEBUG_OPTS) -I$(ft2_srcdir)/include 

endif


SINCSRC = \
	$(top_srcdir)/include/compiler.inc \
	$(top_srcdir)/include/macros.inc \
	$(top_srcdir)/include/types.inc \
	$(top_srcdir)/include/vdi.inc \
	$(top_srcdir)/drivers/include/linea.inc \
	$(top_srcdir)/drivers/include/pixelmac.inc \
	$(top_srcdir)/drivers/1_plane/1_expand.inc

SSRC = \
	fvdi.s \
	blit.s \
	colours.s \
	draw.s \
	expand.s \
	fill_sq.s \
	gnu.s \
	line_sq.s \
	mark_sq.s \
	mouse.s \
	simple.s \
	support.s \
	sys_call.s \
	textrndr.s \
	text.s \
	text_sq.s \
	traptabl.s \
	unimpl.s \
	vdi_misc.s \
	setjmp.s


CSRC = \
	startup.c \
	utility.c \
	bezier.c \
	conic.c \
	escape.c \
	fonts.c \
	line.c \
	loader.c \
	math.c \
	patterns.c \
	polygon.c \
	setup.c \
	workstn.c

ifneq ($(ft2_srcdir),)
CSRC+= \
	$(top_srcdir)/modules/ft2/ft2.c \
	$(top_srcdir)/modules/ft2/ft2_ftsystem.c

ifeq ($(DEBUG),yes)
CSRC+= \
	../modules/ft2/ft2_ftdebug.c
endif
endif

# handle DevPac -> gas conversions
# defines SSRC_GNU and SINCSRC_GNU variables
include $(top_srcdir)/utility/conv2gas/RULES

# need to convert the .inc file as well
$(SSRC_GNU): $(SINCSRC_GNU)

# engine binary objects
OBJECTS = $(SSRC_GNU:.gnu=.gnu.o) $(CSRC:.c=.o)

# libkern + stdlib additions
ifneq ($(stdlib_srcdir),)
CLIBSRC = \
	$(stdlib_srcdir)/qsort.c \
	$(stdlib_srcdir)/msort.c \
	$(stdlib_srcdir)/mempcpy.c \
	$(stdlib_srcdir)/strdup.c

SLIBSRC = \
	$(stdlib_srcdir)/setjmp.S

OBJECTS+= $(CLIBSRC:.c=.o) $(SLIBSRC:.S=.S.o)
endif

%.S.o: %.S
	$(CC) $(ASFLAGS) -Dgas=1 -Dlattice=0 -o $@ -c $<

$(LIBKERN):
	cd $(libkern_srcdir); make 020-60

$(LIBFT2):
	cd $(top_srcdir)/modules/ft2; make

$(TARGET): $(OBJECTS) $(LIBFILES)
	$(LD) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

strip:
	$(STRIP) --strip-all $(TARGET)

clean: clean_gnu
	rm -rf $(OBJECTS)
