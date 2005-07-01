#
# Makefile for C struct to assembly equ converter
#
# $Id: Makefile,v 1.3 2005-07-01 09:51:44 johan Exp $
#
# Copyright 2002, Johan Klockars
# This software is licensed under the GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

CFLAGS		= -O2
LDFLAGS		= $(CFLAGS)
FLEXFLAGS	=
BISONFLAGS	=
LIBS		= -lfl

TARGET		= structer
CSOURCES	= main.c expr.c hash.c list.c memory.c outequ.c \
		  lex.yy.c struct.tab.c
CHEADERS	= expr.h hash.h list.h memory.h misc.h struct.tab.h
OBJECTS		= $(CSOURCES:.c=.o)
FLEXOUT		= lex.yy.c
BISONOUT	= struct.tab.c struct.tab.h

all:		depend $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

depend:		$(CSOURCES) $(CHEADERS)
	$(CC) -MM $(CSOURCES) > depend

clean:
	rm -f $(OBJECTS) $(FLEXOUT) $(BISONOUT) depend


$(BISONOUT):	struct.y
	bison -d $(BISONFLAGS) struct.y

$(FLEXOUT):	struct.lex
	flex $(FLEXFLAGS) struct.lex

include depend
