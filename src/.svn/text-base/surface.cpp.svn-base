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
 * This object incorporated SDL_image/SDL_gfx libraries, thanks to the authors:
 *
 *   Sam Lantinga slouken@libsdl.org
 *   A. Schiffler
 *
 */

#include "lmbox.h"

#define VALUE_LIMIT	0.001

typedef struct tColorY {
   Uint8 y;
} tColorY;

static SDL_Surface *IMG_LoadTyped_RW(SDL_RWops *src, int freesrc, char *type);

GUI_Surface::GUI_Surface(GUI_Surface * image):Object(image->GetName())
{
    type = TYPE_SURFACE;
    surface = zoomSurface(image->GetSurface(), 1, 1, SMOOTHING_ON);
}

GUI_Surface::GUI_Surface(GUI_Surface * image, int w, int h):Object(image->GetName())
{
    type = TYPE_SURFACE;
    float zw = (float) w / image->GetWidth();
    float zh = (float) h / image->GetHeight();
    surface = zoomSurface(image->GetSurface(), zw, zh, SMOOTHING_ON);
}

GUI_Surface::GUI_Surface(const char *aname, SDL_Surface * image):Object(aname)
{
    type = TYPE_SURFACE;
    surface = image;
}

GUI_Surface::GUI_Surface(const char *fn):Object(fn)
{
    surface = LoadImage(fn);
    if (surface == NULL) {
        systemObject->Log(0, "GUI_Surface: failed to load image %s", fn);
    }
}

GUI_Surface::GUI_Surface(const char *aname, int f, int w, int h, int d, int rm, int bm, int gm, int am):Object(aname)
{
    surface = SDL_CreateRGBSurface(f, w, h, d, rm, bm, gm, am);
    if (surface == NULL) {
        systemObject->Log(0, "GUI_Surface: %s failed to allocate surface %dx%dx%dx%d", aname, f, w, h, d);
    }
}

GUI_Surface::~GUI_Surface(void)
{
    if (surface) {
        SDL_FreeSurface(surface);
    }
}

void GUI_Surface::Blit(SDL_Rect * src_r, GUI_Surface * dst, SDL_Rect * dst_r)
{
    if (surface) {
        SDL_BlitSurface(surface, src_r, dst->surface, dst_r);
    }
}

void GUI_Surface::UpdateRect(int x, int y, int w, int h)
{
    if (surface) {
        SDL_UpdateRect(surface, x, y, w, h);
    }
}

void GUI_Surface::UpdateRects(int n, SDL_Rect * rects)
{
    if (surface) {
        SDL_UpdateRects(surface, n, rects);
    }
}

void GUI_Surface::Fill(const SDL_Rect * r, SDL_Color c)
{
    if (surface) {
        SDL_FillRect(surface, (SDL_Rect *)r, MapRGB(c.r, c.g, c.b));
    }
}

void GUI_Surface::Fill(const SDL_Rect * r, Uint32 c)
{
    if (surface) {
        SDL_FillRect(surface, (SDL_Rect *)r, c);
    }
}

SDL_Rect GUI_Surface::GetArea(void)
{
    SDL_Rect r = { 0, 0, 0, 0 };
    if (surface) {
        r.w = surface->w;
        r.h = surface->h;
    }
    return r;
}

int GUI_Surface::GetWidth(void)
{
    return surface ? surface->w : 0;
}

int GUI_Surface::GetHeight(void)
{
    return surface ? surface->h : 0;
}

SDL_Surface *GUI_Surface::GetSurface(void)
{
    return surface;
}

void GUI_Surface::SetSurface(SDL_Surface * image)
{
    if (surface) {
        SDL_FreeSurface(surface);
    }
    surface = image;
}

Uint32 GUI_Surface::MapRGB(int r, int g, int b)
{
    if (!surface) {
        return 0;
    }
    return SDL_MapRGB(surface->format, r, g, b);
}

Uint32 GUI_Surface::MapRGBA(int r, int g, int b, int a)
{
    if (!surface) {
        return 0;
    }
    return SDL_MapRGBA(surface->format, r, g, b, a);
}

int GUI_Surface::IsDoubleBuffered(void)
{
    return (surface && surface->flags & SDL_DOUBLEBUF) != 0;
}

int GUI_Surface::IsHardware(void)
{
    return (surface && surface->flags & SDL_HWSURFACE) != 0;
}

void GUI_Surface::Flip(void)
{
    if (surface) {
        SDL_Flip(surface);
    }
}

void GUI_Surface::DisplayFormat(void)
{
    if (!surface) {
        return;
    }
    SDL_Surface *temp = SDL_DisplayFormat(surface);
    if (!temp) {
        systemObject->Log(0, "DisplayFormat: failed to format surface for display: %d", SDL_GetError());
        return;
    }
    SDL_FreeSurface(surface);
    surface = temp;
}

void GUI_Surface::SetAlpha(Uint32 flag, Uint8 alpha)
{
    if (surface) {
        SDL_SetAlpha(surface, flag, alpha);
    }
}

void GUI_Surface::SetColorKey(Uint32 c)
{
    if (surface) {
        if (SDL_SetColorKey(surface, SDL_RLEACCEL | SDL_SRCCOLORKEY, c) < 0) {
            systemObject->Log(0, "SetColorKey: failed to set color key for surface: %d", SDL_GetError());
        }
    }
}

void GUI_Surface::SaveBMP(const char *filename)
{
    if (surface) {
        SDL_SaveBMP(surface, filename);
    }
}

SDL_Surface *GUI_Surface::LoadImage(const char *file)
{
    SDL_RWops *src = SDL_RWFromFile(file, "rb");
    if (!src) {
        // The error message has been set in SDL_RWFromFile
        return NULL;
    }
    char *ext = strrchr(file, '.');
    if (ext) {
        ext++;
    }
    return IMG_LoadTyped_RW(src, 1, ext);
}

// 32bit Zoomer with optional anti-aliasing by bilinear interpolation.
// Zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.
int GUI_Surface::zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy, int smooth)
{
    int dgap, x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep;
    SDL_Color *c00, *c01, *c10, *c11;
    SDL_Color *sp, *csp, *dp;

    if (smooth) {
	// For interpolation: assume source dimension is one pixel
	// smaller to avoid overflow on right and bottom edge.
	sx = (int) (65536.0 * (float) (src->w - 1) / (float) dst->w);
	sy = (int) (65536.0 * (float) (src->h - 1) / (float) dst->h);
    } else {
	sx = (int) (65536.0 * (float) src->w / (float) dst->w);
	sy = (int) (65536.0 * (float) src->h / (float) dst->h);
    }
    if ((sax = (int *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
	return (-1);
    }
    if ((say = (int *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
	free(sax);
	return (-1);
    }

    // Precalculate row increments
    sp = csp = (SDL_Color *) src->pixels;
    dp = (SDL_Color *) dst->pixels;
    if (flipx) {
        csp += (src->w-1);
    }
    if (flipy) {
        csp  = (SDL_Color*)( (Uint8*)csp + src->pitch*(src->h-1) );
    }
    csx = 0;
    csax = sax;
    for (x = 0; x <= dst->w; x++) {
	*csax = csx;
	csax++;
	csx &= 0xffff;
	csx += sx;
    }
    csy = 0;
    csay = say;
    for (y = 0; y <= dst->h; y++) {
	*csay = csy;
	csay++;
	csy &= 0xffff;
	csy += sy;
    }
    dgap = dst->pitch - dst->w * 4;
    // Switch between interpolating and non-interpolating code
    if (smooth) {
	// Interpolating Zoom
	// Scan destination
	csay = say;
	for (y = 0; y < dst->h; y++) {
	    // Setup color source pointers
	    c00 = csp;
	    c01 = csp;
	    c01++;
	    c10 = (SDL_Color *) ((Uint8 *) csp + src->pitch);
	    c11 = c10;
	    c11++;
	    csax = sax;
	    for (x = 0; x < dst->w; x++) {
		// Interpolate colors
		ex = (*csax & 0xffff);
		ey = (*csay & 0xffff);
		t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
		t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
		dp->r = (((t2 - t1) * ey) >> 16) + t1;
		t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
		t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
		dp->g = (((t2 - t1) * ey) >> 16) + t1;
		t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
		t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
		dp->b = (((t2 - t1) * ey) >> 16) + t1;
		t1 = ((((c01->unused - c00->unused) * ex) >> 16) + c00->unused) & 0xff;
		t2 = ((((c11->unused - c10->unused) * ex) >> 16) + c10->unused) & 0xff;
		dp->unused = (((t2 - t1) * ey) >> 16) + t1;

		// Advance source pointers
		csax++;
		sstep = (*csax >> 16);
		c00 += sstep;
		c01 += sstep;
		c10 += sstep;
		c11 += sstep;
		dp++;
	    }
	    csay++;
	    csp = (SDL_Color *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
	    // Advance destination pointers
	    dp = (SDL_Color *) ((Uint8 *) dp + dgap);
	}
    } else {
	// Non-Interpolating Zoom
	csay = say;
	for (y = 0; y < dst->h; y++) {
	    sp = csp;
	    csax = sax;
	    for (x = 0; x < dst->w; x++) {
		// Draw
		*dp = *sp;
		// Advance source pointers
		csax++;
		sstep = (*csax >> 16);
		if (flipx) sstep = -sstep;
		sp += sstep;
		// Advance destination pointer
		dp++;
	    }
	    // Advance source pointer
	    csay++;
	    sstep = (*csay >> 16) * src->pitch;
	    if (flipy) sstep = -sstep;
	    csp = (SDL_Color *) ((Uint8 *) csp + sstep);
	    // Advance destination pointers
	    dp = (SDL_Color *) ((Uint8 *) dp + dgap);
	}
    }
    free(sax);
    free(say);
    return (0);
}

// 8bit Zoomer without smoothing.
// Zoomes 8bit palette/Y 'src' surface to 'dst' surface.
int GUI_Surface::zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy)
{
    Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
    Uint8 *sp, *dp, *csp;
    int dgap;

    sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
    sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);
    if ((sax = (Uint32 *) malloc(dst->w * sizeof(Uint32))) == NULL) {
	return (-1);
    }
    if ((say = (Uint32 *) malloc(dst->h * sizeof(Uint32))) == NULL) {
	if (sax != NULL) {
	    free(sax);
	}
	return (-1);
    }
    // Precalculate row increments
    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
	csx += sx;
	*csax = (csx >> 16);
	csx &= 0xffff;
	csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
	csy += sy;
	*csay = (csy >> 16);
	csy &= 0xffff;
	csay++;
    }
    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
	csx += (*csax);
	csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
	csy += (*csay);
	csay++;
    }
    // Pointer setup
    sp = csp = (Uint8 *) src->pixels;
    dp = (Uint8 *) dst->pixels;
    dgap = dst->pitch - dst->w;
    // Draw
    csay = say;
    for (y = 0; y < dst->h; y++) {
	csax = sax;
	sp = csp;
	for (x = 0; x < dst->w; x++) {
	    *dp = *sp;
	    sp += (*csax);
	    csax++;
	    dp++;
	}
	// Advance source pointer (for row)
	csp += ((*csay) * src->pitch);
	csay++;
	// Advance destination pointers
	dp += dgap;
    }
    free(sax);
    free(say);
    return (0);
}

// 32bit Rotozoomer with optional anti-aliasing by bilinear interpolation.
// Rotates and zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.
void GUI_Surface::transformSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos, int flipx, int flipy, int smooth)
{
    int gap, x, y, t1, t2, dx, dy, xd, yd, sdx, sdy, ax, ay, ex, ey, sw, sh;
    SDL_Color c00, c01, c10, c11;
    SDL_Color *pc, *sp, *spb;

    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = (SDL_Color*)dst->pixels;
    gap = dst->pitch - dst->w * 4;
    // Switch between interpolating and non-interpolating code
    if (smooth) {
	for (y = 0; y < dst->h; y++) {
	    dy = cy - y;
	    sdx = (ax + (isin * dy)) + xd;
	    sdy = (ay - (icos * dy)) + yd;
	    for (x = 0; x < dst->w; x++) {
		dx = (sdx >> 16);
		dy = (sdy >> 16);
		if ((dx >= -1) && (dy >= -1) && (dx < src->w) && (dy < src->h)) {
		    if ((dx >= 0) && (dy >= 0) && (dx < sw) && (dy < sh)) {
			sp = (SDL_Color *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			sp += 1;
			c01 = *sp;
			sp = (SDL_Color *) ((Uint8 *) sp + src->pitch);
			sp -= 1;
			c10 = *sp;
			sp += 1;
			c11 = *sp;
		    } else
                    if ((dx == sw) && (dy == sh)) {
			sp = (SDL_Color *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else
                    if ((dx == -1) && (dy == -1)) {
			sp = (SDL_Color *) (src->pixels);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else
                    if ((dx == -1) && (dy == sh)) {
			sp = (SDL_Color *) (src->pixels);
			sp = (SDL_Color *) ((Uint8 *) src->pixels + src->pitch * dy);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else
                    if ((dx == sw) && (dy == -1)) {
			sp = (SDL_Color *) (src->pixels);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else
                    if (dx == -1) {
			sp = (SDL_Color *) ((Uint8 *) src->pixels + src->pitch * dy);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			sp = (SDL_Color *) ((Uint8 *) sp + src->pitch);
			c11 = *sp;
		    } else
                    if (dy == -1) {
			sp = (SDL_Color *) (src->pixels);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			sp += 1;
			c11 = *sp;
		    } else
                    if (dx == sw) {
			sp = (SDL_Color *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			sp = (SDL_Color *) ((Uint8 *) sp + src->pitch);
			c10 = *sp;
			c11 = *sp;
		    } else
                    if (dy == sh) {
			sp = (SDL_Color *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			sp += 1;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    }
		    // Interpolate colors
		    ex = (sdx & 0xffff);
		    ey = (sdy & 0xffff);
		    t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
		    t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
		    pc->r = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
		    t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
		    pc->g = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
		    t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
		    pc->b = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.unused - c00.unused) * ex) >> 16) + c00.unused) & 0xff;
		    t2 = ((((c11.unused - c10.unused) * ex) >> 16) + c10.unused) & 0xff;
		    pc->unused = (((t2 - t1) * ey) >> 16) + t1;
		}
		sdx += icos;
		sdy += isin;
		pc++;
	    }
	    pc = (SDL_Color *) ((Uint8 *) pc + gap);
	}
    } else {
	for (y = 0; y < dst->h; y++) {
	    dy = cy - y;
	    sdx = (ax + (isin * dy)) + xd;
	    sdy = (ay - (icos * dy)) + yd;
	    for (x = 0; x < dst->w; x++) {
		dx = (short) (sdx >> 16);
		dy = (short) (sdy >> 16);
		if (flipx) dx = (src->w-1)-dx;
		if (flipy) dy = (src->h-1)-dy;
		if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
		    sp = (SDL_Color *) ((Uint8 *) src->pixels + src->pitch * dy);
		    sp += dx;
		    *pc = *sp;
		}
		sdx += icos;
		sdy += isin;
		pc++;
	    }
	    pc = (SDL_Color *) ((Uint8 *) pc + gap);
	}
    }
}

// 8bit Rotozoomer without smoothing
// Rotates and zoomes 8bit palette/Y 'src' surface to 'dst' surface.
void GUI_Surface::transformSurfaceY(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos)
{
    int gap, x, y, dx, dy, xd, yd, sdx, sdy, ax, ay, sw, sh;
    tColorY *pc, *sp;

    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = (tColorY*)dst->pixels;
    gap = dst->pitch - dst->w;
    // Clear surface to colorkey
    memset(pc, (unsigned char) (src->format->colorkey & 0xff), dst->pitch * dst->h);
    // Iterate through destination surface
    for (y = 0; y < dst->h; y++) {
	dy = cy - y;
	sdx = (ax + (isin * dy)) + xd;
	sdy = (ay - (icos * dy)) + yd;
	for (x = 0; x < dst->w; x++) {
	    dx = (short) (sdx >> 16);
	    dy = (short) (sdy >> 16);
	    if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
		sp = (tColorY *) (src->pixels);
		sp += (src->pitch * dy + dx);
		*pc = *sp;
	    }
	    sdx += icos;
	    sdy += isin;
	    pc++;
	}
	pc += gap;
    }
}

// Rotates and zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
// 'angle' is the rotation in degrees. 'zoom' a scaling factor. If 'smooth' is 1
// then the destination 32bit surface is anti-aliased. If the surface is not 8bit
// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

static void rotozoomSurfaceSizeTrig(int width, int height, double angle, double zoomx, double zoomy, int *dstwidth, int *dstheight, double *canglezoom, double *sanglezoom)
{
    double x, y, cx, cy, sx, sy, radangle;
    int dstwidthhalf, dstheighthalf;

    // Determine destination width and height by rotating a centered source box
    radangle = angle * (M_PI / 180.0);
    *sanglezoom = sin(radangle);
    *canglezoom = cos(radangle);
    *sanglezoom *= zoomx;
    *canglezoom *= zoomx;
    x = width / 2;
    y = height / 2;
    cx = *canglezoom * x;
    cy = *canglezoom * y;
    sx = *sanglezoom * x;
    sy = *sanglezoom * y;
    dstwidthhalf = MAX((int) ceil(MAX(MAX(MAX(fabs(cx + sy), fabs(cx - sy)), fabs(-cx + sy)), fabs(-cx - sy))), 1);
    dstheighthalf = MAX((int) ceil(MAX(MAX(MAX(fabs(sx + cy), fabs(sx - cy)), fabs(-sx + cy)), fabs(-sx - cy))), 1);
    *dstwidth = 2 * dstwidthhalf;
    *dstheight = 2 * dstheighthalf;
}

static void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight)
{
    if (zoomx < VALUE_LIMIT) {
	zoomx = VALUE_LIMIT;
    }
    if (zoomy < VALUE_LIMIT) {
	zoomy = VALUE_LIMIT;
    }
    // Calculate target size
    *dstwidth = (int) ((double) width * zoomx);
    *dstheight = (int) ((double) height * zoomy);
    if (*dstwidth < 1) {
	*dstwidth = 1;
    }
    if (*dstheight < 1) {
	*dstheight = 1;
    }
}

SDL_Surface *GUI_Surface::rotozoomSurfaceXY(SDL_Surface * src, double angle, double zoomx, double zoomy, int smooth)
{
    SDL_Surface *rz_src;
    SDL_Surface *rz_dst;
    double zoominv, sanglezoom, canglezoom, sanglezoominv, canglezoominv;
    double x, y, cx, cy, sx, sy;
    int dstwidthhalf, dstwidth, dstheighthalf, dstheight;
    int is32bit, i, src_converted, flipx,flipy;

    if (src == NULL) {
	return (NULL);
    }
    // Determine if source surface is 32bit or 8bit
    is32bit = (src->format->BitsPerPixel == 32);
    if ((is32bit) || (src->format->BitsPerPixel == 8)) {
	// Use source surface 'as is'
	rz_src = src;
	src_converted = 0;
    } else {
	// New source surface is 32bit with a defined RGBA ordering
	rz_src = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, RMask, GMask, BMask, AMask);
	SDL_BlitSurface(src, NULL, rz_src, NULL);
	src_converted = 1;
	is32bit = 1;
    }
    // Sanity check zoom factor
    flipx = (zoomx<0);
    if (flipx) zoomx=-zoomx;
    flipy = (zoomy<0);
    if (flipy) zoomy=-zoomy;
    if (zoomx < VALUE_LIMIT) zoomx = VALUE_LIMIT;
    if (zoomy < VALUE_LIMIT) zoomy = VALUE_LIMIT;
    zoominv = 65536.0 / (zoomx * zoomx);

    // Check if we have a rotozoom or just a zoom
    if (fabs(angle) > VALUE_LIMIT) {
	// Angle!=0: full rotozoom
	// Determine target size */
	rotozoomSurfaceSizeTrig(rz_src->w, rz_src->h, angle, zoomx, zoomy, &dstwidth, &dstheight, &canglezoom, &sanglezoom);
	// Calculate target factors from sin/cos and zoom
	sanglezoominv = sanglezoom;
	canglezoominv = canglezoom;
	sanglezoominv *= zoominv;
	canglezoominv *= zoominv;
	// Calculate half size
	dstwidthhalf = dstwidth / 2;
	dstheighthalf = dstheight / 2;
	// Alloc space to completely contain the rotated surface
	rz_dst = NULL;
	if (is32bit) {
	    // Target surface is 32bit with source RGBA/ABGR ordering
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32, rz_src->format->Rmask, rz_src->format->Gmask, rz_src->format->Bmask, rz_src->format->Amask);
	} else {
	    // Target surface is 8bit
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
	}
	SDL_LockSurface(rz_src);
	// Check which kind of surface we have
	if (is32bit) {
	    // Call the 32bit transformation routine to do the rotation (using alpha)
	    transformSurfaceRGBA(rz_src, rz_dst, dstwidthhalf, dstheighthalf, (int) (sanglezoominv), (int) (canglezoominv), flipx, flipy, smooth);
	    // Turn on source-alpha support
	    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
	} else {
	    // Copy palette and colorkey info
	    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
		rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	    }
	    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	    // Call the 8bit transformation routine to do the rotation
	    transformSurfaceY(rz_src, rz_dst, dstwidthhalf, dstheighthalf, (int) (sanglezoominv), (int) (canglezoominv));
	    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
	}
	SDL_UnlockSurface(rz_src);
    } else {
	// Angle=0: Just a zoom
	// Calculate target size
	zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight);
	// Alloc space to completely contain the zoomed surface
	rz_dst = NULL;
	if (is32bit) {
	    // Target surface is 32bit with source RGBA/ABGR ordering
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32, rz_src->format->Rmask, rz_src->format->Gmask, rz_src->format->Bmask, rz_src->format->Amask);
	} else {
	    // Target surface is 8bit
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
	}
	SDL_LockSurface(rz_src);
	// Check which kind of surface we have
	if (is32bit) {
	    // Call the 32bit transformation routine to do the zooming (using alpha)
	    zoomSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth);
	    // Turn on source-alpha support
	    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
	} else {
	    // Copy palette and colorkey info
	    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
		rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	    }
	    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	    // Call the 8bit transformation routine to do the zooming
	    zoomSurfaceY(rz_src, rz_dst, flipx, flipy);
	    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
	}
	SDL_UnlockSurface(rz_src);
    }
    if (src_converted) {
	SDL_FreeSurface(rz_src);
    }
    return (rz_dst);
}

SDL_Surface *GUI_Surface::rotozoomSurface(SDL_Surface * src, double angle, double zoom, int smooth)
{
    return rotozoomSurfaceXY(src, angle, zoom, zoom, smooth);
}

// Zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
// 'zoomx' and 'zoomy' are scaling factors for width and height. If 'smooth' is 1
// then the destination 32bit surface is anti-aliased. If the surface is not 8bit
// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

SDL_Surface *GUI_Surface::zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth)
{
    SDL_Surface *rz_src;
    SDL_Surface *rz_dst;
    int dstwidth, dstheight, is32bit, i, src_converted, flipx, flipy;

    if (src == NULL) {
	return (NULL);
    }
    // Determine if source surface is 32bit or 8bit
    is32bit = (src->format->BitsPerPixel == 32);
    if ((is32bit) || (src->format->BitsPerPixel == 8)) {
	// Use source surface 'as is'
	rz_src = src;
	src_converted = 0;
    } else {
	// New source surface is 32bit with a defined RGBA ordering
	rz_src = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, RMask, GMask, BMask, AMask);
	SDL_BlitSurface(src, NULL, rz_src, NULL);
	src_converted = 1;
	is32bit = 1;
    }

    flipx = (zoomx<0);
    if (flipx) {
        zoomx = -zoomx;
    }
    flipy = (zoomy<0);
    if (flipy) {
        zoomy = -zoomy;
    }

    // Get size if target
    zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight);

    // Alloc space to completely contain the zoomed surface
    rz_dst = NULL;
    if (is32bit) {
	// Target surface is 32bit with source RGBA/ABGR ordering
	rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32, rz_src->format->Rmask, rz_src->format->Gmask, rz_src->format->Bmask, rz_src->format->Amask);
    } else {
	// Target surface is 8bit
	rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
    }
    SDL_LockSurface(rz_src);
    // Check which kind of surface we have
    if (is32bit) {
	// Call the 32bit transformation routine to do the zooming (using alpha)
	zoomSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth);
	// Turn on source-alpha support
	SDL_SetAlpha(rz_dst, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    } else {
	// Copy palette and colorkey info
	for (i = 0; i < rz_src->format->palette->ncolors; i++) {
	    rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	}
	rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	// Call the 8bit transformation routine to do the zooming
	zoomSurfaceY(rz_src, rz_dst, flipx, flipy);
	SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
    }
    SDL_UnlockSurface(rz_src);
    if (src_converted) {
	SDL_FreeSurface(rz_src);
    }
    return (rz_dst);
}

#ifdef LOAD_BMP

// See if an image is contained in a data source
static int IMG_isBMP(SDL_RWops *src)
{
    int is_BMP;
    char magic[2];

    is_BMP = 0;
    if (SDL_RWread(src, magic, 2, 1)) {
    	if (strncmp(magic, "BM", 2) == 0 ) {
    	    is_BMP = 1;
    	}
    }
    return(is_BMP);
}

// Compression encodings for BMP files
#ifndef BI_RGB
#define BI_RGB		0
#define BI_RLE8		1
#define BI_RLE4		2
#define BI_BITFIELDS	3
#endif

static int readRlePixels(SDL_Surface * surface, SDL_RWops * src, int isRle8)
{
    // Sets the surface pixels from src.  A bmp image is upside down.
    int pitch = surface->pitch;
    int height = surface->h;
    Uint8 * bits = (Uint8 *)surface->pixels + ((height-1) * pitch);
    int ofs = 0;
    Uint8 ch;
    Uint8 needsPad;

    for (;;) {
    	if ( !SDL_RWread(src, &ch, 1, 1) ) return 1;
    	/*
    	| encoded mode starts with a run length, and then a byte
    	| with two colour indexes to alternate between for the run
    	*/
    	if ( ch ) {
    		Uint8 pixel;
    		if ( !SDL_RWread(src, &pixel, 1, 1) ) return 1;
    		if ( isRle8 ) {                 /* 256-color bitmap, compressed */
    			do {
    				bits[ofs++] = pixel;
    			} while (--ch);
    		}else {                         /* 16-color bitmap, compressed */
    			Uint8 pixel0 = pixel >> 4;
    			Uint8 pixel1 = pixel & 0x0F;
    			for (;;) {
    				bits[ofs++] = pixel0;     /* even count, high nibble */
    				if (!--ch) break;
    				bits[ofs++] = pixel1;     /* odd count, low nibble */
    				if (!--ch) break;
    			}
    		}
    	} else {
    		/*
    		| A leading zero is an escape; it may signal the end of the bitmap,
    		| a cursor move, or some absolute data.
    		| zero tag may be absolute mode or an escape
    		*/
    		if ( !SDL_RWread(src, &ch, 1, 1) ) return 1;
    		switch (ch) {
    		case 0:                         /* end of line */
    			ofs = 0;
    			bits -= pitch;               /* go to previous */
    			break;
    		case 1:                         /* end of bitmap */
    			return 0;                    /* success! */
    		case 2:                         /* delta */
    			if ( !SDL_RWread(src, &ch, 1, 1) ) return 1;
    			ofs += ch;
    			if ( !SDL_RWread(src, &ch, 1, 1) ) return 1;
    			bits -= (ch * pitch);
    			break;
    		default:                        /* no compression */
    			if (isRle8) {
    				needsPad = ( ch & 1 );
    				do {
    					if ( !SDL_RWread(src, bits + ofs++, 1, 1) ) return 1;
    				} while (--ch);
    			} else {
    				needsPad = ( ((ch+1)>>1) & 1 ); /* (ch+1)>>1: bytes size */
    				for (;;) {
    					Uint8 pixel;
    					if ( !SDL_RWread(src, &pixel, 1, 1) ) return 1;
    					bits[ofs++] = pixel >> 4;
    					if (!--ch) break;
    					bits[ofs++] = pixel & 0x0F;
    					if (!--ch) break;
    				}
    			}
    			/* pad at even boundary */
    			if ( needsPad && !SDL_RWread(src, &ch, 1, 1) ) return 1;
    			break;
    		}
    	}
    }
}

static SDL_Surface *LoadBMP_RW (SDL_RWops *src, int freesrc)
{
    int was_error;
    long fp_offset;
    int bmpPitch;
    int i, pad;
    SDL_Surface *surface;
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    SDL_Palette *palette;
    Uint8 *bits;
    int ExpandBMP;

    /* The Win32 BMP file header (14 bytes) */
    char   magic[2];
    Uint32 bfSize;
    Uint16 bfReserved1;
    Uint16 bfReserved2;
    Uint32 bfOffBits;

    /* The Win32 BITMAPINFOHEADER struct (40 bytes) */
    Uint32 biSize;
    Sint32 biWidth;
    Sint32 biHeight;
    Uint16 biPlanes;
    Uint16 biBitCount;
    Uint32 biCompression;
    Uint32 biSizeImage;
    Sint32 biXPelsPerMeter;
    Sint32 biYPelsPerMeter;
    Uint32 biClrUsed;
    Uint32 biClrImportant;

    /* Make sure we are passed a valid data source */
    surface = NULL;
    was_error = 0;
    if ( src == NULL ) {
    	was_error = 1;
    	goto done;
    }

    /* Read in the BMP file header */
    fp_offset = SDL_RWtell(src);
    SDL_ClearError();
    if ( SDL_RWread(src, magic, 1, 2) != 2 ) {
    	SDL_Error(SDL_EFREAD);
    	was_error = 1;
    	goto done;
    }
    if ( strncmp(magic, "BM", 2) != 0 ) {
    	SDL_SetError("File is not a Windows BMP file");
    	was_error = 1;
    	goto done;
    }
    bfSize		= SDL_ReadLE32(src);
    bfReserved1	= SDL_ReadLE16(src);
    bfReserved2	= SDL_ReadLE16(src);
    bfOffBits	= SDL_ReadLE32(src);

    /* Read the Win32 BITMAPINFOHEADER */
    biSize		= SDL_ReadLE32(src);
    if ( biSize == 12 ) {
    	biWidth		= (Uint32)SDL_ReadLE16(src);
    	biHeight	= (Uint32)SDL_ReadLE16(src);
    	biPlanes	= SDL_ReadLE16(src);
    	biBitCount	= SDL_ReadLE16(src);
    	biCompression	= BI_RGB;
    	biSizeImage	= 0;
    	biXPelsPerMeter	= 0;
    	biYPelsPerMeter	= 0;
    	biClrUsed	= 0;
    	biClrImportant	= 0;
    } else {
    	biWidth		= SDL_ReadLE32(src);
    	biHeight	= SDL_ReadLE32(src);
    	biPlanes	= SDL_ReadLE16(src);
    	biBitCount	= SDL_ReadLE16(src);
    	biCompression	= SDL_ReadLE32(src);
    	biSizeImage	= SDL_ReadLE32(src);
    	biXPelsPerMeter	= SDL_ReadLE32(src);
    	biYPelsPerMeter	= SDL_ReadLE32(src);
    	biClrUsed	= SDL_ReadLE32(src);
    	biClrImportant	= SDL_ReadLE32(src);
    }

    /* Check for read error */
    if ( strcmp(SDL_GetError(), "") != 0 ) {
    	was_error = 1;
    	goto done;
    }

    /* Expand 1 and 4 bit bitmaps to 8 bits per pixel */
    switch (biBitCount) {
    	case 1:
    	case 4:
    		ExpandBMP = biBitCount;
    		biBitCount = 8;
    		break;
    	default:
    		ExpandBMP = 0;
    		break;
    }

    /* RLE4 and RLE8 BMP compression is supported */
    Rmask = Gmask = Bmask = 0;
    switch (biCompression) {
    	case BI_RGB:
    		/* If there are no masks, use the defaults */
    		if ( bfOffBits == (14+biSize) ) {
    			/* Default values for the BMP format */
    			switch (biBitCount) {
    				case 15:
    				case 16:
    					Rmask = 0x7C00;
    					Gmask = 0x03E0;
    					Bmask = 0x001F;
    					break;
    				case 24:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
					        Rmask = 0x000000FF;
					        Gmask = 0x0000FF00;
					        Bmask = 0x00FF0000;
						break;
#endif
    				case 32:
    					Rmask = 0x00FF0000;
    					Gmask = 0x0000FF00;
    					Bmask = 0x000000FF;
    					break;
    				default:
    					break;
    			}
    			break;
    		}
    		/* Fall through -- read the RGB masks */

    	default:
    		switch (biBitCount) {
    			case 15:
    			case 16:
    			case 32:
    				Rmask = SDL_ReadLE32(src);
    				Gmask = SDL_ReadLE32(src);
    				Bmask = SDL_ReadLE32(src);
    				break;
    			default:
    				break;
    		}
    		break;
    }

    /* Create a compatible surface, note that the colors are RGB ordered */
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
    		biWidth, biHeight, biBitCount, Rmask, Gmask, Bmask, 0);
    if ( surface == NULL ) {
    	was_error = 1;
    	goto done;
    }

    /* Load the palette, if any */
    palette = (surface->format)->palette;
    if ( palette ) {
    	/*
    	| guich: always use 1<<bpp b/c some bitmaps can bring wrong information
    	| for colorsUsed
    	*/
    	/* if ( biClrUsed == 0 ) {  */
    	biClrUsed = 1 << biBitCount;
    	/* } */
    	if ( biSize == 12 ) {
    		for ( i = 0; i < (int)biClrUsed; ++i ) {
    			SDL_RWread(src, &palette->colors[i].b, 1, 1);
    			SDL_RWread(src, &palette->colors[i].g, 1, 1);
    			SDL_RWread(src, &palette->colors[i].r, 1, 1);
    			palette->colors[i].unused = 0;
    		}
    	} else {
    		for ( i = 0; i < (int)biClrUsed; ++i ) {
    			SDL_RWread(src, &palette->colors[i].b, 1, 1);
    			SDL_RWread(src, &palette->colors[i].g, 1, 1);
    			SDL_RWread(src, &palette->colors[i].r, 1, 1);
    			SDL_RWread(src, &palette->colors[i].unused, 1, 1);
    		}
    	}
    	palette->ncolors = biClrUsed;
    }

    /* Read the surface pixels.  Note that the bmp image is upside down */
    if ( SDL_RWseek(src, fp_offset+bfOffBits, SEEK_SET) < 0 ) {
    	SDL_Error(SDL_EFSEEK);
    	was_error = 1;
    	goto done;
    }
    if ((biCompression == BI_RLE4) || (biCompression == BI_RLE8)) {
    	was_error = readRlePixels(surface, src, biCompression == BI_RLE8);
    	if (was_error) SDL_SetError("Error reading from BMP");
    	goto done;
    }
    bits = (Uint8 *)surface->pixels+(surface->h*surface->pitch);
    switch (ExpandBMP) {
    	case 1:
    		bmpPitch = (biWidth + 7) >> 3;
    		pad  = (((bmpPitch)%4) ? (4-((bmpPitch)%4)) : 0);
    		break;
    	case 4:
    		bmpPitch = (biWidth + 1) >> 1;
    		pad  = (((bmpPitch)%4) ? (4-((bmpPitch)%4)) : 0);
    		break;
    	default:
    		pad  = ((surface->pitch%4) ?
    				(4-(surface->pitch%4)) : 0);
    		break;
    }
    while ( bits > (Uint8 *)surface->pixels ) {
    	bits -= surface->pitch;
    	switch (ExpandBMP) {
    		case 1:
    		case 4: {
    		Uint8 pixel = 0;
    		int   shift = (8-ExpandBMP);
    		for ( i=0; i<surface->w; ++i ) {
    			if ( i%(8/ExpandBMP) == 0 ) {
    				if ( !SDL_RWread(src, &pixel, 1, 1) ) {
    					SDL_SetError(
    				"Error reading from BMP");
    					was_error = 1;
    					goto done;
    				}
    			}
    			*(bits+i) = (pixel>>shift);
    			pixel <<= ExpandBMP;
    		} }
    		break;

    		default:
    		if ( SDL_RWread(src, bits, 1, surface->pitch)
    						 != surface->pitch ) {
    			SDL_Error(SDL_EFREAD);
    			was_error = 1;
    			goto done;
    		}
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			/* Byte-swap the pixels if needed. Note that the 24bpp
			   case has already been taken care of above. */
			switch(biBitCount) {
				case 15:
				case 16: {
				        Uint16 *pix = (Uint16 *)bits;
					for(i = 0; i < surface->w; i++)
					        pix[i] = SDL_Swap16(pix[i]);
					break;
				}

				case 32: {
				        Uint32 *pix = (Uint32 *)bits;
					for(i = 0; i < surface->w; i++)
					        pix[i] = SDL_Swap32(pix[i]);
					break;
				}
			}
#endif
     		break;
     	}
     	/* Skip padding bytes, ugh */
     	if ( pad ) {
     		Uint8 padbyte;
     		for ( i=0; i<pad; ++i ) {
     			SDL_RWread(src, &padbyte, 1, 1);
     		}
     	}
     }
done:
    if ( was_error ) {
    	if ( surface ) {
    		SDL_FreeSurface(surface);
    	}
    	surface = NULL;
    }
    if ( freesrc && src ) {
    	SDL_RWclose(src);
    }
    return(surface);
}

// Load a BMP type image from an SDL datasource
static SDL_Surface *IMG_LoadBMP_RW(SDL_RWops *src)
{
    return(LoadBMP_RW(src, 0));
}

#endif /* LOAD_BMP */

#ifdef LOAD_GIF

#define	MAXCOLORMAPSIZE		        256
#define CM_RED		                0
#define CM_GREEN	                1
#define CM_BLUE		                2
#define	MAX_LWZ_BITS		        12
#define INTERLACE		        0x40
#define LOCALCOLORMAP	                0x80
#define BitSet(byte, bit)	        (((byte) & (bit)) == (bit))
#define	ReadOK(file,buffer,len)	        SDL_RWread(file, buffer, len, 1)
#define LM_to_uint(a,b)			(((b)<<8)|(a))

typedef struct {
    unsigned int Width;
    unsigned int Height;
    unsigned char ColorMap[3][MAXCOLORMAPSIZE];
    unsigned int BitPixel;
    unsigned int ColorResolution;
    unsigned int Background;
    unsigned int AspectRatio;
    int grayScale;
    int transparent;
    int delayTime;
    int inputFlag;
    int disposal;
    unsigned char extbuf[256];
    int ZeroDataBlock;
    unsigned char codebuf[280];
    int curbit;
    int lastbit;
    int last_byte;
    int done;
    int fresh;
    int code_size;
    int set_code_size;
    int max_code;
    int max_code_size;
    int firstcode;
    int oldcode;
    int clear_code;
    int end_code;
    int table[2][(1 << MAX_LWZ_BITS)];
    int stack[(1 << (MAX_LWZ_BITS)) * 2];
    int *sp;
} SDL_Gif;

static int ReadColorMap(SDL_RWops * src, int number, unsigned char buffer[3][MAXCOLORMAPSIZE], int *flag, SDL_Gif *gif);
static int DoExtension(SDL_RWops * src, int label, SDL_Gif *gif);
static int GetDataBlock(SDL_RWops * src, unsigned char *buf, SDL_Gif *gif);
static int GetCode(SDL_RWops * src, int code_size, int flag, SDL_Gif *gif);
static int LWZReadByte(SDL_RWops * src, int flag, int input_code_size, SDL_Gif *gif);
static SDL_Surface *ReadImage(SDL_RWops * src, int len, int height, int, unsigned char cmap[3][MAXCOLORMAPSIZE], int gray, int interlace, int ignore, SDL_Gif *gif);

// See if an image is contained in a data source
static int IMG_isGIF(SDL_RWops *src)
{
    int is_GIF = 0;
    char magic[6];

    if (SDL_RWread(src, magic, 6, 1) ) {
    	if ((strncmp(magic, "GIF", 3) == 0) &&
    	    ((memcmp(magic + 3, "87a", 3) == 0) ||
    	    (memcmp(magic + 3, "89a", 3) == 0)) ) {
    	    is_GIF = 1;
    	}
    }
    return(is_GIF);
}

static SDL_Surface *IMG_LoadGIF_RW(SDL_RWops *src)
{
    char version[4];
    unsigned char c, buf[16];
    unsigned char localColorMap[3][MAXCOLORMAPSIZE];
    int grayScale, useGlobalColormap, bitPixel;
    int imageCount = 0, imageNumber = 1;
    SDL_Surface *image = NULL;
    SDL_Gif gif;

    if (src == NULL) {
        goto done;
    }
    if (!ReadOK(src, buf, 6)) {
	SDL_SetError("error reading magic number");
        goto done;
    }
    if (strncmp((char *) buf, "GIF", 3) != 0) {
	SDL_SetError("not a GIF file");
        goto done;
    }
    strncpy(version, (char *) buf + 3, 3);
    version[3] = '\0';

    if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)) {
	SDL_SetError("bad version number, not '87a' or '89a'");
        goto done;
    }
    memset(&gif, 0, sizeof(gif));
    gif.transparent = -1;
    gif.delayTime = -1;
    gif.inputFlag = -1;
    gif.disposal = 0;

    if (!ReadOK(src, buf, 7)) {
	SDL_SetError("failed to read screen descriptor");
        goto done;
    }
    gif.Width = LM_to_uint(buf[0], buf[1]);
    gif.Height = LM_to_uint(buf[2], buf[3]);
    gif.BitPixel = 2 << (buf[4] & 0x07);
    gif.ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
    gif.Background = buf[5];
    gif.AspectRatio = buf[6];

    if (BitSet(buf[4], LOCALCOLORMAP)) {	// Global Colormap */
	if (ReadColorMap(src, gif.BitPixel, gif.ColorMap, &gif.grayScale, &gif)) {
	    SDL_SetError("error reading global colormap");
            goto done;
	}
    }
    do {
	if (!ReadOK(src, &c, 1)) {
	    SDL_SetError("EOF / read error on image data");
            goto done;
	}
	if (c == ';') {		// GIF terminator */
	    if (imageCount < imageNumber) {
		SDL_SetError("only %d image%s found in file", imageCount, imageCount > 1 ? "s" : "");
                goto done;
	    }
	}
	if (c == '!') {		// Extension */
	    if (!ReadOK(src, &c, 1)) {
		SDL_SetError("EOF / read error on extention function code");
                goto done;
	    }
	    DoExtension(src, c, &gif);
	    continue;
	}
	if (c != ',') {		// Not a valid start character */
	    continue;
	}
	++imageCount;
	if (!ReadOK(src, buf, 9)) {
	    SDL_SetError("couldn't read left/top/width/height");
            goto done;
	}
	useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);
	bitPixel = 1 << ((buf[8] & 0x07) + 1);
	if (!useGlobalColormap) {
	    if (ReadColorMap(src, bitPixel, localColorMap, &grayScale, &gif)) {
		SDL_SetError("error reading local colormap");
                goto done;
	    }
	    image = ReadImage(src, LM_to_uint(buf[4], buf[5]), LM_to_uint(buf[6], buf[7]), bitPixel, localColorMap, grayScale, BitSet(buf[8], INTERLACE), imageCount != imageNumber, &gif);
	} else {
	    image = ReadImage(src, LM_to_uint(buf[4], buf[5]), LM_to_uint(buf[6], buf[7]), gif.BitPixel, gif.ColorMap, gif.grayScale, BitSet(buf[8], INTERLACE), imageCount != imageNumber, &gif);
	}
    } while (image == NULL);
    if (gif.transparent >= 0) {
        SDL_SetColorKey(image, SDL_SRCCOLORKEY, gif.transparent);
    }
done:
    return image;
}

static int ReadColorMap(SDL_RWops *src, int number, unsigned char buffer[3][MAXCOLORMAPSIZE], int *gray, SDL_Gif *gif)
{
    unsigned char rgb[3];
    int flag = 1;

    for (int i = 0; i < number; ++i) {
	if (!ReadOK(src, rgb, sizeof(rgb))) {
	    SDL_SetError("bad colormap");
	    return 1;
	}
	buffer[CM_RED][i] = rgb[0];
	buffer[CM_GREEN][i] = rgb[1];
	buffer[CM_BLUE][i] = rgb[2];
	flag &= (rgb[0] == rgb[1] && rgb[1] == rgb[2]);
    }
    *gray = 0;
    return 0;
}

static int DoExtension(SDL_RWops *src, int label, SDL_Gif *gif)
{
    char *str;

    switch (label) {
    case 0x01:			// Plain Text Extension */
	str = "Plain Text Extension";
	break;
    case 0xff:			// Application Extension */
	str = "Application Extension";
	break;
    case 0xfe:			// Comment Extension */
	str = "Comment Extension";
	while (GetDataBlock(src, (unsigned char *) gif->extbuf, gif) != 0);
	return 0;
    case 0xf9:			// Graphic Control Extension */
	str = "Graphic Control Extension";
	(void) GetDataBlock(src, (unsigned char *) gif->extbuf, gif);
	gif->disposal = (gif->extbuf[0] >> 2) & 0x7;
	gif->inputFlag = (gif->extbuf[0] >> 1) & 0x1;
	gif->delayTime = LM_to_uint(gif->extbuf[1], gif->extbuf[2]);
	if ((gif->extbuf[0] & 0x1) != 0) {
	    gif->transparent = gif->extbuf[3];
        }
	while (GetDataBlock(src, (unsigned char *) gif->extbuf, gif) != 0);
	return 0;
    default:
	str = (char *)gif->extbuf;
	sprintf(str, "UNKNOWN (0x%02x)", label);
	break;
    }
    while (GetDataBlock(src, (unsigned char *) gif->extbuf, gif) != 0);
    return 0;
}

static int GetDataBlock(SDL_RWops *src, unsigned char *buf, SDL_Gif *gif)
{
    unsigned char count;

    if (!ReadOK(src, &count, 1)) {
	SDL_SetError("error in getting DataBlock size" );
	return -1;
    }
    gif->ZeroDataBlock = count == 0;
    if ((count != 0) && (!ReadOK(src, buf, count))) {
	SDL_SetError("error in reading DataBlock" );
	return -1;
    }
    return count;
}

static int GetCode(SDL_RWops *src, int code_size, int flag, SDL_Gif *gif)
{
    int i, j, ret;
    unsigned char count;

    if (flag) {
	gif->curbit = gif->lastbit = gif->done = 0;
	return 0;
    }
    if ((gif->curbit + code_size) >= gif->lastbit) {
	if (gif->done) {
	    if (gif->curbit >= gif->lastbit) {
		SDL_SetError("ran off the end of my bits");
            }
	    return -1;
	}
	gif->codebuf[0] = gif->codebuf[gif->last_byte - 2];
	gif->codebuf[1] = gif->codebuf[gif->last_byte - 1];

	if ((count = GetDataBlock(src, &gif->codebuf[2], gif)) == 0) {
	    gif->done = 1;
        }
	gif->last_byte = 2 + count;
	gif->curbit = (gif->curbit - gif->lastbit) + 16;
	gif->lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = gif->curbit, j = 0; j < code_size; ++i, ++j) {
	ret |= ((gif->codebuf[i / 8] & (1 << (i % 8))) != 0) << j;
    }
    gif->curbit += code_size;
    return ret;
}

static int LWZReadByte(SDL_RWops *src, int flag, int input_code_size, SDL_Gif *gif)
{
    int i, code, incode;

    if (flag) {
	gif->set_code_size = input_code_size;
	gif->code_size = gif->set_code_size + 1;
	gif->clear_code = 1 << gif->set_code_size;
	gif->end_code = gif->clear_code + 1;
	gif->max_code_size = 2 * gif->clear_code;
	gif->max_code = gif->clear_code + 2;
	GetCode(src, 0, 1, gif);
	gif->fresh = 1;
	for (i = 0; i < gif->clear_code; ++i) {
	    gif->table[0][i] = 0;
	    gif->table[1][i] = i;
	}
	for (; i < (1 << MAX_LWZ_BITS); ++i) {
	    gif->table[0][i] = gif->table[1][0] = 0;
        }
	gif->sp = gif->stack;
	return 0;
    } else
    if (gif->fresh) {
	gif->fresh = 0;
	do {
	    gif->firstcode = gif->oldcode = GetCode(src, gif->code_size, 0, gif);
	} while (gif->firstcode == gif->clear_code);
	return gif->firstcode;
    }
    if (gif->sp > gif->stack) {
	return *--gif->sp;
    }
    while ((code = GetCode(src, gif->code_size, 0, gif)) >= 0) {
	if (code == gif->clear_code) {
	    for (i = 0; i < gif->clear_code; ++i) {
		gif->table[0][i] = 0;
		gif->table[1][i] = i;
	    }
	    for (; i < (1 << MAX_LWZ_BITS); ++i) {
		gif->table[0][i] = gif->table[1][i] = 0;
            }
	    gif->code_size = gif->set_code_size + 1;
	    gif->max_code_size = 2 * gif->clear_code;
	    gif->max_code = gif->clear_code + 2;
	    gif->sp = gif->stack;
	    gif->firstcode = gif->oldcode = GetCode(src, gif->code_size, 0, gif);
	    return gif->firstcode;
	} else
        if (code == gif->end_code) {
	    int count;
	    unsigned char buf[260];
	    if (gif->ZeroDataBlock) {
		return -2;
            }
	    while ((count = GetDataBlock(src, buf, gif)) > 0);
	    if (count != 0) {
                SDL_SetError("missing EOD in data stream (common occurence)");
	    }
	    return -2;
	}
	incode = code;
	if (code >= gif->max_code) {
	    *gif->sp++ = gif->firstcode;
	    code = gif->oldcode;
	}
	while (code >= gif->clear_code) {
	    *gif->sp++ = gif->table[1][code];
	    if (code == gif->table[0][code]) {
		SDL_SetError("circular table entry BIG ERROR");
            }
	    code = gif->table[0][code];
	}
	*gif->sp++ = gif->firstcode = gif->table[1][code];
	if ((code = gif->max_code) < (1 << MAX_LWZ_BITS)) {
	    gif->table[0][code] = gif->oldcode;
	    gif->table[1][code] = gif->firstcode;
	    ++gif->max_code;
	    if ((gif->max_code >= gif->max_code_size) && (gif->max_code_size < (1 << MAX_LWZ_BITS))) {
		gif->max_code_size *= 2;
		++gif->code_size;
	    }
	}
	gif->oldcode = incode;
	if (gif->sp > gif->stack) {
	    return *--gif->sp;
        }
    }
    return code;
}

static SDL_Surface *ReadImage(SDL_RWops * src, int len, int height, int cmapSize, unsigned char cmap[3][MAXCOLORMAPSIZE], int gray, int interlace, int ignore, SDL_Gif *gif)
{
    SDL_Surface *image;
    unsigned char c;
    int i, v, xpos = 0, ypos = 0, pass = 0;

    // Initialize the compression routines
    if (!ReadOK(src, &c, 1)) {
	SDL_SetError("EOF / read error on image data");
	return NULL;
    }
    if (LWZReadByte(src, 1, c, gif) < 0) {
	SDL_SetError("error reading image");
	return NULL;
    }
    //	If this is an "uninteresting picture" ignore it.
    if (ignore) {
	while (LWZReadByte(src, 0, c, gif) >= 0);
	return NULL;
    }
    image = SDL_CreateRGBSurface(SDL_SWSURFACE, len, height, 8, 0, 0, 0, 0);
    for (i = 0; i < cmapSize; i++) {
        image->format->palette->colors[i].r = cmap[CM_RED][i];
	image->format->palette->colors[i].g = cmap[CM_GREEN][i];
	image->format->palette->colors[i].b = cmap[CM_BLUE][i];
    }
    while ((v = LWZReadByte(src, 0, c, gif)) >= 0) {
        ((Uint8 *)image->pixels)[xpos + ypos * image->pitch] = v;
	++xpos;
	if (xpos == len) {
	    xpos = 0;
	    if (interlace) {
		switch (pass) {
		case 0:
		case 1:
		    ypos += 8;
		    break;
		case 2:
		    ypos += 4;
		    break;
		case 3:
		    ypos += 2;
		    break;
		}
		if (ypos >= height) {
		    ++pass;
		    switch (pass) {
		    case 1:
			ypos = 4;
			break;
		    case 2:
			ypos = 2;
			break;
		    case 3:
			ypos = 1;
			break;
		    default:
			goto fini;
		    }
		}
	    } else {
		++ypos;
	    }
	}
	if (ypos >= height) {
	    break;
        }
    }
fini:
    return image;
}

#endif // LOAD_GIF */

#ifdef LOAD_JPG

extern "C" {
#include <jpeglib.h>
}

#define INPUT_BUFFER_SIZE	4096
typedef struct {
	struct jpeg_source_mgr pub;

	SDL_RWops *ctx;
	Uint8 buffer[INPUT_BUFFER_SIZE];
} my_source_mgr;

// See if an image is contained in a data source
static int IMG_isJPG(SDL_RWops *src)
{
    int is_JPG = 0;
    Uint8 magic[4];

    if (SDL_RWread(src, magic, 2, 1)) {
    	if ((magic[0] == 0xFF) && (magic[1] == 0xD8) ) {
    	    SDL_RWread(src, magic, 4, 1);
    	    SDL_RWread(src, magic, 4, 1);
    	    if (memcmp((char *)magic, "JFIF", 4) == 0 || memcmp((char *)magic, "Exif", 4) == 0) {
      		is_JPG = 1;
    	    }
    	}
    }
    return(is_JPG);
}

// Initialize source --- called by jpeg_read_header before any data is actually read.
static void init_source (j_decompress_ptr cinfo)
{
    return;
}

//Fill the input buffer --- called whenever buffer is emptied.
static int fill_input_buffer (j_decompress_ptr cinfo)
{
    my_source_mgr * src = (my_source_mgr *) cinfo->src;
    int nbytes = SDL_RWread(src->ctx, src->buffer, 1, INPUT_BUFFER_SIZE);
    if (nbytes <= 0) {
    	// Insert a fake EOI marker
    	src->buffer[0] = (Uint8) 0xFF;
    	src->buffer[1] = (Uint8) JPEG_EOI;
    	nbytes = 2;
    }
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    return 1;
}

// Skip data --- used to skip over a potentially large amount of
// uninteresting data (such as an APPn marker).
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr * src = (my_source_mgr *) cinfo->src;
    if (num_bytes > 0) {
    	while (num_bytes > (long) src->pub.bytes_in_buffer) {
    	      num_bytes -= (long) src->pub.bytes_in_buffer;
    	      src->pub.fill_input_buffer(cinfo);
    	}
    	src->pub.next_input_byte += (size_t) num_bytes;
    	src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

// Terminate source --- called by jpeg_finish_decompress after all data has been read.
static void term_source (j_decompress_ptr cinfo)
{
    return;
}

// Prepare for input from a stdio stream.
// The caller must have already opened the stream, and is responsible
// for closing it after finishing decompression.
static void jpeg_SDL_RW_src (j_decompress_ptr cinfo, SDL_RWops *ctx)
{
    my_source_mgr *src;

    // first time for this JPEG object?
    if (cinfo->src == NULL) {
        cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(my_source_mgr));
        src = (my_source_mgr *) cinfo->src;
    }
    src = (my_source_mgr *) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method */
    src->pub.term_source = term_source;
    src->ctx = ctx;
    src->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL; // until buffer loaded */
}

struct my_error_mgr {
    struct jpeg_error_mgr errmgr;
    jmp_buf escape;
};

static void my_error_exit(j_common_ptr cinfo)
{
    struct my_error_mgr *err = (struct my_error_mgr *)cinfo->err;
    longjmp(err->escape, 1);
}

static void output_no_message(j_common_ptr cinfo)
{
}

// Load a JPEG type image from an SDL datasource */
static SDL_Surface *IMG_LoadJPG_RW(SDL_RWops *src)
{
    struct jpeg_decompress_struct cinfo;
    JSAMPROW rowptr[1];
    SDL_Surface *volatile surface = NULL;
    struct my_error_mgr jerr;

    if ( !src ) {
    	// The error message has been set in SDL_RWFromFile */
    	return NULL;
    }

    // Create a decompression structure and load the JPEG header */
    cinfo.err = jpeg_std_error(&jerr.errmgr);
    jerr.errmgr.error_exit = my_error_exit;
    jerr.errmgr.output_message = output_no_message;
    if(setjmp(jerr.escape)) {
    	// If we get here, libjpeg found an error */
    	jpeg_destroy_decompress(&cinfo);
    	SDL_SetError("JPEG loading error");
    	SDL_FreeSurface(surface);
    	return NULL;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_SDL_RW_src(&cinfo, src);
    jpeg_read_header(&cinfo, 1);

    // Set 24-bit RGB output */
    cinfo.out_color_space = JCS_RGB;
    cinfo.quantize_colors = 0;
#ifdef FAST_JPEG
    cinfo.scale_num   = 1;
    cinfo.scale_denom = 1;
    cinfo.dct_method = JDCT_FASTEST;
    cinfo.do_fancy_upsampling = 0;
#endif
    jpeg_calc_output_dimensions(&cinfo);

    // Allocate an output surface to hold the image */
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, cinfo.output_width, cinfo.output_height, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	                           0x0000FF, 0x00FF00, 0xFF0000,
#else
	                           0xFF0000, 0x00FF00, 0x0000FF,
#endif
    0);

    if (surface == NULL) {
	SDL_SetError("Out of memory");
	goto done;
    }

    // Decompress the image
    jpeg_start_decompress(&cinfo);
    while (cinfo.output_scanline < cinfo.output_height) {
	  rowptr[0] = (JSAMPROW)(Uint8 *)surface->pixels + cinfo.output_scanline * surface->pitch;
      	  jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
    }
    jpeg_finish_decompress(&cinfo);
done:
    jpeg_destroy_decompress(&cinfo);
    return(surface);
}

#endif // LOAD_JPG */

#ifdef LOAD_PNG

#define PNG_BYTES_TO_CHECK 4

// See if an image is contained in a data source
static int IMG_isPNG(SDL_RWops *src)
{
    unsigned char buf[PNG_BYTES_TO_CHECK];

    // Read in the signature bytes
    if (SDL_RWread(src, buf, 1, PNG_BYTES_TO_CHECK) != PNG_BYTES_TO_CHECK) {
      return 0;
    }
    // Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
    return( !png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
}

// Load a PNG type image from an SDL datasource
static void png_read_data(png_structp ctx, png_bytep area, png_size_t size)
{
    SDL_RWops *src = (SDL_RWops *)png_get_io_ptr(ctx);
    SDL_RWread(src, area, size, 1);
}

static SDL_Surface *IMG_LoadPNG_RW(SDL_RWops *src)
{
    SDL_Surface *volatile surface;
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
    SDL_Palette *palette;
    png_bytep *volatile row_pointers;
    int row, i;
    volatile int ckey = -1;
    png_color_16 *transv;

    if (!src) {
    	// The error message has been set in SDL_RWFromFile
    	return NULL;
    }

    // Initialize the data we will clean up when we're done
    png_ptr = NULL; info_ptr = NULL; row_pointers = NULL; surface = NULL;

    // Create the PNG loading context structure
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
    				  NULL,NULL,NULL);
    if (png_ptr == NULL){
    	SDL_SetError("Couldn't allocate memory for PNG file or incompatible PNG dll");
    	goto done;
    }

    // Allocate/initialize the memory for image information.  REQUIRED.
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
    	SDL_SetError("Couldn't create image information for PNG file");
    	goto done;
    }

    // Set error handling if you are using setjmp/longjmp method (this is
    // the normal method of doing things with libpng).  REQUIRED unless you
    // set up your own error handlers in png_create_read_struct() earlier.
    if (setjmp(png_ptr->jmpbuf)) {
    	SDL_SetError("Error reading the PNG file.");
    	goto done;
    }
    // Set up the input control
    png_set_read_fn(png_ptr, src, png_read_data);
    // Read PNG header info
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    // tell libpng to strip 16 bit/color files down to 8 bits/color
    png_set_strip_16(png_ptr) ;
    // Extract multiple pixels with bit depths of 1, 2, and 4 from a single
    // byte into separate bytes (useful for paletted and grayscale images).
    png_set_packing(png_ptr);
    // scale greyscale values to the range 0..255
    if(color_type == PNG_COLOR_TYPE_GRAY) {
    	png_set_expand(png_ptr);
    }
    // For images with a single "transparent colour", set colour key;
    // if more than one index has transparency, or if partially transparent
    //  entries exist, use full alpha channel
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        int num_trans;
    	Uint8 *trans;
    	png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &transv);
    	if (color_type == PNG_COLOR_TYPE_PALETTE) {
    	    // Check if all tRNS entries are opaque except one
    	    int i, t = -1;
    	    for (i = 0; i < num_trans; i++) {
    		 if (trans[i] == 0) {
    		     if (t >= 0) {
    			 break;
                     }
    		     t = i;
    		 } else
                 if (trans[i] != 255) {
    		    break;
                 }
            }
    	    if (i == num_trans) {
    		// exactly one transparent index
    		ckey = t;
    	    } else {
    		// more than one transparent index, or translucency
    		png_set_expand(png_ptr);
    	    }
    	} else
    	    ckey = 0; // actual value will be set later
    }
    if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    	png_set_gray_to_rgb(png_ptr);
    }
    png_read_update_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    // Allocate the SDL surface to hold the image
    Rmask = Gmask = Bmask = Amask = 0 ;
    if (color_type != PNG_COLOR_TYPE_PALETTE) {
    	if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
    	    Rmask = 0x000000FF;
    	    Gmask = 0x0000FF00;
    	    Bmask = 0x00FF0000;
    	    Amask = (info_ptr->channels == 4) ? 0xFF000000 : 0;
    	} else {
    	    int s = (info_ptr->channels == 4) ? 0 : 8;
    	    Rmask = 0xFF000000 >> s;
    	    Gmask = 0x00FF0000 >> s;
    	    Bmask = 0x0000FF00 >> s;
    	    Amask = 0x000000FF >> s;
    	}
    }
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, bit_depth*info_ptr->channels, Rmask, Gmask, Bmask, Amask);
    if (surface == NULL) {
    	SDL_SetError("Out of memory");
    	goto done;
    }

    if (ckey != -1) {
        if(color_type != PNG_COLOR_TYPE_PALETTE)
    	// FIXME: Should these be truncated or shifted down?
    	ckey = SDL_MapRGB(surface->format, (Uint8)transv->red, (Uint8)transv->green, (Uint8)transv->blue);
        SDL_SetColorKey(surface, SDL_SRCCOLORKEY, ckey);
    }
    // Create the array of pointers to image data
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*height);
    if ((row_pointers == NULL)) {
    	SDL_SetError("Out of memory");
    	SDL_FreeSurface(surface);
    	surface = NULL;
    	goto done;
    }
    for (row = 0; row < (int)height; row++) {
    	row_pointers[row] = (png_bytep)(Uint8 *)surface->pixels + row*surface->pitch;
    }
    // Read the entire image in one go
    png_read_image(png_ptr, row_pointers);
    // and we're done!  (png_read_end() can be omitted if no processing of
    // post-IDAT text/time/etc. is desired)
    // In some cases it can't read PNG's created by some popular programs (ACDSEE),
    // we do not want to process comments, so we omit png_read_end
    png_read_end(png_ptr, info_ptr);
    // Load the palette, if any
    palette = surface->format->palette;
    if (palette) {
        if (color_type == PNG_COLOR_TYPE_GRAY) {
    	    palette->ncolors = 256;
    	    for(i = 0; i < 256; i++) {
    	        palette->colors[i].r = i;
    	        palette->colors[i].g = i;
    	        palette->colors[i].b = i;
    	    }
        } else
        if (info_ptr->num_palette > 0 ) {
    	    palette->ncolors = info_ptr->num_palette;
    	    for (i = 0; i<info_ptr->num_palette; ++i ) {
    	        palette->colors[i].b = info_ptr->palette[i].blue;
    	        palette->colors[i].g = info_ptr->palette[i].green;
    	        palette->colors[i].r = info_ptr->palette[i].red;
    	    }
        }
    }
done:
    png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0, (png_infopp)0);
    if (row_pointers) {
    	free(row_pointers);
    }
    return(surface);
}

#endif // LOAD_PNG

#ifdef LOAD_XPM

// Hash table to look up colors from pixel strings
#define STARTING_HASH_SIZE 256
// fast lookup that works if cpp == 1
#define QUICK_COLORHASH(hash, key) ((hash)->table[*(Uint8 *)(key)]->color)

struct hash_entry {
     char *key;
     Uint32 color;
     struct hash_entry *next;
};

struct color_hash {
     struct hash_entry **table;
     struct hash_entry *entries; // array of all entries
     struct hash_entry *next_free;
     int size;
     int maxnum;
};

// See if an image is contained in a data source
static int IMG_isXPM(SDL_RWops *src)
{
    char magic[9];

    return (SDL_RWread(src, magic, sizeof(magic), 1) && memcmp(magic, "// XPM ", 9) == 0);
}


static int hash_key(const char *key, int cpp, int size)
{
    int hash;

    hash = 0;
    while (cpp-- > 0) {
    	hash = hash * 33 + *key++;
    }
    return hash & (size - 1);
}

static struct color_hash *create_colorhash(int maxnum)
{
    int bytes, s;
    struct color_hash *hash;

    // we know how many entries we need, so we can allocate everything here
    hash = (color_hash*)malloc(sizeof *hash);
    if (!hash) {
    	return NULL;
    }
    // use power-of-2 sized hash table for decoding speed
    for (s = STARTING_HASH_SIZE; s < maxnum; s <<= 1) ;
    hash->size = s;
    hash->maxnum = maxnum;
    bytes = hash->size * sizeof(struct hash_entry **);
    hash->entries = NULL;	// in case malloc fails
    hash->table = (struct hash_entry**)malloc(bytes);
    if (!hash->table) {
    	return NULL;
    }
    memset(hash->table, 0, bytes);
    hash->entries = (struct hash_entry*)malloc(maxnum * sizeof(struct hash_entry));
    if (!hash->entries) {
    	return NULL;
    }
    hash->next_free = hash->entries;
    return hash;
}

static int add_colorhash(struct color_hash *hash, char *key, int cpp, Uint32 color)
{
    int index = hash_key(key, cpp, hash->size);
    struct hash_entry *e = hash->next_free++;
    e->color = color;
    e->key = key;
    e->next = hash->table[index];
    hash->table[index] = e;
    return 1;
}

static Uint32 get_colorhash(struct color_hash *hash, const char *key, int cpp)
{
    struct hash_entry *entry = hash->table[hash_key(key, cpp, hash->size)];
    while (entry) {
    	  if (memcmp(key, entry->key, cpp) == 0) {
    	      return entry->color;
          }
    	  entry = entry->next;
    }
    return 0;
}

static void free_colorhash(struct color_hash *hash)
{
    if (hash && hash->table) {
    	free(hash->table);
    	free(hash->entries);
    	free(hash);
    }
}

// convert colour spec to RGB (in 0xrrggbb format).
// return 1 if successful.

static int color_to_rgb(char *spec, int speclen, Uint32 *rgb)
{
    // poor man's rgb.txt
    static struct {
        char *name;
        Uint32 rgb;
    } known[] = {
    	{"none",  0xffffffff},
    	{"black", 0x00000000},
    	{"white", 0x00ffffff},
    	{"red",   0x00ff0000},
    	{"green", 0x0000ff00},
    	{"blue",  0x000000ff},
        { 0, 0 }
    };

    if (spec[0] == '#') {
    	char buf[7];
    	switch(speclen) {
    	case 4:
    	    buf[0] = buf[1] = spec[1];
    	    buf[2] = buf[3] = spec[2];
    	    buf[4] = buf[5] = spec[3];
    	    break;
    	case 7:
    	    memcpy(buf, spec + 1, 6);
    	    break;
    	case 13:
    	    buf[0] = spec[1];
    	    buf[1] = spec[2];
    	    buf[2] = spec[5];
    	    buf[3] = spec[6];
    	    buf[4] = spec[9];
    	    buf[5] = spec[10];
    	    break;
    	}
    	buf[6] = '\0';
    	*rgb = strtol(buf, NULL, 16);
    	return 1;
    } else {
    	for (int i = 0; known[i].name; i++) {
    	    if (!strncasecmp(known[i].name, spec, speclen)) {
    	        *rgb = known[i].rgb;
    	        return 1;
    	    }
            }
    	return 0;
    }
}

//
// Read next line from the source.
// If len > 0, it's assumed to be at least len chars (for efficiency).
// Return NULL and set error upon EOF or parse error.

static char *get_next_line(char ***lines, SDL_RWops *src, int len, char **linebuf, int *buflen)
{
    if (lines) {
    	return *(*lines)++;
    } else {
    	char c;
    	int n;
    	do {
    	   if (SDL_RWread(src, &c, 1, 1) <= 0) {
    	       SDL_SetError("Premature end of data");
    	       return NULL;
    	   }
    	} while(c != '"');
    	if (len) {
    	    len += 4;	// "\",\n\0"
    	    if (len > *buflen){
    	    	*buflen = len;
    	    	*linebuf = (char*)realloc(*linebuf, *buflen);
    	    	if (!(*linebuf)) {
    	    	    SDL_SetError("Out of memory");
    	    	    return NULL;
    	    	}
    	    }
    	    if (SDL_RWread(src, *linebuf, len - 1, 1) <= 0) {
    	    	SDL_SetError("Premature end of data");
    	    	return NULL;
    	    }
    	    n = len - 2;
    	} else {
    	    n = 0;
    	    do {
    	    	if (n >= *buflen - 1) {
    	    	    if (*buflen == 0) {
    	    		*buflen = 16;
                    }
    	    	    *buflen *= 2;
    	    	    *linebuf = (char*)realloc(*linebuf, *buflen);
    	    	    if (!(*linebuf)) {
    	    		SDL_SetError("Out of memory");
    	    		return NULL;
    	    	    }
    	    	}
    	    	if (SDL_RWread(src, (*linebuf) + n, 1, 1) <= 0) {
    	    	    SDL_SetError("Premature end of data");
  	    	    return NULL;
    	    	}
    	    } while((*linebuf)[n++] != '"');
    	    n--;
    	}
    	(*linebuf)[n] = '\0';
    	return *linebuf;
    }
}

#define SKIPSPACE(p) do { while(isspace((unsigned char)*(p))) ++(p); } while(0)
#define SKIPNONSPACE(p) do { while(!isspace((unsigned char)*(p)) && *p)	++(p); } while(0)

// read XPM from either array or RWops
static SDL_Surface *load_xpm(char **xpm, SDL_RWops *src)
{
    SDL_Surface *image = NULL;
    int index;
    int x, y;
    int w, h, ncolors, cpp;
    int indexed;
    Uint8 *dst;
    struct color_hash *colors = NULL;
    SDL_Color *im_colors = NULL;
    char *keystrings = NULL, *nextkey;
    char *line;
    char ***xpmlines = NULL;
    int pixels_len;
    char *linebuf = 0;
    int buflen = 0;

    if(xpm) {
    	xpmlines = &xpm;
    }
    line = get_next_line(xpmlines, src, 0, &linebuf, &buflen);
    if (!line) {
    	goto err;
    }
    //
    // The header string of an XPMv3 image has the format
    //
    // <width> <height> <ncolors> <cpp> [ <hotspot_x> <hotspot_y> ]
    //
    // where the hotspot coords are intended for mouse cursors.
    // Right now we don't use the hotspots but it should be handled one day.

    if (sscanf(line, "%d %d %d %d", &w, &h, &ncolors, &cpp) != 4 || w <= 0 || h <= 0 || ncolors <= 0 || cpp <= 0) {
    	SDL_SetError("Invalid format description");
    	goto err;
    }
    keystrings = (char*)malloc(ncolors * cpp);
    if (!keystrings) {
    	SDL_SetError("Out of memory");
    	goto err;
    }
    nextkey = keystrings;

    // Create the new surface
    if (ncolors <= 256) {
    	indexed = 1;
    	image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
    	im_colors = image->format->palette->colors;
    	image->format->palette->ncolors = ncolors;
    } else {
    	indexed = 0;
    	image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0xff0000, 0x00ff00, 0x0000ff, 0);
    }
    if (!image) {
    	goto done;
    }
    // Read the colors
    colors = create_colorhash(ncolors);
    if (!colors) {
    	SDL_SetError("Out of memory");
    	goto err;
    }
    for (index = 0; index < ncolors; ++index ) {
    	char *p;
    	line = get_next_line(xpmlines, src, 0, &linebuf, &buflen);
    	if (!line) {
    	    goto err;
        }
    	p = line + cpp + 1;
    	// parse a colour definition
    	for (;;) {
    	    char nametype, *colname;
    	    Uint32 rgb, pixel;

    	    SKIPSPACE(p);
    	    if (!*p) {
    	    	SDL_SetError("colour parse error");
    	    	goto err;
    	    }
    	    nametype = *p;
    	    SKIPNONSPACE(p);
    	    SKIPSPACE(p);
    	    colname = p;
    	    SKIPNONSPACE(p);
    	    if (nametype == 's') {
    	    	continue;      // skip symbolic colour names
            }
    	    if (!color_to_rgb(colname, p - colname, &rgb)) {
    	    	continue;
            }
    	    memcpy(nextkey, line, cpp);
    	    if (indexed) {
    	    	SDL_Color *c = im_colors + index;
    	    	c->r = rgb >> 16;
    	    	c->g = rgb >> 8;
    	    	c->b = rgb;
    	    	pixel = index;
    	    } else {
    	    	pixel = rgb;
            }
    	    add_colorhash(colors, nextkey, cpp, pixel);
    	    nextkey += cpp;
    	    if (rgb == 0xffffffff) {
    	    	SDL_SetColorKey(image, SDL_SRCCOLORKEY, pixel);
            }
    	    break;
    	}
    }
    // Read the pixels
    pixels_len = w * cpp;
    dst = (Uint8*)image->pixels;
    for (y = 0; y < h; y++) {
    	line = get_next_line(xpmlines, src, pixels_len, &linebuf, &buflen);
    	if (indexed) {
    	    // optimization for some common cases
    	    if (cpp == 1) {
    	    	for (x = 0; x < w; x++) {
    	    	    dst[x] = QUICK_COLORHASH(colors, line + x);
                }
    	    } else {
    	    	for (x = 0; x < w; x++) {
    	    	    dst[x] = get_colorhash(colors, line + x * cpp, cpp);
                }
            }
    	} else {
    	    for (x = 0; x < w; x++) {
    		((Uint32*)dst)[x] = get_colorhash(colors, line + x * cpp, cpp);
            }
    	}
    	dst += image->pitch;
    }
    goto done;
err:
    SDL_FreeSurface(image);
    image = NULL;
done:
    free(keystrings);
    free_colorhash(colors);
    free(linebuf);
    return(image);
}

static SDL_Surface *IMG_LoadXPM_RW(SDL_RWops *src)
{
    if (!src) {
    	// The error message has been set in SDL_RWFromFile
    	return NULL;
    }
    return load_xpm(NULL, src);
}

static SDL_Surface *IMG_ReadXPMFromArray(char **xpm)
{
    return load_xpm(xpm, NULL);
}

#endif // not LOAD_XPM

//
// PNM (portable anymap) image loader:
//
// Supports: PBM, PGM and PPM, ASCII and binary formats
// (PBM and PGM are loaded as 8bpp surfaces)
// Does not support: maximum component value > 255

#ifdef LOAD_PNM

// See if an image is contained in a data source
static int IMG_isPNM(SDL_RWops *src)
{
    char magic[2];
    // PNM magic signatures:
    // P1	PBM, ascii format
    // P2	PGM, ascii format
    // P3	PPM, ascii format
    // P4	PBM, binary format
    // P5	PGM, binary format
    // P6	PPM, binary format
    return (SDL_RWread(src, magic, 2, 1) && magic[0] == 'P' && magic[1] >= '1' && magic[1] <= '6');
}

// read a non-negative integer from the source. return -1 upon error
static int ReadNumber(SDL_RWops *src)
{
    int number = 0;
    unsigned char ch;

    // Skip leading whitespace
    do {
    	if (!SDL_RWread(src, &ch, 1, 1)) {
    	    return(0);
    	}
    	// Eat comments as whitespace
    	if (ch == '#') {  // Comment is '#' to end of line
    	    do {
    	       if (!SDL_RWread(src, &ch, 1, 1) ) {
    		   return -1;
    	       }
    	    } while ( (ch != '\r') && (ch != '\n') );
    	}
    } while (isspace(ch));
    // Add up the number
    do {
       number *= 10;
       number += ch-'0';
       if (!SDL_RWread(src, &ch, 1, 1)) {
    	   return -1;
       }
    } while (isdigit(ch));
    return(number);
}

#define ERROR(s) do { error = (s); goto done; } while(0)

static SDL_Surface *IMG_LoadPNM_RW(SDL_RWops *src)
{
    SDL_Surface *surface = NULL;
    int width, height;
    int maxval, y, bpl;
    Uint8 *row;
    Uint8 *buf = NULL;
    char *error = NULL;
    Uint8 magic[2];
    int ascii, kind;
    //enum { PBM, PGM, PPM } kind;

    if (!src) {
    	// The error message has been set in SDL_RWFromFile
    	return NULL;
    }

    SDL_RWread(src, magic, 2, 1);
    kind = magic[1] - '1';
    ascii = 1;
    if (kind >= 3) {
    	ascii = 0;
    	kind -= 3;
    }
    width = ReadNumber(src);
    height = ReadNumber(src);
    if (width <= 0 || height <= 0)
    	ERROR("Unable to read image width and height");

    if (kind != 0) {
    	maxval = ReadNumber(src);
    	if (maxval <= 0 || maxval > 255)
    	    ERROR("unsupported PNM format");
    } else {
    	maxval = 255;	// never scale PBMs
    }
    // binary PNM allows just a single character of whitespace after
    // the last parameter, and we've already consumed it
    if (kind == 2) {
    	// 24-bit surface in R,G,B byte order
    	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
				   0x000000ff, 0x0000ff00, 0x00ff0000,
#else
				   0x00ff0000, 0x0000ff00, 0x000000ff,
#endif
    				   0);
    } else {
    	// load PBM/PGM as 8-bit indexed images
    	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
    }
    if (surface == NULL)
    	ERROR("Out of memory");
    bpl = width * surface->format->BytesPerPixel;
    if (kind == 1) {
    	SDL_Color *c = surface->format->palette->colors;
    	for (int i = 0; i < 256; i++) {
    	    c[i].r = c[i].g = c[i].b = i;
        }
    	surface->format->palette->ncolors = 256;
    } else
    if (kind == 0) {
    	// for some reason PBM has 1=black, 0=white
    	SDL_Color *c = surface->format->palette->colors;
    	c[0].r = c[0].g = c[0].b = 255;
    	c[1].r = c[1].g = c[1].b = 0;
    	surface->format->palette->ncolors = 2;
    	bpl = (width + 7) >> 3;
    	buf = (Uint8*)malloc(bpl);
    	if (buf == NULL)
    	    ERROR("Out of memory");
    }
    // Read the image into the surface
    row = (Uint8*)surface->pixels;
    for (y = 0; y < height; y++) {
    	if (ascii) {
    	    int i;
    	    if (kind == 0) {
    		for (i = 0; i < width; i++) {
    		    Uint8 ch;
    		    do {
    		    	if (!SDL_RWread(src, &ch, 1, 1))
    	         	    ERROR("file truncated");
    		    	ch -= '0';
    		    } while(ch > 1);
    		    row[i] = ch;
    		}
    	    } else {
    		for (i = 0; i < bpl; i++) {
    		    int c;
    		    c = ReadNumber(src);
    		    if (c < 0)
    			ERROR("file truncated");
    		    row[i] = c;
    		}
    	    }
    	} else {
    	    Uint8 *dst = (kind == 0) ? buf : row;
    	    if (!SDL_RWread(src, dst, bpl, 1))
    	    	ERROR("file truncated");
    	    if (kind == 0) {
    	    	// expand bitmap to 8bpp
    	    	for (int i = 0; i < width; i++) {
    	    	    int bit = 7 - (i & 7);
    	    	    row[i] = (buf[i >> 3] >> bit) & 1;
    	    	}
    	    }
    	}
    	if (maxval < 255) {
    	    // scale up to full dynamic range (slow)
    	    for (int i = 0; i < bpl; i++) {
    	    	row[i] = row[i] * 255 / maxval;
            }
    	}
    	row += surface->pitch;
    }
done:
    free(buf);
    if (error) {
    	SDL_FreeSurface(surface);
    	SDL_SetError(error);
    	surface = NULL;
    }
    return(surface);
}

#endif // LOAD_PNM

/* Table of image detection and loading functions */
static struct {
	char *type;
	int (*is)(SDL_RWops *src);
	SDL_Surface *(*load)(SDL_RWops *src);
} supported[] = {
	/* keep magicless formats first */
#ifdef LOAD_BMP
	{ "BMP", IMG_isBMP, IMG_LoadBMP_RW },
#endif
#ifdef LOAD_PNM
	{ "PNM", IMG_isPNM, IMG_LoadPNM_RW }, /* P[BGP]M share code */
#endif
#ifdef LOAD_XPM
	{ "XPM", IMG_isXPM, IMG_LoadXPM_RW },
#endif
#ifdef LOAD_GIF
	{ "GIF", IMG_isGIF, IMG_LoadGIF_RW },
#endif
#ifdef LOAD_JPG
	{ "JPG", IMG_isJPG, IMG_LoadJPG_RW },
#endif
#ifdef LOAD_PNG
	{ "PNG", IMG_isPNG, IMG_LoadPNG_RW },
#endif
        { 0, 0, 0}
};

// Load an image from an SDL datasource, optionally specifying the type
static SDL_Surface *IMG_LoadTyped_RW(SDL_RWops *src, int freesrc, char *type)
{
    int i, start;
    SDL_Surface *image = 0;

    // Make sure there is something to do..
    if (src == NULL) {
    	SDL_SetError("Passed a NULL data source");
    	return(NULL);
    }
    // See whether or not this data source can handle seeking
    if (SDL_RWseek(src, 0, SEEK_CUR) < 0) {
    	SDL_SetError("Can't seek in this data source");
    	if (freesrc) {
    	    SDL_RWclose(src);
        }
    	return(NULL);
    }
    // Detect the type of image being loaded
    start = SDL_RWtell(src);
    for (i = 0; supported[i].type; ++i) {
    	if (supported[i].is) {
    	    SDL_RWseek(src, start, SEEK_SET);
    	    if (!supported[i].is(src)) {
    		continue;
            }
    	} else {
    	    // magicless format
    	    if (strcasecmp(type, supported[i].type)) {
     		continue;
            }
    	}
    	SDL_RWseek(src, start, SEEK_SET);
    	image = supported[i].load(src);
    	if (freesrc) {
    	    SDL_RWclose(src);
        }
    	return image;
    }
    if (freesrc) {
    	SDL_RWclose(src);
    }
    SDL_SetError("Unsupported image format");
    return NULL;
}
