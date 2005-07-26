/***************************************************************************/
/*                                                                         */
/*  ftsystem.c                                                             */
/*                                                                         */
/*    ANSI-specific FreeType low-level system interface (body).            */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file contains the default interface used by FreeType to access   */
  /* low-level, i.e. memory management, i/o access as well as thread       */
  /* synchronisation.  It can be replaced by user-specific routines if     */
  /* necessary.                                                            */
  /*                                                                       */
  /*************************************************************************/


#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include FT_INTERNAL_DEBUG_H
#include FT_SYSTEM_H
#include FT_ERRORS_H
#include FT_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <mint/osbind.h>
#include <fcntl.h>

#include "globals.h"
#include "utility.h"

/* We are not a FreeMiNT kernel driver for now
 * -> we close the files immediately as we are done with the I/O operation.
 */
#if 0
#define KERNEL
#else
#undef KERNEL
#endif

#ifndef KERNEL
short keep_open = 0;
#endif

void ft_keep_open(void)
{
#ifndef KERNEL
  keep_open = 1;
#endif
}

/* This should go through all open files and close them */
void ft_keep_closed(void)
{
#ifndef KERNEL
  keep_open = 0;
#endif
}

  /*************************************************************************/
  /*                                                                       */
  /*                       MEMORY MANAGEMENT INTERFACE                     */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* It is not necessary to do any error checking for the                  */
  /* allocation-related functions.  This will be done by the higher level  */
  /* routines like FT_Alloc() or FT_Realloc().                             */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_alloc                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The memory allocation function.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory :: A pointer to the memory object.                          */
  /*                                                                       */
  /*    size   :: The requested size in bytes.                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The address of newly allocated block.                              */
  /*                                                                       */
  FT_CALLBACK_DEF( void* )
  ft_alloc( FT_Memory  memory,
            long       size )
  {
    /* FT_UNUSED( memory ); */


    void *addr = (void*)malloc( size );
#if 0
    while( size-- > 0 ) ((char*)addr)[size] = 0;
    memset( addr, 0, size);
    access->funcs.puts("FT2: alloc\r\n");
#endif
    return addr;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_realloc                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The memory reallocation function.                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory   :: A pointer to the memory object.                        */
  /*                                                                       */
  /*    cur_size :: The current size of the allocated memory block.        */
  /*                                                                       */
  /*    new_size :: The newly requested size in bytes.                     */
  /*                                                                       */
  /*    block    :: The current address of the block in memory.            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The address of the reallocated memory block.                       */
  /*                                                                       */
  FT_CALLBACK_DEF( void* )
  ft_realloc( FT_Memory  memory,
              long       cur_size,
              long       new_size,
              void*      block )
  {
    FT_UNUSED( memory );
    FT_UNUSED( cur_size );

    return (void*)realloc( block, new_size );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_free                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The memory release function.                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory  :: A pointer to the memory object.                         */
  /*                                                                       */
  /*    block   :: The address of block in memory to be freed.             */
  /*                                                                       */
  FT_CALLBACK_DEF( void )
  ft_free( FT_Memory  memory,
           void*      block )
  {
    FT_UNUSED( memory );

    free( block );
  }


  /*************************************************************************/
  /*                                                                       */
  /*                     RESOURCE MANAGEMENT INTERFACE                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_io

  /* We use the macro STREAM_FILE for convenience to extract the       */
  /* system-specific stream handle from a given FreeType stream object */
#define STREAM_FILE( stream )  ( (int)stream->descriptor.value )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_ansi_stream_close                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The function to close a stream.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream :: A pointer to the stream object.                          */
  /*                                                                       */
  FT_CALLBACK_DEF( void )
  ft_ansi_stream_close( FT_Stream  stream )
  {
#ifdef KERNEL
    Fclose( STREAM_FILE( stream ) );
#else
 #if 0
 #else
#if 1
    access->funcs.puts("FT2: close\r\n");
#endif
    if (STREAM_FILE(stream) != 0xbadc0de1) {
      Fclose( STREAM_FILE( stream ) );
    }
 #endif
#endif

    stream->descriptor.value   = 0;
    stream->size               = 0;
    stream->base               = 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_ansi_stream_io                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The function to read from a stream.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream :: A pointer to the stream object.                          */
  /*                                                                       */
  /*    offset :: The position in the data stream to start reading.        */
  /*                                                                       */
  /*    buffer :: The address of buffer to store the read data.            */
  /*                                                                       */
  /*    count  :: The number of bytes to read from the stream.             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The number of bytes actually read.                                 */
  /*                                                                       */
  FT_CALLBACK_DEF( unsigned long )
  ft_ansi_stream_io( FT_Stream       stream,
                     unsigned long   offset,
                     unsigned char*  buffer,
                     unsigned long   count )
  {
#ifdef KERNEL
    int file = STREAM_FILE( stream );

    Fseek( offset, file, SEEK_SET );

    return (unsigned long)Fread( file, count, buffer );
#else
 #if 0
    unsigned long ret;
    int file = Fopen( stream->pathname.pointer, O_RDONLY );
    Fseek( offset, file, SEEK_SET );
    ret = (unsigned long)Fread( file, count, buffer );
    Fclose( file );
    return ret;
 #else
    unsigned long ret;
#if 0
    access->funcs.puts("FT2: io\r\n");
#endif
    if (stream->descriptor.value == 0xbadc0de1) {
      int file = Fopen( stream->pathname.pointer, O_RDONLY );
      Fseek( offset, file, SEEK_SET );
      ret = (unsigned long)Fread( file, count, buffer );
      if (!keep_open) {
	Fclose( file );
      } else {
	stream->descriptor.value = file;
      }
      return ret;
    } else {
      int file = STREAM_FILE( stream );

      Fseek( offset, file, SEEK_SET );

      ret = (unsigned long)Fread( file, count, buffer );
      if (!keep_open) {
	Fclose( file);
	stream->descriptor.value = 0xbadc0de1;
      }
      return ret;
    }
 #endif
#endif
  }


  /* documentation is in ftobjs.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Stream_Open( FT_Stream    stream,
                  const char*  filepathname )
  {
    int file;


    if ( !stream )
      return FT_Err_Invalid_Stream_Handle;

    file = Fopen( filepathname, O_RDONLY );
    if ( file < 0 )
    {
      FT_ERROR(( "FT_Stream_Open:" ));
      FT_ERROR(( " could not open `%s'\n", filepathname ));

      return FT_Err_Cannot_Open_Resource;
    }

    stream->size = Fseek( 0, file, SEEK_END );
#ifdef KERNEL
    Fseek( 0, file, SEEK_SET );
#else
 #if 0
    Fclose( file );
 #else
#if 1
    access->funcs.puts("FT2: open\r\n");
#endif
    if (!keep_open) {
      Fclose( file );
      file = 0xbadc0de1;
    }
 #endif
#endif

    stream->descriptor.value   = file;
    stream->pathname.pointer   = (char*)filepathname;
    stream->pos                = 0;

    stream->read  = ft_ansi_stream_io;
    stream->close = ft_ansi_stream_close;

    FT_TRACE1(( "FT_Stream_Open:" ));
    FT_TRACE1(( " opened `%s' (%d bytes) successfully\n",
                filepathname, stream->size ));

    return FT_Err_Ok;
  }


#ifdef FT_DEBUG_MEMORY

  extern FT_Int
  ft_mem_debug_init( FT_Memory  memory );

  extern void
  ft_mem_debug_done( FT_Memory  memory );

#endif


  /* documentation is in ftobjs.h */

  FT_EXPORT_DEF( FT_Memory )
  FT_New_Memory( void )
  {
    FT_Memory  memory;


    memory = (FT_Memory)malloc( sizeof ( *memory ) );
    if ( memory )
    {
      memory->user    = 0;
      memory->alloc   = ft_alloc;
      memory->realloc = ft_realloc;
      memory->free    = ft_free;
#ifdef FT_DEBUG_MEMORY
      ft_mem_debug_init( memory );
#endif
    }

    access->funcs.puts("FT2: Memory OK\r\n");

    return memory;
  }


  /* documentation is in ftobjs.h */

  FT_EXPORT_DEF( void )
  FT_Done_Memory( FT_Memory  memory )
  {
#ifdef FT_DEBUG_MEMORY
    ft_mem_debug_done( memory );
#endif
    memory->free( memory, memory );
  }


/* END */
