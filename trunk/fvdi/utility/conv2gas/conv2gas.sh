#!/bin/sh

for file in $@; do
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
`dirname $0`/conv2gas $file.tmp >$file.gnu

# Remove temporary file
rm $file.tmp

done

