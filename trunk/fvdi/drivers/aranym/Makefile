#!/bin/sh

# This is obviously not yet an actual Makefile.
# For now, use:
# source Makefile
# or something equivalent.
#
# Look at the end for compiler version dependent things.

gcc -o ../../utility/conv2gas/conv2gas ../../utility/conv2gas/conv2gas.c

for file in ../../include/*.inc ../include/*.inc ../common/*.s; do
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
../../utility/conv2gas/conv2gas $file.tmp >$file.gnu

# Remove temporary file
rm $file.tmp

# Compile
#m68k-atari-mint-gcc -Wa,-m68040,--defsym,gas=1,--defsym,lattice=0,-I../include,-I../drivers/include,-I../drivers/1_plane,-alh=utfil -c <$file.gnu
#m68k-atari-mint-as -m68040 --defsym gas=1 --defsym lattice=0 -I../include -I../drivers/include -I../drivers/1_plane -alh=utfil <$file.gnu

done

rm ../common/purec.s.gnu

for file in ../common/*.s.gnu ; do
echo Assembling $file ;
m68k-atari-mint-as -m68040 --defsym gas=1 --defsym lattice=0 --defsym mc68000=0 -I../../include -I../include -o $file.o <$file ;
done

m68k-atari-mint-gcc -mshort -m68040 -O2 -fomit-frame-pointer -c -I../../include/ -I../include \
../common/init.c \
../common/colours.c \
spec.c \
8b_pal.c \
16b_pal.c \
32b_pal.c \
dispatch.c


m68k-atari-mint-ld -o aranym.sys -L/usr/lib/gcc-lib/m68k-atari-mint/2.95.3 \
../common/c_common.s.gnu.o \
../common/common.s.gnu.o \
../common/clip.s.gnu.o \
../common/lineachk.s.gnu.o \
init.o \
colours.o \
spec.o \
8b_pal.o \
16b_pal.o \
32b_pal.o \
dispatch.o \
-lgcc

m68k-atari-mint-strip --strip-all aranym.sys

for file in ../../include/*.inc.gnu ../include/*.inc.gnu ../common/*.s.gnu ; do
rm $file;
done
