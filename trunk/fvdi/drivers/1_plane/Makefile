#!/bin/sh

# This is obviously not yet an actual Makefile.
# For now, use:
# source Makefile
# or something equivalent.
#
# Look at the end for compiler version dependent things.

gcc -o ../../utility/conv2gas/conv2gas ../../utility/conv2gas/conv2gas.c

for file in ../../include/*.inc ../include/*.inc *.inc *.s ; do
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

for file in *.s.gnu ; do
echo Assembling $file ;
m68k-atari-mint-as -m68040 --defsym gas=1 --defsym lattice=0 -I../../include -I../include -I. -o $file.o <$file ;
done

m68k-atari-mint-gcc -O2 -c -I../../include/ 1_spec.c ../common/init.c 

m68k-atari-mint-ld -o fvdi_gnu.prg -L/usr/lib/gcc-lib/m68k-atari-mint/2.95.3 \
init.o \
1_spec.o \
1_expand.o \
1_fill.o \
1_line.o \
1_text.o \
1_pixel.o \
-lgcc

m68k-atari-mint-strip --strip-all 1_plane.sys

for file in ../../include/*.inc.gnu ../include/*.inc.gnu *.inc.gnu *.s.gnu ; do
rm $file;
done
