#ifndef MISC_H
#define MISC_H

#define SUPPORTED_TABLE 0x10

#define Q_NEXT_DRIVER	1
#define Q_FILE	2
#define S_DEBUG	3
#define S_OPTION	4
#define S_DRVOPTION	5
#define Q_NAME	100
#define S_SCREEN	101
#define S_AESBUF	102
#define S_CACHEIMG	103
#define S_DOBLIT	104


struct Info {
   unsigned short version;
   short flags;
   long (*remove)(void);
   long (*setup)(unsigned long type, long value);
   struct fVDI_log *log;
} ;

#endif
