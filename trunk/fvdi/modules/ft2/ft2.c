/*
 * fVDI font load and setup
 *
 * $Id: ft2.c,v 1.11 2005-12-09 08:07:25 johan Exp $
 *
 * Copyright 1997-2000/2003, Johan Klockars 
 *                     2005, Standa Opichal
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#if 0
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>
#include <freetype/ttnameid.h>
#endif

#include "fvdi.h"
#include "globals.h"
#include "utility.h"
#include "function.h"
#include "list.h"

#undef CACHE_YSIZE

extern short Atari2Unicode[];

/* Cached glyph information */
typedef struct cached_glyph {
	int stored;
	FT_UInt index;
	FT_Bitmap bitmap;
	int minx;
	int maxx;
#if CACHE_YSIZE
	int miny;
	int maxy;
#endif
	int yoffset;
	int advance;
	short cached;
} c_glyph;


/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)	((X & -64) / 64)
#define FT_CEIL(X)	(((X + 63) & -64) / 64)

/* external leading */
#define LINE_GAP	1

#define CACHED_METRICS	0x10
#define CACHED_BITMAP	0x01

#if 1
#define DEBUG_FONTS 1
#endif

#if 0

/* The structure used to hold internal font information */
struct _TTF_Font {
	/* Freetype2 maintains all sorts of useful info itself */
	FT_Face face;
	/* filename and size */
	int	opened;
	char	*filename;
	int	ptsize;
	long	index;


	/* We'll cache these ourselves */
	int height;
	int ascent;
	int descent;
	int lineskip;

	/* The font style */
	int style;

	/* Extra width in glyph bounds for text styles */
	int glyph_overhang;
	float glyph_italics;

	/* Information in the font for underlining */
	int underline_offset;
	int underline_height;

	/* Cache for style-transformed glyphs */
	c_glyph *current;
	c_glyph cache[256];
	c_glyph scratch;
};

/* The FreeType font engine/library */
static int TTF_initialized = 0;


TTF_Font* TTF_OpenFontIndex(const char *file, int ptsize, long index)
{
	TTF_Font* font;

	font = (TTF_Font *)malloc(sizeof *font);
	if (font == NULL) {
		TTF_SetError("Out of memory");
		return NULL;
	}
	setmem(font, 0, sizeof(*font));

	font->filename = strdup(file);
	font->ptsize = ptsize;
	font->index = index;

	return TTF_FTOpen(font);
}

TTF_Font* TTF_OpenFont(const char *file, int ptsize)
{
	return TTF_OpenFontIndex(file, ptsize, 0);
}

#endif

static FT_Library library;
static LIST       fonts;

typedef struct {
	struct Linkable *next;
	struct Linkable *prev;
	Fontheader	*font;
} FontheaderListItem;


static FT_Error ft2_find_glyph(Fontheader* font, short ch, int want);


#define USE_FREETYPE_ERRORS 1

static char* ft2_error(const char *msg, FT_Error error)
{
	static char buffer[1024] = "uninitialized\r\n";
#ifdef USE_FREETYPE_ERRORS
#undef FTERRORS_H
#define FT_ERRORDEF(e, v, s)  {e, s},
	static const struct
	{
	  int          err_code;
	  const char*  err_msg;
	} ft_errors[] = {
#include <freetype/fterrors.h>
	};
	int i;
	const char *err_msg;

	err_msg = NULL;
	for(i = 0; i < ((sizeof ft_errors) / (sizeof ft_errors[0])); ++i) {
		if (error == ft_errors[i].err_code) {
			err_msg = ft_errors[i].err_msg;
			break;
		}
	}
	if (!err_msg) {
		err_msg = "unknown FreeType error";
	}
	sprintf(buffer, "%s: %s\r\n", msg, err_msg);
#endif /* USE_FREETYPE_ERRORS */
	return buffer;
}


void ft2_term(void)
{
	FT_Done_FreeType(library);
}

int ft2_init(void)
{
	FT_Error error;
       	error = FT_Init_FreeType(&library);
	if (error)
		return -1;
	
	/* Initialize Fontheader LRU cache */
	listInit(&fonts);

	return 0;
}


static void ft2_close_face(Fontheader *font)
{
	if (!font->extra.unpacked.data)
		return;

#ifdef DEBUG_FONTS
	if (debug > 1) {
		char buf[10];
		ltoa(buf, (long)font->size, 10);
		access->funcs.puts("FT2 close_face: ");
		access->funcs.puts(font->extra.filename);
		access->funcs.puts("\r\n");
		access->funcs.puts(((FT_Face)font->extra.unpacked.data)->family_name);
	       	access->funcs.puts(", size: ");
	       	access->funcs.puts(buf);
		access->funcs.puts("\r\n");
	}
#endif

	FT_Done_Face((FT_Face)font->extra.unpacked.data);
	font->extra.unpacked.data = NULL;
}

static Fontheader* ft2_load_metrics(Fontheader *font, FT_Face face, short ptsize)
{
	FT_Error error;

	if (FT_IS_SCALABLE(face)) {
		FT_Fixed scale;

		/* Set the character size and use default DPI (72) */
		error = FT_Set_Char_Size(face, 0, ptsize * 64, 0, 0);
		if (error) {
			access->funcs.puts(ft2_error("FT2  Couldn't set vector font size", error));
			ft2_close_face(font);
			return NULL;
		}

		scale = face->size->metrics.y_scale;
		font->distance.ascent  = FT_CEIL(FT_MulFix(face->ascender, scale));
		font->distance.descent = FT_CEIL(FT_MulFix(-face->descender, scale));
		font->distance.top     = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
		font->distance.bottom  = FT_CEIL(FT_MulFix(-face->bbox.yMin, scale));

		/* This gives us weird values - perhaps caused by taking care of unusual characters out of Latin-1 charset */
		font->widest.cell = FT_CEIL(FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale));
		font->widest.character = FT_CEIL(FT_MulFix(face->max_advance_width, scale));

		font->underline = FT_FLOOR(FT_MulFix(face->underline_thickness, scale));
	} else if (face->num_fixed_sizes) {
		int s;
		/* Find the required font size in the bitmap face (if present) */
		if (debug > 1)
			access->funcs.puts("FT2 face_sizes:");
		for(s = 0; s < face->num_fixed_sizes; s++) {
			if (debug > 1) { 
				char buf[10];
				access->funcs.puts(", ");
				ltoa(buf, (long)face->available_sizes[s].size / 64, 10);
				access->funcs.puts(buf);
				access->funcs.puts(" [");
				ltoa(buf, (long)face->available_sizes[s].width, 10);
				access->funcs.puts(buf);
				access->funcs.puts(",");
				ltoa(buf, (long)face->available_sizes[s].height, 10);
				access->funcs.puts(buf);
				access->funcs.puts(" ]");
			}
			if (ptsize >= face->available_sizes[s].size / 64)
				break;
		}
		if (debug > 1)
			access->funcs.puts("\r\n");

		/* No size match -> pick the last one */
		if (s == face->num_fixed_sizes)
			s--;

		/* Set the character size and use default DPI (72) */
		error = FT_Set_Pixel_Sizes(face, face->available_sizes[s].width, face->available_sizes[s].height);
		if (error) {
			access->funcs.puts(ft2_error("FT2  Couldn't set bitmap font size", error));
			ft2_close_face(font);
			return NULL;
		}

		font->distance.ascent  = face->available_sizes[s].height;
		font->distance.descent = 0;
		font->distance.top     = face->available_sizes[s].height;
		font->distance.bottom  = 0;

		/* This gives us weird values - perhaps caused by taking care of unusual characters out of Latin-1 charset */
		font->widest.cell      = face->available_sizes[s].width;
		font->widest.character = face->available_sizes[s].width;

		font->underline = 1;
	}

#if 0
	if (0) { /* !!! Needs the cache fields allocated which is not the case at this point */
	       	/* Scan the font for widest parts */
		short ch;
		for(ch = 32; ch < 128; ch++) {
			FT_Error error = ft2_find_glyph(font, ch, CACHED_METRICS);
			if (!error) {
				c_glyph *glyph = font->extra.current;

				font->widest.cell      = MAX(font->widest.cell, glyph->maxx - glyph->minx);
				font->widest.character = MAX(font->widest.character, glyph->advance);
			}
		}
	}
#endif
	font->size = ptsize;

	/* Finish the font metrics fill-in */
	font->distance.half = (font->distance.top + font->distance.bottom) >> 1;
	font->height        = font->distance.ascent + font->distance.descent + LINE_GAP;

	/* Fake for vqt_fontinfo() as some apps might rely on this */
	font->code.low  = 0;
	font->code.high = 255;

	//FIXME: font->lineskip = FT_CEIL(FT_MulFix(face->height, scale));
	//FIXME: font->underline_offset = FT_FLOOR(FT_MulFix(face->underline_position, scale));

	if (font->underline < 1) {
		font->underline = 1;
	}

#if 0
	if (debug > 1) {
		char buf[255];
		access->funcs.puts("Font metrics:\r\n");
		sprintf(buf,"\tascent = %d, descent = %d\r\n",
		        font->distance.ascent, font->distance.descent);
		access->funcs.puts(buf);
		sprintf(buf,"\ttop = %d, bottom = %d\r\n",
		        font->distance.top, font->distance.bottom);
		access->funcs.puts(buf);
		sprintf(buf,"\tcell = %d, character = %d\r\n",
		        font->widest.cell, font->widest.character);
		access->funcs.puts(buf);
	}
#endif

	/* Set the default font style */
	//FIXME: font->style = TTF_STYLE_NORMAL;
#if CAN_BOLD
	font->glyph_overhang = 0; //FIXME: face->size->metrics.y_ppem / 10;
#endif
	/* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
	// font->glyph_italics = 0.207f;
	// font->glyph_italics *= font->height;

	{ /* Fixup to handle alignments better way */
		int top = font->distance.top;
		font->extra.distance.base    = -top;
		font->extra.distance.half    = -top + font->distance.half;
		font->extra.distance.ascent  = -top + font->distance.ascent;
		font->extra.distance.bottom  = -top - font->distance.bottom;
		font->extra.distance.descent = -top - font->distance.descent;
		font->extra.distance.top     = 0;
	}

	return font;
}

/*
 * Load a font and make it ready for use
 */
Fontheader *ft2_load_font(const char *filename)
{
   Fontheader *font = (Fontheader *)malloc(sizeof(Fontheader));
   if (font) {
	   static id = 5000;
	   FT_Error error;
	   FT_Face face;

	   ft_keep_open();
	   /* Open the font and create ancillary data */
	   error = FT_New_Face(library, filename, 0, &face);
	   if (error) {
		   ft_keep_closed();
		   free(font);
		   return NULL;
	   }

	   /* Clear the structure */
	   memset(font, 0, sizeof(Fontheader));

#if 0
	   if (debug > 0) {
#else
	   {
#endif
		   char buf[255];
		   strcpy(buf, face->family_name);
		   strcat(buf, " " );
		   strcat(buf, face->style_name);		/* FIXME: Concatenate? */
		   strncpy(font->name, buf, 32);		/* family name would be the font name? */
	   }

	   font->id = id++;
	   font->flags = 0x8000 |				/* FT2 handled font */
			 (FT_IS_SCALABLE(face)    ? 0x4000 : 0) |
		  	 (FT_IS_FIXED_WIDTH(face) ? 0x0008 : 0);	/* .FNT compatible flag */
	   font->extra.filename = strdup(filename);		/* Font filename to load_glyphs on-demand */
	   font->extra.index = 0;				/* Index to load, FIXME: how to we load multiple of them */


	   if (!face->num_fixed_sizes) {
		   font->size = 0;				/* Vector fonts have size = 0 */
	   } else {
		   font->size = face->available_sizes[0].size / 64;	/* Bitmap font size */
	   }

	   FT_Done_Face(face);
	   ft_keep_closed();

	   if (debug > 0) {
		   char buf[10];
		   ltoa(buf, (long)font->size, 10);
		   access->funcs.puts("FT2 load_font: ");
		   access->funcs.puts(font->name);
		   access->funcs.puts(": size=");
		   access->funcs.puts(buf);
		   access->funcs.puts("\r\n");
	   }

	   /* By default faces should not be kept in memory... (void*)face */;
	   font->extra.unpacked.data = NULL;
	   font->extra.cache         = NULL;
	   font->extra.scratch       = NULL;
   }

   return font;
}


static Fontheader* ft2_open_face(Fontheader *font, short ptsize)
{
	FT_Error error;
	FT_Face face;

#if 0
	if (font->extra.unpacked.data)
		return font;
#endif

#ifdef DEBUG_FONTS
	if (debug > 1) {
		access->funcs.puts("FT2  open_face: ");
		access->funcs.puts(font->extra.filename);
		access->funcs.puts("\r\n");
	}
#endif

	/* Open the font and create ancillary data */
	error = FT_New_Face(library, font->extra.filename, 0, &face);
	if (error) {
		access->funcs.puts(ft2_error("FT2  open_face error: ", error));
		return NULL;
	}

#ifdef DEBUG_FONTS
	if (debug > 1) {
		char buf[10];
		ltoa(buf, (long)ptsize, 10);
		access->funcs.puts(face->family_name);
		access->funcs.puts(", size: ");
		access->funcs.puts(buf);
		access->funcs.puts("\r\n");
	}
#endif

	if (font->extra.index != 0) {
		if (face->num_faces > font->extra.index) {
		  	FT_Done_Face(face);
			error = FT_New_Face(library, font->extra.filename, font->extra.index, &face);
			if (error) {
				access->funcs.puts(ft2_error("FT2  Couldn't get font face", error));
				return NULL;
			}
		} else {
			access->funcs.puts(ft2_error("FT2  No such font face", error));
			return NULL;
		}
	}

	if (!font->extra.cache) {
	   font->extra.cache = malloc(sizeof(c_glyph) * 256);	/* Cache */
	   memset(font->extra.cache, 0, sizeof(c_glyph) * 256);	/* Cache */
	   font->extra.scratch = malloc(sizeof(c_glyph));	/* Scratch */
	   memset(font->extra.scratch, 0, sizeof(c_glyph));
	}

	font = ft2_load_metrics(font, face, ptsize);
	if (!font) {
		access->funcs.puts("FT2  Cannot load metrics\r\n");
		return NULL;
	}

	/* Face loaded successfully */
	font->extra.unpacked.data = (void *)face;

	return font;
}

static inline FT_Face ft2_get_face(Fontheader *font)
{
	/* Open the face if needed */
	if (!font->extra.unpacked.data) {
		font = ft2_open_face(font, font->size);
	}

	return (FT_Face)font->extra.unpacked.data;
}

static Fontheader *ft2_dup_font(Fontheader *src, short ptsize)
{
   Fontheader *font = (Fontheader *)malloc(sizeof(Fontheader));
   if (font) {
	   memcpy(font, src, sizeof(Fontheader));
	   font->extra.filename = strdup(src->extra.filename);

	   /* Only for rendering fonts */
	   font->extra.unpacked.data = NULL;
	   font->extra.cache = NULL;

   	   /* underline == 0 -> metrics were not read yet */
	   font->underline = 0;

#ifdef DEBUG_FONTS
	   if (debug > 1) {
		   char buf[10];
		   ltoa(buf, (long)font->size, 10);
		   access->funcs.puts("FT2  dup_font: ");
		   access->funcs.puts(font->name);
		   access->funcs.puts(", size: ");
		   access->funcs.puts(buf);
		   access->funcs.puts("\r\n");
	   }
#endif

	   font = ft2_open_face(font, ptsize);
   }

   return font;
}

static void ft2_dispose_font(Fontheader *font)
{
	/* Close the FreeType2 face */
	ft2_close_face(font);

	/* Dispose of the data */
	free(font->extra.filename);
	free(font->extra.cache);
	free(font->extra.scratch);
	free(font);
}

static FT_Error ft2_load_glyph(Fontheader *font, short ch, c_glyph *cached, int want)
{
	FT_Face face;
	FT_Error error;
	FT_GlyphSlot glyph;
	FT_Glyph_Metrics *metrics;
	FT_Outline *outline;

	face = ft2_get_face(font);

	/* Load the glyph */
	if (!cached->index) {
		cached->index = FT_Get_Char_Index(face, Atari2Unicode[ch]);
	}
	error = FT_Load_Glyph(face, cached->index, FT_LOAD_DEFAULT);
	if (error) {
		return error;
	}

	/* Get our glyph shortcuts */
	glyph = face->glyph;
	metrics = &glyph->metrics;
	outline = &glyph->outline;

	/* Get the glyph metrics if desired */
	if ((want & CACHED_METRICS) && !(cached->stored & CACHED_METRICS)) {
		/* Get the bounding box */
#if 1
		FT_Glyph g;
		FT_BBox bbox;
		FT_Get_Glyph(glyph, &g);
		FT_Glyph_Get_CBox(g, FT_GLYPH_BBOX_PIXELS, &bbox);

		cached->minx = bbox.xMin;
		cached->maxx = bbox.xMax;
		cached->yoffset = font->distance.ascent - bbox.yMax;
#else

		cached->minx = FT_FLOOR(metrics->horiBearingX);
		cached->maxx = cached->minx + FT_CEIL(metrics->width);
#if CACHE_YSIZE
		cached->maxy = FT_FLOOR(metrics->horiBearingY);
		cached->miny = cached->maxy - FT_CEIL(metrics->height);

		cached->yoffset = font->distance.ascent - cached->maxy;
#else
		cached->yoffset = font->distance.ascent - FT_FLOOR(metrics->horiBearingY);
#endif
#endif
		cached->advance = FT_CEIL(metrics->horiAdvance);

#if 0
		/* Adjust for bold and italic text */
		if (font->style & TTF_STYLE_BOLD) {
			cached->maxx += font->glyph_overhang;
		}
		if (font->style & TTF_STYLE_ITALIC) {
			cached->maxx += (int)ceil(font->glyph_italics);
		}
#endif
		cached->stored |= CACHED_METRICS;
	}

	if (((want & CACHED_BITMAP) && !(cached->stored & CACHED_BITMAP))) { 
		int i;
		FT_Bitmap* src;
		FT_Bitmap* dst;

#if 0
		/* Handle the italic style */
		if (font->style & TTF_STYLE_ITALIC) {
			FT_Matrix shear;

			shear.xx = 1 << 16;
			shear.xy = (int) (font->glyph_italics * (1 << 16)) / font->height;
			shear.yx = 0;
			shear.yy = 1 << 16;

			FT_Outline_Transform(outline, &shear);
		}
#endif

		/* Render the glyph */
		error = FT_Render_Glyph(glyph, ft_render_mode_mono);
		if (error) {
			return error;
		}

		/* Copy over information to cache */
		src = &glyph->bitmap;
		dst = &cached->bitmap;
		memcpy(dst, src, sizeof(*dst));

#if 0
		/* Adjust for bold and italic text */
		if (font->style & TTF_STYLE_BOLD) {
			int bump = font->glyph_overhang;
			dst->width += bump;
		}
		if (font->style & TTF_STYLE_ITALIC) {
			int bump = (int)ceil(font->glyph_italics);
			dst->width += bump;
		}
#endif
#if 0
		dst->pitch = (dst->width + 7) >> 3;
#else
		dst->pitch = ((dst->width + 15) >> 4) * 2;   /* Only whole words */
#endif

		if (dst->rows != 0) {
			dst->buffer = malloc(dst->pitch * dst->rows);
			if (!dst->buffer) {
				return FT_Err_Out_Of_Memory;
			}
			setmem(dst->buffer, 0, dst->pitch * dst->rows);

			for(i = 0; i < src->rows; i++) {
				int soffset = i * src->pitch;
				int doffset = i * dst->pitch;
				memcpy(dst->buffer + doffset,
				       src->buffer + soffset, src->pitch);
			}
		}

#if 0
		/* Handle the bold style */
		if (0 && font->style & TTF_STYLE_BOLD) {
			int row;
			int col;
			int offset;
			int pixel;
			unsigned char* pixmap;

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
   supports 256 shades of gray, but we should instead key off of num_grays
   in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS       256

			/* The pixmap is a little hard, we have to add and clamp */
			for(row = dst->rows - 1; row >= 0; --row) {
				pixmap = (unsigned char*) dst->buffer + row * dst->pitch;
				for(offset = 1; offset <= font->glyph_overhang; ++offset) {
					for(col = dst->width - 1; col > 0; --col) {
						pixel = (pixmap[col] + pixmap[col - 1]);
						if (pixel > NUM_GRAYS - 1) {
							pixel = NUM_GRAYS - 1;
						}
						pixmap[col] = (unsigned char)pixel;
					}
				}
			}
		}
#endif

		/* Mark that we rendered this format */
		cached->stored |= CACHED_BITMAP;
	}

	/* We're done, mark this glyph cached */
	cached->cached = ch;

	return 0;
}

static void ft2_flush_glyph(c_glyph* glyph)
{
	glyph->stored = 0;
	glyph->index = 0;
	if (glyph->bitmap.buffer) {
		free(glyph->bitmap.buffer);
		glyph->bitmap.buffer = 0;
	}
	glyph->cached = 0;
}
	
static void ft2_flush_cache(Fontheader *font)
{
	int i;
	int size = 256;

	for(i = 0; i < size; ++i) {
		if (((c_glyph *)font->extra.cache)[i].cached) {
			ft2_flush_glyph(&((c_glyph *)font->extra.cache)[i]);
		}

	}
	if (((c_glyph *)font->extra.scratch)->cached) {
		ft2_flush_glyph(font->extra.scratch);
	}
}

static FT_Error ft2_find_glyph(Fontheader* font, short ch, int want)
{
	int retval = 0;

	if (ch < 256) {
		font->extra.current = &((c_glyph *)font->extra.cache)[ch];
	} else {
		if (((c_glyph *)font->extra.scratch)->cached != ch) {
			ft2_flush_glyph(font->extra.scratch);
		}
		font->extra.current = font->extra.scratch;
	}
	if ((((c_glyph *)font->extra.current)->stored & want) != want) {
		retval = ft2_load_glyph(font, ch, font->extra.current, want);
	}

	return retval;
}

void *ft2_char_bitmap(Fontheader *font, long ch)
{
  if (ft2_find_glyph(font, ch, CACHED_BITMAP))
    return 0;

  return ((c_glyph *)font->extra.current)->bitmap.buffer;
}

int ft2_text_size(Fontheader *font, const short *text, int *w, int *h)
{
#if 0
	char buf[255];
#endif
	int status;
	const short *ch;
	int x, z;
	int minx, maxx;
	int miny, maxy;
	c_glyph *glyph;
	FT_Error error;

	/* Initialize everything to 0 */
	status = 0;
	minx = maxx = 0;
	miny = maxy = 0;

	/* Load each character and sum it's bounding box */
	x = 0;
	for(ch = text; *ch; ++ch) {
#if 0
		buf[ch - text] = *ch;
#endif
		error = ft2_find_glyph(font, *ch, CACHED_METRICS);
		if (error) {
			return -1;
		}
		glyph = font->extra.current;

		z = x + glyph->minx;
		if (minx > z) {
			minx = z;
		}
#if CAN_BOLD
		if (font->style & TTF_STYLE_BOLD) {
			x += font->glyph_overhang;
		}
#endif
		if (glyph->advance > glyph->maxx) {
			z = x + glyph->advance;
		} else {
			z = x + glyph->maxx;
		}
		if (maxx < z) {
			maxx = z;
		}
		x += glyph->advance;

#if CACHE_YSIZE
		if (glyph->miny < miny) {
			miny = glyph->miny;
		}
		if (glyph->maxy > maxy) {
			maxy = glyph->maxy;
		}
#endif
	}

#if 0
	buf[ch - text] = '\0';
#endif

	/* Fill the bounds rectangle */
	if (w) {
		*w = (maxx - minx);
	}
	if (h) {
#if CACHE_YSIZE
#if 0 /* This is correct, but breaks many applications */
		*h = (maxy - miny);
#else
		*h = font->height;
#endif
#else
		*h = font->height;
#endif
	}

#if 0
	if (1) {
		access->funcs.puts("txt width: \"");
		access->funcs.puts(buf);
		access->funcs.puts("\"\r\n");
		ltoa(buf, (long)*w, 10);
		access->funcs.puts("txt width: ");
		access->funcs.puts(buf);
		access->funcs.puts("\"\r\n");
	}
#endif

	return status;
}


MFDB *ft2_text_render(Fontheader *font, const short *text, MFDB *textbuf)
{
	int xstart;
	int width;
	int height;
	const short *ch;
	unsigned char *src;
	unsigned char *dst;
	unsigned char *dst_check;
	int row, col;
	c_glyph *glyph;

	FT_Bitmap *current;
	FT_Error error;
	FT_Long use_kerning;
	FT_UInt prev_index = 0;
	FT_Face face;

	/* Get the dimensions of the text surface */
	if ((ft2_text_size(font, text, &width, NULL) < 0) || !width) {
		// TTF_SetError("Text has zero width");
		return NULL;
	}
	height = font->height;

	/* Fill in the target surface */
	textbuf->width     = width;
	textbuf->height    = height;
	textbuf->standard  = 1;
	textbuf->bitplanes = 1;
	/* +1 for end write */
	textbuf->wdwidth   = ((width + 15) >> 4) + 1; /* Words per line */
	textbuf->address = malloc(textbuf->wdwidth * 2 * textbuf->height);
	if (textbuf->address == NULL) {
		return NULL;
	}
	memset(textbuf->address, 0, textbuf->wdwidth * 2 * textbuf->height);

	/* Adding bounds checking to avoid all kinds of memory
	 * corruption errors that may occur.
	 */
	dst_check = (unsigned char *)textbuf->address + textbuf->wdwidth * 2 * textbuf->height;

       	face = (FT_Face)font->extra.unpacked.data;

	/* Check kerning */
	use_kerning = 0; // FIXME: FT_HAS_KERNING(face);
	
	/* Load and render each character */
	xstart = 0;
	for(ch = text; *ch; ++ch) {
		short c = *ch;
#if 0
		int swapped;
		swapped = TTF_byteswapped;
		if (c == UNICODE_BOM_NATIVE) {
			swapped = 0;
			if (text == ch) {
				++text;
			}
			continue;
		}
		if (c == UNICODE_BOM_SWAPPED) {
			swapped = 1;
			if (text == ch) {
				++text;
			}
			continue;
		}
		if (swapped) {
			c = SDL_Swap16(c);
		}
#endif

#if 1
		error = ft2_find_glyph(font, c, CACHED_METRICS | CACHED_BITMAP);
		if (error) {
			free(textbuf->address);
			return NULL;
		}
		glyph = font->extra.current;
#else
		/* This should be done via a macro! */
#if 0
		c = Atari2Unicode[c];
#endif
		if (c < 256) {
			glyph = &((c_glyph *)font->extra.cache)[c];
		} else {
			if (((c_glyph *)font->extra.scratch)->cached != c) {
				ft2_flush_glyph(font->extra.scratch);
			}
			glyph = font->extra.scratch;
		}
		if ((glyph->stored & (CACHED_METRICS | CACHED_BITMAP)) !=
		    (CACHED_METRICS | CACHED_BITMAP)) {
			if (ft2_load_glyph(font, c, glyph,
			                   (CACHED_METRICS | CACHED_BITMAP))) {
				free(textbuf->address);
				return NULL;
			}
		}
#endif
		current = &glyph->bitmap;
		/* Ensure the width of the pixmap is correct. In some cases,
		 * FreeType may report a larger pixmap than possible.
		 */
		width = current->width;
		if (width > glyph->maxx - glyph->minx) {
			width = glyph->maxx - glyph->minx;
		}
		/* Do kerning, if possible AC-Patch */
		if (use_kerning && prev_index && glyph->index) {
			FT_Vector delta; 
			FT_Get_Kerning(face, prev_index, glyph->index,
			               ft_kerning_default, &delta); 
			xstart += delta.x >> 6;
		}
		/* Compensate for wrap around bug with negative minx's */
		if ((ch == text) && (glyph->minx < 0)) {
			xstart -= glyph->minx;
		}
		
		{
#if 0
			int offset = xstart + glyph->minx;
			short shift = offset % 8;
			unsigned char rmask = (1 << shift) - 1;
			unsigned char lmask = ~rmask;

			for(row = 0; row < current->rows; ++row) {
				/* Make sure we don't go either over, or under the
				 * limit */
				if (row + glyph->yoffset < 0) {
					continue;
				}
				if (row + glyph->yoffset >= textbuf->height) {
					continue;
				}

				dst = (unsigned char *)textbuf->address +
					(row + glyph->yoffset) * textbuf->wdwidth * 2 +
					(offset >> 3);
				src = current->buffer + row * current->pitch;

				for(col = (width + 7) >> 3; col > 0; --col) {
					unsigned char x = *src++;
					*dst++ |= (x & lmask) >> shift;

					/* Sanity end of buffer check */
					if (dst >= dst_check) {
						break;
					}

					*dst |= (x & rmask) << (8 - shift);
				}
			}
#else
			int offset = xstart + glyph->minx;
			short shift = offset % 8;
			int last_row;
			unsigned char *dst_base, *src_base, byte;
			
			row = 0;
			src_base = current->buffer;
			dst_base = (unsigned char *)textbuf->address +
			           (offset >> 3);
			if (glyph->yoffset < 0) {   /* Under limit? */
			    row      -= glyph->yoffset;
			    src_base += row * current->pitch;
			} else if (glyph->yoffset) {
			    dst_base += glyph->yoffset * textbuf->wdwidth * 2;
			}

			/* Over limit? */
			last_row = current->rows - 1;
			if (last_row + glyph->yoffset >= textbuf->height)
			    last_row = textbuf->height - glyph->yoffset - 1;

			width = (width + 7) >> 3;

			for(; row <= last_row; ++row) {

				dst = dst_base;
				dst_base += textbuf->wdwidth * 2;
				src = src_base;
				src_base += current->pitch;

				byte = *dst;
				for(col = width; col > 0; --col) {
					unsigned char x = *src++;
					*dst++ = byte | (x >> shift);
					byte = x << (8 - shift);
				}
				*dst = byte;
			}
#endif
		}

		xstart += glyph->advance;
#if CAN_BOLD
		if (font->style & TTF_STYLE_BOLD) {
			xstart += font->glyph_overhang;
		}
#endif
		prev_index = glyph->index;
	}

#if 0
	/* Handle the underline style */
	if (0 && font->style & TTF_STYLE_UNDERLINE) {
		row = font->ascent - font->underline_offset - 1;
		if (row >= textbuf->height) {
			row = (textbuf->height - 1) - font->underline_height;
		}
		dst = (unsigned char *)textbuf->address + row * textbuf->wdwidth * 2;
		for (row = font->underline_height; row > 0; --row) {
			/* 1 because 0 is the bg color */
			setmem(dst, 1, textbuf->width);
			dst += textbuf->wdwidth * 2;
		}
	}
#endif

	return textbuf;
}

/**
 * Maintains LRU cache of Fontheader instances coresponding to
 * different sizes of FreeType2 fonts loaded in the beginning
 * (which are maintained in the global font list normally).
 **/
Fontheader *ft2_find_fontsize(Fontheader *font, short ptsize)
{
	static short font_count = 0;
	Fontheader *f;
	FontheaderListItem *i;

	if ( !(font->flags & 0x4000) ) {
		/* fall back to the common add way of finding the right font size */
		f = font->extra.first_size;
		while (f->extra.next_size && (f->extra.next_size->size <= ptsize)) {
			f = f->extra.next_size;
		}
		/* set the closest available bitmap font size */
		ptsize = f->size;
	}

	/* LRU: put the selected font to the front of the list */
	listForEach(FontheaderListItem*, i, &fonts) {
		if (i->font->id == font->id && i->font->size == ptsize) {
			listRemove((LINKABLE *)i);
			listInsert(fonts.head.next, (LINKABLE *)i);
			return i->font;
		}
	}

	if (debug > 1) {
		char buf[10];
		ltoa(buf, (long)ptsize, 10);
		puts("FT2 find_font: fetch size="); puts_nl(buf);
	}

	/* FIXME: handle maximum number of fonts in the cache here (configurable) */
	if (font_count > 10) {
		FontheaderListItem *x = (FontheaderListItem *)listLast(&fonts);
		listRemove((LINKABLE *)x);
		if ( x->font->flags & 0x4000 ) {
			ft2_dispose_font(x->font); /* remove the whole font */
		} else {
			ft2_close_face(x->font); /* just close the FT2 face */
		}
		free(x);
		font_count--;
	}

	/* create additional size face as it is scalable font */
	if ( font->flags & 0x4000 ) {
		f = ft2_dup_font(font, ptsize);
	} else {
		f = font;

		/* read the font metrics before */
		if ( !font->underline ) {
			f = ft2_open_face(font, font->size);
		}

		if (debug > 1) {
			char buf[10];
			puts("FT2 find_font: bitmap id="); ltoa(buf, (long)f->id, 10); puts(buf);
			puts(" size="); ltoa(buf, (long)f->size, 10); puts_nl(buf);
		}
	}

	if (f) {
		i = malloc(sizeof(FontheaderListItem));
		i->font = f;
		listInsert(fonts.head.next, (LINKABLE *)i);
		font_count++;
	}

	return f;
}


long ft2_text_render_default(Virtual *vwk, unsigned long coords, short *s, long slen)
{
	Fontheader *font = vwk->text.current_font;
	MFDB textbuf, *t;

#ifdef DEBUG_FONTS
	if (debug > 2) {
		char buffer[10];
		ltoa(buffer, (long)slen, 10);
		puts("Text len: ");
		puts_nl(buffer);
	}
#endif

	/* FIXME: this should not happen once we have all the font id/size setup routines intercepted */
	if (!font->size) {
		access->funcs.puts("FT2  text_render_default font->size == 0\r\n");
		/* Create a copy of the font for the particular size */
		font = ft2_find_fontsize(font, 16);
		if (!font) {
			access->funcs.puts("Cannot open face\r\n");
			return 0;
		}
	}

	/* Terminate text */
	s[slen] = 0;
	t = ft2_text_render(font, s, &textbuf); 
	if (t && t->address) {
		short colors[2];
		short pxy[8];
		short x = coords >> 16;
		short y = coords & 0xffffUL;

		colors[1] = vwk->text.colour.background;
		colors[0] = vwk->text.colour.foreground;

		y += ((short *)&font->extra.distance)[vwk->text.alignment.vertical];

		pxy[0] = 0;
		pxy[1] = 0;
		pxy[2] = t->width - 1;
		pxy[3] = t->height - 1;
		pxy[4] = x;
		pxy[5] = y;
		pxy[6] = x + t->width - 1;
		pxy[7] = y + t->height - 1;
		
		lib_vdi_spppp(&lib_vrt_cpyfm, vwk, vwk->mode, pxy, t, NULL, colors);
		free(t->address);
	}

	/* Dispose of the FreeType2 objects */
	// ft2_close_face(font);

	return 1;
}

long ft2_char_width(Fontheader *font, long ch)
{
	short s[] = {ch, 0};
	int width;
	/* Get the dimensions of the text surface */
	if ((ft2_text_size(font, s, &width, NULL) < 0) || !width) {
		return 0;
	}

	return width;
}

long ft2_text_width(Fontheader *font, short *s, long slen)
{
	int width;

	/* Terminate text */
	s[slen] = 0;
	/* Get the dimensions of the text surface */
	if ((ft2_text_size(font, s, &width, NULL) < 0) || !width) {
		return 0;
	}

	return width;
}

Fontheader *ft2_vst_point(Virtual *vwk, long ptsize, unsigned short *sizes)
{
	Fontheader *font = vwk->text.current_font;

	if (font->size == ptsize)
		return font;

        if (ptsize > 32000)
		ptsize = 32000;

	if (sizes) {
#if 1
		while(*(sizes + 1) <= ptsize)
			sizes++;
		ptsize = *sizes;
#else
		char buf[10];
		puts("Searching ");
		ltoa(buf, (long)sizes, 16);
		puts(buf);
		puts(" for ");
		ltoa(buf, ptsize, 10);
		puts(buf);
		puts(": ");
		
		while(*(sizes + 1) <= ptsize) {
		  ltoa(buf, *(sizes + 1), 10);
		  puts(buf);
		  puts(" ");
			sizes++;
		}
		ptsize = *sizes;
		ltoa(buf, ptsize, 10);
		puts(buf);
		puts("\x0a\x0d");
#endif
	}
#if 0
	else {
	  char buf[10];
	  puts("No search for size ");
	  ltoa(buf, ptsize, 10);
	  puts(buf);
	  puts("\x0a\x0d");
	}
#endif

	font = ft2_find_fontsize(font, ptsize);

	/* Dispose of the FreeType2 objects */
	// ft2_close_face(font);

	return font;

}


#if 0

long ft2_height2point(long height)
{
	return face->size->metrics.y_scale;
}

Fontheader *ft2_vst_height(Virtual *vwk, long height)
{
	return ft2_vst_point(vwk, ft2_height2point(height));
}



char *TTF_FontFaceStyleName(TTF_Font *font)
{
	return font->face->style_name;
}



void TTF_FTClose(TTF_Font *font)
{
	if (font->opened) {
		FT_Done_Face(font->face);
		font->opened = 0;
	}
}

void TTF_CloseFont(TTF_Font *font)
{
	ft2_flush_cache(font);
	TTF_FTClose(font);
	free(font->filename);
	free(font);
}

int TTF_FontHeight(TTF_Font *font)
{
	return font->height;
}

int TTF_FontAscent(TTF_Font *font)
{
       return font->ascent;
}

int TTF_FontDescent(TTF_Font *font)
{
	return font->descent;
}

int TTF_FontLineSkip(TTF_Font *font)
{
	return font->lineskip;
}

long TTF_FontFaces(TTF_Font *font)
{
	return font->face->num_faces;
}

int TTF_FontFaceIsFixedWidth(TTF_Font *font)
{
	return FT_IS_FIXED_WIDTH(font->face);
}

char *TTF_FontFaceFamilyName(TTF_Font *font)
{
	return font->face->family_name;
}

char *TTF_FontFaceStyleName(TTF_Font *font)
{
	return font->face->style_name;
}

int TTF_GlyphMetrics(TTF_Font *font, Uint16 ch,
                     int *minx, int *maxx, int *miny, int *maxy, int *advance)
{
	FT_Error error;

	error = ft2_find_glyph(font, ch, CACHED_METRICS);
	if (error) {
		TTF_SetFTError("Couldn't find glyph", error);
		return -1;
	}

	if (minx) {
		*minx = font->current->minx;
	}
	if (maxx) {
		*maxx = font->current->maxx;
	}
	if (miny) {
		*miny = font->current->miny;
	}
	if (maxy) {
		*maxy = font->current->maxy;
	}
	if (advance) {
		*advance = font->current->advance;
	}

	return 0;
}


void TTF_SetFontStyle(TTF_Font* font, int style)
{
	font->style = style;
	ft2_flush_cache(font);
}

int TTF_GetFontStyle(TTF_Font* font)
{
	return font->style;
}




static Uint16 *ASCII_to_UNICODE(Uint16 *unicode, const char *text, int len)
{
	int i;

	for(i = 0; i < len; ++i) {
		unicode[i] = ((const unsigned char *)text)[i];
	}
	unicode[i] = 0;

	return unicode;
}

int TTF_SizeText(TTF_Font *font, const char *text, int *w, int *h)
{
	Uint16 *unicode_text;
	int unicode_len;
	int status;

	/* Copy the Latin-1 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len + 1) * (sizeof *unicode_text));
	if (unicode_text == NULL) {
		TTF_SetError("Out of memory");
		return -1;
	}
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	status = TTF_SizeUNICODE(font, unicode_text, w, h);

	/* Free the text buffer and return */
	free(unicode_text);

	return status;
}

/* Convert the Latin-1 text to UNICODE and render it
*/
MFDB *TTF_RenderText_Solid(TTF_Font *font,
                           const char *text, MFDB *textbuf)
{
	Uint16 *unicode_text;
	int unicode_len;

	/* Copy the Latin-1 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len + 1) * (sizeof *unicode_text));
	if (unicode_text == NULL) {
		TTF_SetError("Out of memory");
		return(NULL);
	}
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, textbuf);

	/* Free the text buffer and return */
	free(unicode_text);
	return(textbuf);
}

#endif
