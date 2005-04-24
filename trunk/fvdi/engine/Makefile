#!make -f
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#

#
# gcc >= 2.95.3 (sparemint) version
#

TARGET = fvdi_gnu.prg

all: $(TARGET)

top_srcdir = ..

include $(top_srcdir)/CONFIGVARS

CFLAGS  = -mshort -m68040 -O2 -fomit-frame-pointer -I$(top_srcdir)/include $(M68K_ATARI_MINT_CFLAGS)
LDFLAGS = $(M68K_ATARI_MINT_LDFLAGS)
ASFLAGS = -m68040 --defsym gas=1 --defsym lattice=0 -I$(top_srcdir)/include -I$(top_srcdir)/drivers/include -I$(top_srcdir)/drivers/1_plane

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
	vdi_misc.s

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

# handle DevPac -> gas conversions
# defines SSRC_GNU and SINCSRC_GNU variables
include $(top_srcdir)/utility/conv2gas/RULES

# need to convert the .inc file as well
$(SSRC_GNU): $(SINCSRC_GNU)

# engine binary objects
OBJECTS = $(SSRC_GNU:.gnu=.gnu.o) $(CSRC:.c=.o)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS)

strip:
	$(STRIP) --strip-all $(TARGET)

clean: clean_gnu
	rm -rf $(OBJECTS)
