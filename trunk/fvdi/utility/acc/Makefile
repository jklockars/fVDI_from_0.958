#!make -f
#
# This software is licensed under the GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

#
# gcc >= 2.95.3 (sparemint) version
#

TARGET = fvdiagnu.acc

all: $(TARGET)

top_srcdir = ../..

include $(top_srcdir)/CONFIGVARS

#CFLAGS  = -m68$(CPU) -mshort $(OPTS) -I$(top_srcdir)/include $(M68K_ATARI_MINT_CFLAGS) -D__GEMLIB_OLDNAMES -DNEW_GEMLIB -Wall
CFLAGS  = -m68$(CPU) $(OPTS) -I$(top_srcdir)/include -D__GEMLIB_OLDNAMES -DNEW_GEMLIB -Wall

#LDFLAGS = $(M68K_ATARI_MINT_LDFLAGS)
LIBS += -lgem

CSRC = \
	fvdiacc.c \
	dloghndl.c \
	dlogtble.c \
	dodialog.c \
	objcedit.c \
	popup.c \
	wind.c

# engine binary objects
OBJECTS = $(SSRC_GNU:.gnu=.gnu.o) $(CSRC:.c=.o)

$(TARGET): depend $(OBJECTS) $(LIBFILES)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

depend: $(CSRC)
#	"$(CC) -MM" does not seem able to keep track of the directory of the
#	object file, so dependencies of files outside this directory are
#	never taken into account later. makedepend does this correctly.
#	Unfortunately, makedepend includes system headers in its
#	dependency lists, but you can't get everything...
#	$(CC) -MM $(CFLAGS) $(CSRC) > depend
	makedepend -f- -- $(CFLAGS) $(CSRC) > depend 2> /dev/null
	sed "s#\/usr[^ ]*\.h##g" < depend  > depend2
	sed "s#^.*: *\$$##"      < depend2 > depend
	rm depend2

strip:
	$(STRIP) --strip-all $(TARGET)

clean:
	rm -rf $(OBJECTS)
	rm depend

include depend
