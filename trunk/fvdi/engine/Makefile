#!/bin/sh

# This is obviously not yet an actual Makefile.
# For now, use:
# source Makefile
# or something equivalent.
#
# Look at the end for compiler version dependent things.

gcc -o ../utility/conv2gas/conv2gas ../utility/conv2gas/conv2gas.c

for file in ../include/*.inc ../drivers/include/*.inc ../drivers/1_plane/1_expand.inc *.s ; do
echo Converting $file $file.gnu;

# Comments with * in the first position on the line
sed "s%^\*.*%%" $file >$file.gnu
mv $file.gnu $file.tmp

# Comments after ; separator
sed "s%;.*$%%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Conditional compilation
sed "s%^\\([ \t]\+\\)ifne%\1.ifne%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ \t]\+\\)ifeq%\1.ifeq%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ \t]\+\\)else%\1.else%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ \t]\+\\)endc%\1.endif%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Constants
sed "s%^\\([^ \t]*\\)[ \t]\+equ[ \t]\+%\t.equiv\t\\1,%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([^ \t]*\\)[ \t]\+set[ \t]\+%\t.set\t\\1,%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Sections
sed "s%^\\([ \t]\+\\)text%\1.text%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ \t]\+\\)data%\1.data%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ \t]\+\\)bss%\1.bss%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Includes
sed "s%\.inc\"%.inc.gnu\"%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ \t]\+\\)include%\1.include%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Hexadecimal numbers
sed "s%\\$%0x%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Data
sed "s%\\([ \t]\+\\)dc.b%\1.byte%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ \t]\+\\)dc.w%\1.word%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ \t]\+\\)dc.l%\1.long%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Miscellaneous directives
sed "s%\.end\\([ \t]\+\|$\|\:\\)%.end___fix\1%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ \t]\+\\)end\\(\\([ \t]\+\\)\|$\\)%\1.end%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp


sed "s%^.loop1:$%1:%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^.loop2:$%2:%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^.loop2_end:$%3:%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%,.loop1$%,1b%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%,.loop2$%,2b%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%.loop2_end$%3f%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Things that would be hard do deal with using sed
../utility/conv2gas/conv2gas $file.tmp >$file.gnu

# Remove temporary file
rm $file.tmp

# Compile
#m68k-atari-mint-gcc -Wa,-m68040,--defsym,gas=1,--defsym,lattice=0,-I../include,-I../drivers/include,-I../drivers/1_plane,-alh=utfil -c <$file.gnu
#m68k-atari-mint-as -m68040 --defsym gas=1 --defsym lattice=0 -I../include -I../drivers/include -I../drivers/1_plane -alh=utfil <$file.gnu

done

rm purec.s.gnu

for file in *.s.gnu ; do
echo Assembling $file ;
m68k-atari-mint-as -m68040 --defsym gas=1 --defsym lattice=0 -I../include -I../drivers/include -I../drivers/1_plane -o $file.o <$file ;
done

m68k-atari-mint-gcc -mshort -m68040 -O2 -fomit-frame-pointer -c -I../include/ \
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
startup.c \
utility.c \
workstn.c


m68k-atari-mint-ld -o fvdi_gnu.prg -L/usr/lib/gcc-lib/m68k-atari-mint/2.95.3 \
fvdi.s.gnu.o \
blit.s.gnu.o \
colours.s.gnu.o \
draw.s.gnu.o \
expand.s.gnu.o \
fill_sq.s.gnu.o \
gnu.s.gnu.o \
line_sq.s.gnu.o \
mark_sq.s.gnu.o \
mouse.s.gnu.o \
simple.s.gnu.o \
support.s.gnu.o \
sys_call.s.gnu.o \
textrndr.s.gnu.o \
text.s.gnu.o \
text_sq.s.gnu.o \
traptabl.s.gnu.o \
unimpl.s.gnu.o \
vdi_misc.s.gnu.o \
startup.o \
utility.o \
bezier.o \
conic.o \
escape.o \
fonts.o \
line.o \
loader.o \
math.o \
patterns.o \
polygon.o \
setup.o \
workstn.o \
-lgcc

m68k-atari-mint-strip --strip-all fvdi_gnu.prg

for file in ../include/*.inc.gnu ../drivers/include/*.inc.gnu ../drivers/1_plane/1_expand.inc.gnu *.s.gnu ; do
rm $file;
done
