#!/bin/sh

for file in $@; do
# Comments with * in the first position on the line
sed "s%^\*.*%%" $file >$file.gnu
mv $file.gnu $file.tmp

# Comments after ; separator
sed "s%;.*$%%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Conditional compilation
sed "s%^\\([ 	][ 	]*\\)ifne%\1.ifne%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ 	][ 	]*\\)ifeq%\1.ifeq%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ 	][ 	]*\\)ifge%\1.ifge%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ 	][ 	]*\\)else%\1.else%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ 	][ 	]*\\)endc%\1.endif%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Constants
sed "s%^\\([^ 	]*\\)[ 	][ 	]*equ[ 	][ 	]*%	.equiv	\\1,%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([^ 	]*\\)[ 	][ 	]*set[ 	][ 	]*%	.set	\\1,%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Sections
sed "s%^\\([ 	][ 	]*\\)text%\1.text%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ 	][ 	]*\\)data%\1.data%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%^\\([ 	][ 	]*\\)bss%\1.bss%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Includes
sed "s%\.inc\"%.inc.gnu\"%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ 	][ 	]*\\)include%\1.include%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Hexadecimal numbers
sed "s%\\$%0x%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Data
sed "s%\\([ 	][ 	]*\\)dc.b%\1.byte%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ 	][ 	]*\\)dc.w%\1.word%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ 	][ 	]*\\)dc.l%\1.long%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp

# Miscellaneous directives
sed "s%\.end\\([ 	][ 	]*\|$\|\:\\)%.end___fix\1%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ 	][ 	]*\\)end\\(\\([ 	][ 	]*\\)\|$\\)%\1.end%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ 	][ 	]*\\)end\\([ 	][ 	]*\\)%\1.end%" $file.tmp >$file.gnu
mv $file.gnu $file.tmp
sed "s%\\([ 	][ 	]*\\)end\\($\\)%\1.end%" $file.tmp >$file.gnu
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

