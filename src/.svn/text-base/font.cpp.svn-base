/*
 *
 * Author Vlad Seryakov vlad@crystalballinc.com
 *   
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://mozilla.org/.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * This object incorporated SDL_ttf library, thanks to the author:
 *
 *   Sam Lantinga slouken@libsdl.org
 *
 */

#include "lmbox.h"

#include <ft2build.h>
#include FT_TYPES_H
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_IDS_H
//#include <freetype/internal/ftobjs.h>

#ifndef FT_OPEN_STREAM
#define FT_OPEN_STREAM ft_open_stream
#endif

// Macro to convert a character to a Unicode value -- assume already Unicode
#define UNICODE(c)	                c

// FIXME: Right now we assume the gray-scale renderer Freetype is using
// supports 256 shades of gray, but we should instead key off of num_grays
// in the result FT_Bitmap after the FT_Render_Glyph() call.
#define NUM_GRAYS                       256

#define FT_FLOOR(X)	                ((X & -64) / 64)
#define FT_CEIL(X)	                (((X + 63) & -64) / 64)

#define CACHED_METRICS	                0x10
#define CACHED_BITMAP	                0x01
#define CACHED_PIXMAP	                0x02

#define TTF_STYLE_NORMAL	        0x00
#define TTF_STYLE_BOLD		        0x01
#define TTF_STYLE_ITALIC	        0x02
#define TTF_STYLE_UNDERLINE	        0x04

/* ZERO WIDTH NO-BREAKSPACE (Unicode byte order mark) */
#define UNICODE_BOM_NATIVE	0xFEFF
#define UNICODE_BOM_SWAPPED	0xFFFE

// Cached glyph information
typedef struct cached_glyph {
    int stored;
    FT_UInt index;
    FT_Bitmap bitmap;
    FT_Bitmap pixmap;
    int minx;
    int maxx;
    int miny;
    int maxy;
    int yoffset;
    int advance;
    Uint16 cached;
} c_glyph;

// The structure used to hold internal font information
struct _TTF_Font {
    FT_Face face;
    int height;
    int ascent;
    int descent;
    int lineskip;
    int style;
    int glyph_overhang;
    float glyph_italics;
    int underline_offset;
    int underline_height;
    c_glyph *current;
    c_glyph cache[256];
    c_glyph scratch;
    SDL_RWops *src;
    int freesrc;
    FT_Open_Args args;
    int font_size_family;
};

static void TTF_CloseFont(TTF_Font *font);
static TTF_Font * TTF_OpenFont(const char *file, int ptsize);
static TTF_Font * TTF_OpenFontIndex(const char *file, int ptsize, long index);
static TTF_Font * TTF_OpenFontIndexRW(SDL_RWops *src, int freesrc, int ptsize, long index);
static int TTF_SizeText(TTF_Font *font, const char *text, int *w, int *h);
static int TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h);
static int TTF_SizeUNICODE(TTF_Font *font, const Uint16 *text, int *w, int *h);
static SDL_Surface * TTF_RenderText_Solid(TTF_Font *font, const char *text, SDL_Color fg);
static SDL_Surface * TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg);
static SDL_Surface * TTF_RenderUNICODE_Solid(TTF_Font *font, const Uint16 *text, SDL_Color fg);
static SDL_Surface * TTF_RenderText_Shaded(TTF_Font *font, const char *text, SDL_Color fg, SDL_Color bg);
static SDL_Surface * TTF_RenderUTF8_Shaded(TTF_Font *font, const char *text, SDL_Color fg, SDL_Color bg);
static SDL_Surface * TTF_RenderUNICODE_Shaded(TTF_Font *font, const Uint16 *text, SDL_Color fg, SDL_Color bg);
static SDL_Surface * TTF_RenderText_Blended(TTF_Font *font, const char *text, SDL_Color fg);
static SDL_Surface * TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg);
static SDL_Surface * TTF_RenderUNICODE_Blended(TTF_Font *font,	const Uint16 *text, SDL_Color fg);

// The FreeType font engine/library
static FT_Library library;
static int TTF_initialized = 0;
static int TTF_byteswapped = 0;

static unsigned long RWread(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    SDL_RWops *src = (SDL_RWops *)stream->descriptor.pointer;
    SDL_RWseek(src, (int)offset, SEEK_SET);
    return SDL_RWread(src, buffer, 1, (int)count);
}

static TTF_Font* TTF_OpenFontIndexRW(SDL_RWops *src, int freesrc, int ptsize, long index)
{
    TTF_Font* font;
    FT_Error error;
    FT_Face face;
    FT_Fixed scale;
    FT_Stream stream;
    int position;

    if (!TTF_initialized) {
    	 SDL_SetError("Library not initialized");
    	 return NULL;
    }
    /* Check to make sure we can seek in this stream */
    position = SDL_RWtell(src);
    if (position < 0) {
    	 SDL_SetError("Can't seek in stream");
    	 return NULL;
    }
    font = (TTF_Font*) malloc(sizeof *font);
    if (font == NULL) {
    	 SDL_SetError("Out of memory");
    	 return NULL;
    }
    memset(font, 0, sizeof(*font));
    font->src = src;
    font->freesrc = freesrc;
    stream = (FT_Stream)malloc(sizeof(*stream));
    if (stream == NULL) {
    	 SDL_SetError("Out of memory");
    	 TTF_CloseFont(font);
    	 return NULL;
    }
    memset(stream, 0, sizeof(*stream));
    //stream->memory = library->memory;
    stream->read = RWread;
    stream->descriptor.pointer = src;
    stream->pos = (unsigned long)position;
    SDL_RWseek(src, 0, SEEK_END);
    stream->size = (unsigned long)(SDL_RWtell(src) - position);
    SDL_RWseek(src, position, SEEK_SET);
    font->args.flags = FT_OPEN_STREAM;
    font->args.stream = stream;
    error = FT_Open_Face(library, &font->args, index, &font->face);
    if (error) {
    	 SDL_SetError("Couldn't load font file %d", error);
    	 TTF_CloseFont(font);
    	 return NULL;
    }
    face = font->face;
    /* Make sure that our font face is scalable (global metrics) */
    if (FT_IS_SCALABLE(face)) {
      	 /* Set the character size and use default DPI (72) */
      	 error = FT_Set_Char_Size(font->face, 0, ptsize * 64, 0, 0);
    	 if (error) {
            SDL_SetError("Couldn't set font size", error);
            TTF_CloseFont(font);
            return NULL;
        }
        /* Get the scalable font metrics for this font */
        scale = face->size->metrics.y_scale;
        font->ascent  = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
        font->descent = FT_CEIL(FT_MulFix(face->bbox.yMin, scale));
        font->height  = font->ascent - font->descent + /* baseline */ 1;
        font->lineskip = FT_CEIL(FT_MulFix(face->height, scale));
        font->underline_offset = FT_FLOOR(FT_MulFix(face->underline_position, scale));
        font->underline_height = FT_FLOOR(FT_MulFix(face->underline_thickness, scale));
    } else {
    	 /* Non-scalable font case.  ptsize determines which family
    	  * or series of fonts to grab from the non-scalable format.
    	  * It is not the point size of the font.
    	  * */
    	  if (ptsize >= font->face->num_fixed_sizes) {
    	      ptsize = font->face->num_fixed_sizes - 1;
         }
         font->font_size_family = ptsize;
    	  error = FT_Set_Pixel_Sizes(face, face->available_sizes[ptsize].height, face->available_sizes[ptsize].width);
      	  /* With non-scalale fonts, Freetype2 likes to fill many of the
    	   * font metrics with the value of 0.  The size of the
    	   * non-scalable fonts must be determined differently
    	   * or sometimes cannot be determined.
    	   * */
      	  font->ascent = face->available_sizes[ptsize].height;
      	  font->descent = 0;
      	  font->height = face->available_sizes[ptsize].height;
      	  font->lineskip = FT_CEIL(font->ascent);
      	  font->underline_offset = FT_FLOOR(face->underline_position);
      	  font->underline_height = FT_FLOOR(face->underline_thickness);
    }
    if (font->underline_height < 1) {
    	 font->underline_height = 1;
    }
    /* Set the default font style */
    font->style = TTF_STYLE_NORMAL;
    font->glyph_overhang = face->size->metrics.y_ppem / 10;
    /* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
    font->glyph_italics = 0.207f;
    font->glyph_italics *= font->height;
    return font;
}

static TTF_Font* TTF_OpenFontIndex(const char *file, int ptsize, long index)
{
    SDL_RWops *rw = SDL_RWFromFile(file, "rb");
    if (rw == NULL) {
      	SDL_SetError(SDL_GetError());
     	return NULL;
    }
    return TTF_OpenFontIndexRW(rw, 1, ptsize, index);
}

static TTF_Font* TTF_OpenFont(const char *file, int ptsize)
{
    return TTF_OpenFontIndex(file, ptsize, 0);
}

static void Flush_Glyph(c_glyph* glyph)
{
    glyph->stored = 0;
    glyph->index = 0;
    if (glyph->bitmap.buffer) {
    	free(glyph->bitmap.buffer);
    	glyph->bitmap.buffer = 0;
    }
    if (glyph->pixmap.buffer) {
    	free(glyph->pixmap.buffer);
    	glyph->pixmap.buffer = 0;
    }
    glyph->cached = 0;
}

static void Flush_Cache(TTF_Font* font)
{
    int i;
    int size = sizeof(font->cache) / sizeof(font->cache[0]);

    for (i = 0; i < size; ++i) {
    	if (font->cache[i].cached) {
    	    Flush_Glyph(&font->cache[i]);
    	}
    }
    if (font->scratch.cached) {
     	 Flush_Glyph(&font->scratch);
    }
}

static FT_Error Load_Glyph(TTF_Font* font, Uint16 ch, c_glyph* cached, int want)
{
    FT_Face face;
    FT_Error error;
    FT_GlyphSlot glyph;
    FT_Glyph_Metrics* metrics;
    FT_Outline* outline;

    if (!font || !font->face) {
    	 return FT_Err_Invalid_Handle;
    }
    face = font->face;
    /* Load the glyph */
    if (!cached->index) {
     	 cached->index = FT_Get_Char_Index(face, ch);
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
    	if (FT_IS_SCALABLE(face)) {
    	    /* Get the bounding box */
    	    cached->minx = FT_FLOOR(metrics->horiBearingX);
    	    cached->maxx = cached->minx + FT_CEIL(metrics->width);
    	    cached->maxy = FT_FLOOR(metrics->horiBearingY);
    	    cached->miny = cached->maxy - FT_CEIL(metrics->height);
    	    cached->yoffset = font->ascent - cached->maxy;
    	    cached->advance = FT_CEIL(metrics->horiAdvance);
    	} else {
    	    /* Get the bounding box for non-scalable format.
    	     * Again, freetype2 fills in many of the font metrics
    	     * with the value of 0, so some of the values we
    	     * need must be calculated differently with certain
    	     * assumptions about non-scalable formats.
    	     * */
    	    cached->minx = FT_FLOOR(metrics->horiBearingX);
    	    cached->maxx = cached->minx + FT_CEIL(metrics->horiAdvance);
    	    cached->maxy = FT_FLOOR(metrics->horiBearingY);
    	    cached->miny = cached->maxy - FT_CEIL(face->available_sizes[font->font_size_family].height);
    	    cached->yoffset = 0;
    	    cached->advance = FT_CEIL(metrics->horiAdvance);
    	}
    	/* Adjust for bold and italic text */
    	if (font->style & TTF_STYLE_BOLD) {
    	    cached->maxx += font->glyph_overhang;
    	}
    	if (font->style & TTF_STYLE_ITALIC) {
    	    cached->maxx += (int)ceil(font->glyph_italics);
    	}
    	cached->stored |= CACHED_METRICS;
    }
    if (((want & CACHED_BITMAP) && !(cached->stored & CACHED_BITMAP)) ||
        ((want & CACHED_PIXMAP) && !(cached->stored & CACHED_PIXMAP))) {
    	int i, mono = (want & CACHED_BITMAP);
    	FT_Bitmap *src, *dst;
    	/* Handle the italic style */
    	if (font->style & TTF_STYLE_ITALIC) {
    	    FT_Matrix shear;
    	    shear.xx = 1 << 16;
    	    shear.xy = (int) (font->glyph_italics * (1 << 16)) / font->height;
    	    shear.yx = 0;
    	    shear.yy = 1 << 16;
    	    FT_Outline_Transform(outline, &shear);
    	}
    	/* Render the glyph */
    	if (mono) {
    	    error = FT_Render_Glyph(glyph, ft_render_mode_mono);
    	} else {
    	    error = FT_Render_Glyph(glyph, ft_render_mode_normal);
    	}
    	if (error) {
    	    return error;
    	}
    	/* Copy over information to cache */
    	src = &glyph->bitmap;
    	if (mono) {
    	    dst = &cached->bitmap;
    	} else {
    	    dst = &cached->pixmap;
    	}
    	memcpy(dst, src, sizeof(*dst));
    	/* FT_Render_Glyph() and .fon fonts always generate a
    	 * two-color (black and white) glyphslot surface, even
    	 * when rendered in ft_render_mode_normal.  This is probably
    	 * a freetype2 bug because it is inconsistent with the
    	 * freetype2 documentation under FT_Render_Mode section.
    	 * */
    	if (mono || !FT_IS_SCALABLE(face)) {
    	    dst->pitch *= 8;
    	}
    	/* Adjust for bold and italic text */
    	if (font->style & TTF_STYLE_BOLD) {
    	    int bump = font->glyph_overhang;
    	    dst->pitch += bump;
    	    dst->width += bump;
    	}
    	if (font->style & TTF_STYLE_ITALIC) {
    	    int bump = (int)ceil(font->glyph_italics);
    	    dst->pitch += bump;
    	    dst->width += bump;
    	}
    	if (dst->rows != 0) {
    	    dst->buffer = (unsigned char *)malloc(dst->pitch * dst->rows);
    	    if (!dst->buffer) {
    	    	return FT_Err_Out_Of_Memory;
    	    }
    	    memset(dst->buffer, 0, dst->pitch * dst->rows);
    	    for (i = 0; i < src->rows; i++) {
    	    	int soffset = i * src->pitch;
    	    	int doffset = i * dst->pitch;
    	    	if (mono) {
    	    	    unsigned char *srcp = src->buffer + soffset;
    	    	    unsigned char *dstp = dst->buffer + doffset;
    	    	    int j;
    	    	    for (j = 0; j < src->width; j += 8) {
    	    	    	unsigned char ch = *srcp++;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    	ch <<= 1;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    	ch <<= 1;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    	ch <<= 1;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    	ch <<= 1;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    	ch <<= 1;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    	ch <<= 1;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    	ch <<= 1;
    	    	    	*dstp++ = (ch&0x80) >> 7;
    	    	    }
    	    	} else
               if (!FT_IS_SCALABLE(face)) {
    	    	    /* This special case wouldn't
    	    	     * be here if the FT_Render_Glyph()
    	    	     * function wasn't buggy when it tried
    	    	     * to render a .fon font with 256
    	    	     * shades of gray.  Instead, it
    	    	     * returns a black and white surface
    	    	     * and we have to translate it back
    	    	     * to a 256 gray shaded surface. 
    	    	     * */
    	    	    unsigned char *srcp = src->buffer + soffset;
    	    	    unsigned char *dstp = dst->buffer + doffset;
    	    	    unsigned char ch;
    	    	    int j, k;
    	    	    for (j = 0; j < src->width; j += 8) {
    	    	    	ch = *srcp++;
    	    	    	for (k = 0; k < 8; ++k) {
    	    	    	    if ((ch&0x80) >> 7) {
    	    	    	    	*dstp++ = NUM_GRAYS - 1;
    	    	    	    } else {
    	    	    	    	*dstp++ = 0x00;
    	    	    	    }
    	    	    	    ch <<= 1;
    	    	    	}
    	    	    }
    	    	} else {
    	    	    memcpy(dst->buffer+doffset, src->buffer+soffset, src->pitch);
    	    	}
    	    }
    	}
    	/* Handle the bold style */
    	if (font->style & TTF_STYLE_BOLD) {
    	    int row, col, offset, pixel;
    	    Uint8* pixmap;

    	    /* The pixmap is a little hard, we have to add and clamp */
    	    for (row = dst->rows - 1; row >= 0; --row) {
    	    	pixmap = (Uint8*) dst->buffer + row * dst->pitch;
    	    	for (offset=1; offset <= font->glyph_overhang; ++offset) {
    	    	    for (col = dst->width - 1; col > 0; --col) {
    	    	    	pixel = (pixmap[col] + pixmap[col-1]);
    	    	    	if (pixel > NUM_GRAYS - 1) {
    	    	    	    pixel = NUM_GRAYS - 1;
    	    	    	}
    	    	    	pixmap[col] = (Uint8) pixel;
    	    	    }
    	    	}
    	    }
    	}
    	/* Mark that we rendered this format */
    	if (mono) {
    	    cached->stored |= CACHED_BITMAP;
    	} else {
    	    cached->stored |= CACHED_PIXMAP;
    	}
    }
    /* We're done, mark this glyph cached */
    cached->cached = ch;
    return 0;
}

static FT_Error Find_Glyph(TTF_Font* font, Uint16 ch, int want)
{
    int retval = 0;

    if (ch < 256) {
    	 font->current = &font->cache[ch];
    } else {
    	if (font->scratch.cached != ch) {
    	    Flush_Glyph(&font->scratch);
    	}
    	font->current = &font->scratch;
    }
    if ((font->current->stored & want) != want) {
    	retval = Load_Glyph(font, ch, font->current, want);
    }
    return retval;
}

static void TTF_CloseFont(TTF_Font* font)
{
    Flush_Cache(font);
    if (font->face) {
    	 FT_Done_Face(font->face);
    }
    if (font->args.stream) {
    	 free(font->args.stream);
    }
    if (font->freesrc) {
    	 SDL_RWclose(font->src);
    }
    free(font);
}

static Uint16 *LATIN1_to_UNICODE(Uint16 *unicode, const char *text, int len)
{
    for (int i = 0; i < len; ++i) {
    	 unicode[i] = ((const unsigned char *)text)[i];
    }
    unicode[len] = 0;
    return unicode;
}

static Uint16 *UTF8_to_UNICODE(Uint16 *unicode, const char *utf8, int len)
{
    int i, j;
    Uint16 ch;

    for (i = 0, j = 0; i < len; ++i, ++j) {
    	 ch = ((const unsigned char *)utf8)[i];
    	 if (ch >= 0xF0) {
    	     ch = (Uint16)(utf8[i] & 0x07) << 18;
    	     ch |= (Uint16)(utf8[++i] & 0x3F) << 12;
    	     ch |= (Uint16)(utf8[++i] & 0x3F) << 6;
    	     ch |= (Uint16)(utf8[++i] & 0x3F);
    	} else
    	if (ch >= 0xE0) {
    	    ch = (Uint16)(utf8[i] & 0x0F) << 12;
    	    ch |= (Uint16)(utf8[++i] & 0x3F) << 6;
    	    ch |= (Uint16)(utf8[++i] & 0x3F);
    	} else
    	if (ch >= 0xC0) {
    	    ch = (Uint16)(utf8[i] & 0x1F) << 6;
    	    ch |= (Uint16)(utf8[++i] & 0x3F);
    	}
    	unicode[j] = ch;
    }
    unicode[j] = 0;
    return unicode;
}

static int TTF_SizeText(TTF_Font *font, const char *text, int *w, int *h)
{
    Uint16 *unicode_text;
    int unicode_len;
    int status;

    /* Copy the Latin-1 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
    	 SDL_SetError("Out of memory");
    	 return -1;
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    LATIN1_to_UNICODE(unicode_text+1, text, unicode_len);
    /* Render the new text */
    status = TTF_SizeUNICODE(font, unicode_text, w, h);
    /* Free the text buffer and return */
    free(unicode_text);
    return status;
}

static int TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h)
{
    Uint16 *unicode_text;
    int unicode_len;
    int status;

    /* Copy the UTF-8 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
     	 SDL_SetError("Out of memory");
    	 return -1;
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    UTF8_to_UNICODE(unicode_text+1, text, unicode_len);
    /* Render the new text */
    status = TTF_SizeUNICODE(font, unicode_text, w, h);
    /* Free the text buffer and return */
    free(unicode_text);
    return status;
}

static int TTF_SizeUNICODE(TTF_Font *font, const Uint16 *text, int *w, int *h)
{
    int status;
    const Uint16 *ch;
    int swapped;
    int x, z;
    int minx, maxx;
    int miny, maxy;
    c_glyph *glyph;
    FT_Error error;
    FT_Long use_kerning;
    FT_UInt prev_index = 0;

    /* Initialize everything to 0 */
    if (!TTF_initialized) {
    	 SDL_SetError("Library not initialized");
    	 return -1;
    }
    status = 0;
    minx = maxx = 0;
    miny = maxy = 0;
    swapped = TTF_byteswapped;
    /* check kerning */
    use_kerning = FT_HAS_KERNING(font->face);
    /* Load each character and sum it's bounding box */
    x = 0;
    for (ch = text; *ch; ++ch) {
    	Uint16 c = *ch;
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
    	error = Find_Glyph(font, c, CACHED_METRICS);
    	if (error) {
    	    return -1;
    	}
    	glyph = font->current;
    	/* handle kerning */
    	if (use_kerning && prev_index && glyph->index) {
    	    FT_Vector delta;
    	    FT_Get_Kerning(font->face, prev_index, glyph->index, ft_kerning_default, &delta);
    	    x += delta.x >> 6;
    	}
    	z = x + glyph->minx;
    	if (minx > z) {
    	    minx = z;
    	}
    	if (font->style & TTF_STYLE_BOLD) {
    	    x += font->glyph_overhang;
    	}
    	if (glyph->advance > glyph->maxx) {
    	    z = x + glyph->advance;
    	} else {
    	    z = x + glyph->maxx;
    	}
    	if (maxx < z) {
    	    maxx = z;
    	}
    	x += glyph->advance;
    	if (glyph->miny < miny) {
    	    miny = glyph->miny;
    	}
    	if (glyph->maxy > maxy) {
    	    maxy = glyph->maxy;
    	}
    	prev_index = glyph->index;
    }
    /* Fill the bounds rectangle */
    if (w) {
    	*w = (maxx - minx);
    }
    if (h) {
        *h = (maxy - miny);
      	 //*h = font->height;
    }
    return status;
}

/* Convert the Latin-1 text to UNICODE and render it */
static SDL_Surface *TTF_RenderText_Solid(TTF_Font *font, const char *text, SDL_Color fg)
{
    SDL_Surface *textbuf;
    Uint16 *unicode_text;
    int unicode_len;

    /* Copy the Latin-1 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
    	 SDL_SetError("Out of memory");
    	 return(NULL);
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    LATIN1_to_UNICODE(unicode_text+1, text, unicode_len);
    /* Render the new text */
    textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg);
    /* Free the text buffer and return */
    free(unicode_text);
    return(textbuf);
}

/* Convert the UTF-8 text to UNICODE and render it */
static SDL_Surface *TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg)
{
    SDL_Surface *textbuf;
    Uint16 *unicode_text;
    int unicode_len;

    /* Copy the UTF-8 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
    	 SDL_SetError("Out of memory");
    	 return(NULL);
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    UTF8_to_UNICODE(unicode_text, text, unicode_len);
    /* Render the new text */
    textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg);
    /* Free the text buffer and return */
    free(unicode_text);
    return(textbuf);
}

static SDL_Surface *TTF_RenderUNICODE_Solid(TTF_Font *font, const Uint16 *text, SDL_Color fg)
{
    int xstart;
    int width;
    int height;
    SDL_Surface* textbuf;
    SDL_Palette* palette;
    const Uint16* ch;
    Uint8* src;
    Uint8* dst;
    Uint8 *dst_check;
    int swapped;
    int row, col;
    c_glyph *glyph;

    FT_Bitmap *current;
    FT_Error error;
    FT_Long use_kerning;
    FT_UInt prev_index = 0;

    /* Get the dimensions of the text surface */
    if ((TTF_SizeUNICODE(font, text, &width, NULL) < 0) || !width) {
    	 SDL_SetError("Text has zero width");
    	 return NULL;
    }
    height = font->height;
    /* Create the target surface */
    textbuf = SDL_AllocSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
    if (textbuf == NULL) {
    	 return NULL;
    }
    /* Adding bound checking to avoid all kinds of memory corruption errors that may occur. */
    dst_check = (Uint8*)textbuf->pixels + textbuf->pitch * textbuf->h;
    /* Fill the palette with the foreground color */
    palette = textbuf->format->palette;
    palette->colors[0].r = 255 - fg.r;
    palette->colors[0].g = 255 - fg.g;
    palette->colors[0].b = 255 - fg.b;
    palette->colors[1].r = fg.r;
    palette->colors[1].g = fg.g;
    palette->colors[1].b = fg.b;
    SDL_SetColorKey(textbuf, SDL_SRCCOLORKEY, 0);
    /* check kerning */
    use_kerning = FT_HAS_KERNING(font->face);
    /* Load and render each character */
    xstart = 0;
    swapped = TTF_byteswapped;
    for (ch = text; *ch; ++ch) {
    	 Uint16 c = *ch;
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
    	 error = Find_Glyph(font, c, CACHED_METRICS|CACHED_BITMAP);
    	 if (error) {
    	     SDL_FreeSurface(textbuf);
    	     return NULL;
    	 }
    	 glyph = font->current;
    	 current = &glyph->bitmap;
    	 /* Ensure the width of the pixmap is correct. On some cases,
    	  * freetype may report a larger pixmap than possible.*/
    	 width = current->width;
    	 if (width > glyph->maxx - glyph->minx) {
    	     width = glyph->maxx - glyph->minx;
    	 }
    	 /* do kerning, if possible AC-Patch */
    	 if (use_kerning && prev_index && glyph->index) {
    	     FT_Vector delta;
    	     FT_Get_Kerning(font->face, prev_index, glyph->index, ft_kerning_default, &delta);
    	     xstart += delta.x >> 6;
    	 }
    	 /* Compensate for wrap around bug with negative minx's */
    	 if ((ch == text) && (glyph->minx < 0)) {
    	     xstart -= glyph->minx;
    	 }
    	 for (row = 0; row < current->rows; ++row) {
    	     /* Make sure we don't go either over, or under the limit */
    	     if (row+glyph->yoffset < 0) {
    	     	continue;
    	     }
    	     if (row+glyph->yoffset >= textbuf->h) {
    	     	continue;
    	     }
    	     dst = (Uint8*) textbuf->pixels + (row+glyph->yoffset) * textbuf->pitch + xstart + glyph->minx;
    	     src = current->buffer + row * current->pitch;
    	     for (col=width; col>0 && dst < dst_check; --col) {
    	     	*dst++ |= *src++;
    	     }
    	 }
    	 xstart += glyph->advance;
    	 if (font->style & TTF_STYLE_BOLD) {
    	     xstart += font->glyph_overhang;
    	 }
    	 prev_index = glyph->index;
    }
    /* Handle the underline style */
    if (font->style & TTF_STYLE_UNDERLINE) {
    	 row = font->ascent - font->underline_offset - 1;
    	 if (row >= textbuf->h) {
    	     row = (textbuf->h-1) - font->underline_height;
    	 }
    	 dst = (Uint8 *)textbuf->pixels + row * textbuf->pitch;
    	 for (row = font->underline_height; row > 0; --row) {
    	     /* 1 because 0 is the bg color */
    	     memset(dst, 1, textbuf->w);
    	     dst += textbuf->pitch;
    	}
    }
    return textbuf;
}

/* Convert the Latin-1 text to UNICODE and render it
*/
static SDL_Surface *TTF_RenderText_Shaded(TTF_Font *font, const char *text, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface *textbuf;
    Uint16 *unicode_text;
    int unicode_len;

    /* Copy the Latin-1 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
    	 SDL_SetError("Out of memory");
    	 return(NULL);
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    LATIN1_to_UNICODE(unicode_text+1, text, unicode_len);
    /* Render the new text */
    textbuf = TTF_RenderUNICODE_Shaded(font, unicode_text, fg, bg);
    /* Free the text buffer and return */
    free(unicode_text);
    return(textbuf);
}

/* Convert the UTF-8 text to UNICODE and render it
*/
static SDL_Surface *TTF_RenderUTF8_Shaded(TTF_Font *font, const char *text, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface *textbuf;
    Uint16 *unicode_text;
    int unicode_len;

    /* Copy the UTF-8 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
    	 SDL_SetError("Out of memory");
    	 return(NULL);
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    UTF8_to_UNICODE(unicode_text+1, text, unicode_len);
    /* Render the new text */
    textbuf = TTF_RenderUNICODE_Shaded(font, unicode_text, fg, bg);
    /* Free the text buffer and return */
    free(unicode_text);
    return(textbuf);
}

static SDL_Surface* TTF_RenderUNICODE_Shaded(TTF_Font* font, const Uint16* text, SDL_Color fg, SDL_Color bg)
{
    int xstart;
    int width;
    int height;
    SDL_Surface* textbuf;
    SDL_Palette* palette;
    int index;
    int rdiff;
    int gdiff;
    int bdiff;
    const Uint16* ch;
    Uint8* src;
    Uint8* dst;
    Uint8* dst_check;
    int swapped;
    int row, col;
    FT_Bitmap* current;
    c_glyph *glyph;
    FT_Error error;
    FT_Long use_kerning;
    FT_UInt prev_index = 0;

    /* Get the dimensions of the text surface */
    if ((TTF_SizeUNICODE(font, text, &width, NULL) < 0) || !width) {
    	 SDL_SetError("Text has zero width");
    	 return NULL;
    }
    height = font->height;
    /* Create the target surface */
    textbuf = SDL_AllocSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
    if (textbuf == NULL) {
    	return NULL;
    }
    /* Adding bound checking to avoid all kinds of memory corruption errors that may occur. */
    dst_check = (Uint8*)textbuf->pixels + textbuf->pitch * textbuf->h;
    /* Fill the palette with NUM_GRAYS levels of shading from bg to fg */
    palette = textbuf->format->palette;
    rdiff = fg.r - bg.r;
    gdiff = fg.g - bg.g;
    bdiff = fg.b - bg.b;
    for (index = 0; index < NUM_GRAYS; ++index) {
    	 palette->colors[index].r = bg.r + (index*rdiff) / (NUM_GRAYS-1);
    	 palette->colors[index].g = bg.g + (index*gdiff) / (NUM_GRAYS-1);
    	 palette->colors[index].b = bg.b + (index*bdiff) / (NUM_GRAYS-1);
    }
    /* check kerning */
    use_kerning = FT_HAS_KERNING(font->face);
    /* Load and render each character */
    xstart = 0;
    swapped = TTF_byteswapped;
    for (ch = text; *ch; ++ch) {
    	 Uint16 c = *ch;
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
    	error = Find_Glyph(font, c, CACHED_METRICS|CACHED_PIXMAP);
    	if (error) {
    	    SDL_FreeSurface(textbuf);
    	    return NULL;
    	}
    	glyph = font->current;
    	/* Ensure the width of the pixmap is correct. On some cases,
    	 * freetype may report a larger pixmap than possible.*/
    	width = glyph->pixmap.width;
    	if (width > glyph->maxx - glyph->minx) {
    	    width = glyph->maxx - glyph->minx;
    	}
    	/* do kerning, if possible AC-Patch */
    	if (use_kerning && prev_index && glyph->index) {
    	    FT_Vector delta;
    	    FT_Get_Kerning(font->face, prev_index, glyph->index, ft_kerning_default, &delta);
    	    xstart += delta.x >> 6;
    	}
    	/* Compensate for the wrap around with negative minx's */
    	if ((ch == text) && (glyph->minx < 0)) {
    	    xstart -= glyph->minx;
    	}
    	
    	current = &glyph->pixmap;
    	for (row = 0; row < current->rows; ++row) {
    	    /* Make sure we don't go either over, or under the limit */
    	    if (row+glyph->yoffset < 0) {
    	    	continue;
    	    }
    	    if (row+glyph->yoffset >= textbuf->h) {
    	    	continue;
    	    }
    	    dst = (Uint8*) textbuf->pixels + (row+glyph->yoffset) * textbuf->pitch + xstart + glyph->minx;
    	    src = current->buffer + row * current->pitch;
    	    for (col=width; col>0 && dst < dst_check; --col) {
    	    	*dst++ |= *src++;
    	    }
    	}
    	xstart += glyph->advance;
    	if (font->style & TTF_STYLE_BOLD) {
    	    xstart += font->glyph_overhang;
    	}
    	prev_index = glyph->index;
    }
    /* Handle the underline style */
    if (font->style & TTF_STYLE_UNDERLINE) {
    	row = font->ascent - font->underline_offset - 1;
    	if (row >= textbuf->h) {
    	    row = (textbuf->h-1) - font->underline_height;
    	}
    	dst = (Uint8 *)textbuf->pixels + row * textbuf->pitch;
    	for (row = font->underline_height; row > 0; --row) {
    	    memset(dst, NUM_GRAYS - 1, textbuf->w);
    	    dst += textbuf->pitch;
    	}
    }
    return textbuf;
}

/* Convert the Latin-1 text to UNICODE and render it
*/
static SDL_Surface *TTF_RenderText_Blended(TTF_Font *font, const char *text, SDL_Color fg)
{
    SDL_Surface *textbuf;
    Uint16 *unicode_text;
    int unicode_len;

    /* Copy the Latin-1 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
    	 SDL_SetError("Out of memory");
    	 return(NULL);
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    LATIN1_to_UNICODE(unicode_text+1, text, unicode_len);
    /* Render the new text */
    textbuf = TTF_RenderUNICODE_Blended(font, unicode_text, fg);
    /* Free the text buffer and return */
    free(unicode_text);
    return(textbuf);
}

/* Convert the UTF-8 text to UNICODE and render it
*/
static SDL_Surface *TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg)
{
    SDL_Surface *textbuf;
    Uint16 *unicode_text;
    int unicode_len;

    /* Copy the UTF-8 text to a UNICODE text buffer */
    unicode_len = strlen(text);
    unicode_text = (Uint16 *)malloc((1+unicode_len+1)*(sizeof *unicode_text));
    if (unicode_text == NULL) {
    	 SDL_SetError("Out of memory");
    	 return(NULL);
    }
    *unicode_text = UNICODE_BOM_NATIVE;
    UTF8_to_UNICODE(unicode_text+1, text, unicode_len);
    /* Render the new text */
    textbuf = TTF_RenderUNICODE_Blended(font, unicode_text, fg);
    /* Free the text buffer and return */
    free(unicode_text);
    return(textbuf);
}

static SDL_Surface *TTF_RenderUNICODE_Blended(TTF_Font *font, const Uint16 *text, SDL_Color fg)
{
    int xstart;
    int width, height;
    SDL_Surface *textbuf;
    Uint32 alpha;
    Uint32 pixel;
    const Uint16 *ch;
    Uint8 *src;
    Uint32 *dst;
    Uint32 *dst_check;
    int swapped;
    int row, col;
    c_glyph *glyph;
    FT_Error error;
    FT_Long use_kerning;
    FT_UInt prev_index = 0;

    /* Get the dimensions of the text surface */
    if ((TTF_SizeUNICODE(font, text, &width, NULL) < 0) || !width) {
    	SDL_SetError("Text has zero width");
    	return(NULL);
    }
    height = font->height;
    textbuf = SDL_AllocSurface(SDL_SWSURFACE, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (textbuf == NULL) {
    	return(NULL);
    }
    /* Adding bound checking to avoid all kinds of memory corruption errors that may occur. */
    dst_check = (Uint32*)textbuf->pixels + textbuf->pitch/4 * textbuf->h;
    /* check kerning */
    use_kerning = FT_HAS_KERNING(font->face);
    /* Load and render each character */
    xstart = 0;
    swapped = TTF_byteswapped;
    pixel = (fg.r<<16)|(fg.g<<8)|fg.b;
    for (ch=text; *ch; ++ch) {
    	 Uint16 c = *ch;
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
    	 error = Find_Glyph(font, c, CACHED_METRICS|CACHED_PIXMAP);
    	 if (error) {
    	     SDL_FreeSurface(textbuf);
    	     return NULL;
    	 }
    	 glyph = font->current;
    	 /* Ensure the width of the pixmap is correct. On some cases,
    	  * freetype may report a larger pixmap than possible.*/
    	 width = glyph->pixmap.width;
    	 if (width > glyph->maxx - glyph->minx) {
    	     width = glyph->maxx - glyph->minx;
    	 }
    	 /* do kerning, if possible AC-Patch */
    	 if (use_kerning && prev_index && glyph->index) {
    	     FT_Vector delta;
    	     FT_Get_Kerning(font->face, prev_index, glyph->index, ft_kerning_default, &delta);
    	     xstart += delta.x >> 6;
    	 }
    	 /* Compensate for the wrap around bug with negative minx's */
    	 if ((ch == text) && (glyph->minx < 0)) {
    	     xstart -= glyph->minx;
    	 }
    	 for (row = 0; row < glyph->pixmap.rows; ++row) {
    	     /* Make sure we don't go either over, or under the
    	      * limit */
    	     if (row+glyph->yoffset < 0) {
    	     	continue;
    	     }
    	     if (row+glyph->yoffset >= textbuf->h) {
    	     	continue;
    	     }
    	     dst = (Uint32*) textbuf->pixels + (row+glyph->yoffset) * textbuf->pitch/4 +	xstart + glyph->minx;
    	     /* Added code to adjust src pointer for pixmaps to account for pitch. */
    	     src = (Uint8*) (glyph->pixmap.buffer + glyph->pixmap.pitch * row);
    	     for (col = width; col>0 && dst < dst_check; --col) {
    	     	alpha = *src++;
    	     	*dst++ |= pixel | (alpha << 24);
    	     }
    	 }
    	 xstart += glyph->advance;
    	 if (font->style & TTF_STYLE_BOLD) {
    	     xstart += font->glyph_overhang;
    	 }
    	 prev_index = glyph->index;
    }
    /* Handle the underline style */
    if (font->style & TTF_STYLE_UNDERLINE) {
    	 row = font->ascent - font->underline_offset - 1;
    	 if (row >= textbuf->h) {
    	     row = (textbuf->h-1) - font->underline_height;
    	 }
     	 dst = (Uint32 *)textbuf->pixels + row * textbuf->pitch/4;
     	 pixel |= 0xFF000000; /* Amask */
    	 for (row = font->underline_height; row > 0; --row) {
    	     for (col=0; col < textbuf->w; ++col) {
    	       	  dst[col] = pixel;
    	     }
    	     dst += textbuf->pitch/4;
    	}
    }
    return(textbuf);
}

int TTF_Init(void)
{
    int status = 0;

    if (!TTF_initialized) {
    	FT_Error error = FT_Init_FreeType(&library);
    	if (error) {
    	    SDL_SetError("Couldn't init FreeType engine %d", error);
    	    status = -1;
    	}
    }
    if (status == 0) {
        ++TTF_initialized;
    }
    return status;
}

void TTF_Quit(void)
{
    if (TTF_initialized) {
        if (--TTF_initialized == 0) {
    	     FT_Done_FreeType(library);
        }
    }
}

GUI_Font::GUI_Font(const char *fn, int size):Object(fn)
{
    type = TYPE_FONT;
    ttf = TTF_OpenFont(fn, size);
    if (ttf == NULL) {
        systemObject->Log(0, "TTF_OpenFont failed name=%s size=%d", fn, size);
    }
}

GUI_Font::~GUI_Font(void)
{
    if (ttf) {
        TTF_CloseFont(ttf);
    }
}

GUI_Surface *GUI_Font::RenderFast(const char *s, SDL_Color fg)
{
    if (!ttf || !s || !*s) {
        return 0;
    }
    return new GUI_Surface("text", TTF_RenderText_Solid(ttf, s, fg));
}

GUI_Surface *GUI_Font::RenderQuality(const char *s, SDL_Color fg)
{
    if (!ttf || !s || !*s) {
        return 0;
    }
    return new GUI_Surface("text", TTF_RenderText_Blended(ttf, s, fg));
}

GUI_Surface *GUI_Font::RenderColor(const char *s, SDL_Color *fg)
{
    if (!ttf || !s || !*s) {
        return 0;
    }

    if (!strstr(s, "^#")) {
        return RenderQuality(s, *fg);
    }

    int x = 0, w = 0, h = 0;
    list < SDL_Surface * > slist;
    char *end, *text = tlsStr(1, s);

    if (!strncmp(text, "^#", 2)) {
        *fg = strToColor(text + 1);
        text += 8;
    }
    while (text && *text) {
        if ((end = strstr(text, "^#"))) {
            *end++ = 0;
        }
        SDL_Surface *src = TTF_RenderText_Solid(ttf, text, *fg);
        if (src) {
            w += src->w;
            h = MAX(h, src->h);
            slist.push_back(src);
        }
        if (end) {
            *fg = strToColor(end);
            end += 7;
        }
        text = end;
    }
    SDL_Rect r;
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, RMask, GMask, BMask, AMask);
    while (slist.size() > 0) {
          SDL_Surface *src = slist.front();
          slist.pop_front();
          r.x = x;
          r.y = 0;
          SDL_BlitSurface(src, NULL, surface, &r);
          x += src->w;
          SDL_FreeSurface(src);
    }
    return new GUI_Surface(name, surface);
}

GUI_Surface **GUI_Font::RenderColorList(const char *s, SDL_Color *fg)
{
    if (!ttf || !s || !*s) {
        return 0;
    }
    int ssize = 2, spos = 0;
    GUI_Surface **surfaces = (GUI_Surface**)calloc(sizeof(GUI_Surface*), ssize);

    if (!strstr(s, "^#")) {
        surfaces[0] = RenderQuality(s, *fg);
        return surfaces;
    }

    int x = 0, w = 0, h = 0;
    char *end, *text = tlsStr(1, s);

    if (!strncmp(text, "^#", 2)) {
        *fg = strToColor(text + 1);
        text += 8;
    }
    while (text && *text) {
        if ((end = strstr(text, "^#"))) {
            *end++ = 0;
        }
        SDL_Surface *src = TTF_RenderText_Blended(ttf, text, *fg);
        if (src) {
            w += src->w;
            h = MAX(h, src->h);
            if (spos + 1 >= ssize) {
                ssize += 2;
                surfaces = (GUI_Surface**)realloc(surfaces, sizeof(GUI_Surface*) * ssize);
            }
            surfaces[spos++] = new GUI_Surface(name, src);
        }
        if (end) {
            *fg = strToColor(end);
            end += 7;
        }
        text = end;
    }
    surfaces[spos] = 0;
    return surfaces;
}

SDL_Rect GUI_Font::GetTextSize(const char *s)
{
    SDL_Rect r = { 0, 0, 0, 0 };
    if (!ttf || !s || !*s) {
        return r;
    }
    int w, h;
    if (!strstr(s, "^#")) {
        if (!TTF_SizeText(ttf, s, &w, &h)) {
            r.w = w;
            r.h = h;
        }
        return r;
    }
    char *end, *text = tlsStr(1, s);
    if (!strncmp(text, "^#", 2)) {
        text += 8;
    }
    while (text && *text) {
        if ((end = strstr(text, "^#"))) {
            *end++ = 0;
        }
        if (!TTF_SizeText(ttf, text, &w, &h)) {
            r.w += w;
            r.h = MAX(h, r.h);
        }
        if (end) {
            end += 7;
        }
        text = end;
    }
    return r;
}

int GUI_Font::GetTextWidth(const char *s)
{
    SDL_Rect r = GetTextSize(s);
    return r.w;
}

