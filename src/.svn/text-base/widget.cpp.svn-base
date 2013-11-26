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
 */

#include "lmbox.h"

const SDL_Color DEFAULT_COLOR = { 0, 0, 0, 0 };
const SDL_Color DEFAULT_FONTCOLOR = { 0xFF, 0xFF, 0xFF, 0 };
const SDL_Color DEFAULT_SELECTED_FONTCOLOR = { 0xFF, 0xFF, 0x0, 0 };

GUI_Widget::GUI_Widget(const char *aname, int x, int y, int w, int h):DynamicObject(aname)
{
    type = TYPE_WIDGET;
    alpha = SDL_ALPHA_OPAQUE;
    parent = 0;
    align = 0;
    area.x = x;
    area.y = y;
    area.w = w;
    area.h = h;
    areaorig = area;
    contentx = 0;
    contenty = 0;
    ongotfocuscb = 0;
    onlostfocuscb = 0;
    lastclicktime = 0;

    memset(color, 0, sizeof(color));
    memset(&background, 0, sizeof(background));
    memset(&border, 0 , sizeof(border));
    memset(&border.color[1], 255 , sizeof(SDL_Color));
    memset(&border.color[2], 255 , sizeof(SDL_Color));

    background.style = BACKGROUND_NORMAL;

    MemberFunctionProperty < GUI_Widget > *mp;
    mp = new MemberFunctionProperty < GUI_Widget > ("focus", this, &GUI_Widget::pget_Focus, &GUI_Widget::pset_Focus, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("area", this, &GUI_Widget::pget_Area, &GUI_Widget::pset_Area, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("areaorig", this, &GUI_Widget::pget_AreaOrig, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("areanoscale", this, &GUI_Widget::pget_AreaNoScale, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("parentname", this, &GUI_Widget::pget_ParentName, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("x", this, &GUI_Widget::pget_X, &GUI_Widget::pset_X, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("y", this, &GUI_Widget::pget_Y, &GUI_Widget::pset_Y, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("contentx", this, &GUI_Widget::pget_ContentX, &GUI_Widget::pset_ContentX, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("contenty", this, &GUI_Widget::pget_ContentY, &GUI_Widget::pset_ContentY, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("width", this, &GUI_Widget::pget_Width, &GUI_Widget::pset_Width, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("height", this, &GUI_Widget::pget_Height, &GUI_Widget::pset_Height, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("alpha", this, &GUI_Widget::pget_Alpha, &GUI_Widget::pset_Alpha, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("visible", this, &GUI_Widget::pget_Visible, &GUI_Widget::pset_Visible, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("selectable", this, &GUI_Widget::pget_Selectable, &GUI_Widget::pset_Selectable, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("enabled", this, &GUI_Widget::pget_Enabled, &GUI_Widget::pset_Enabled, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("borderstyle", this, &GUI_Widget::pget_BorderStyle, &GUI_Widget::pset_BorderStyle, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("bordercolor", this, &GUI_Widget::pget_BorderColor, &GUI_Widget::pset_BorderColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("abordercolor", this, &GUI_Widget::pget_ABorderColor, &GUI_Widget::pset_ABorderColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("obordercolor", this, &GUI_Widget::pget_OBorderColor, &GUI_Widget::pset_OBorderColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("borderx", this, &GUI_Widget::pget_BorderX, &GUI_Widget::pset_BorderX, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("bordery", this, &GUI_Widget::pget_BorderY, &GUI_Widget::pset_BorderY, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("background", this, &GUI_Widget::pget_Background, &GUI_Widget::pset_Background, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("focusedbackground", this, &GUI_Widget::pget_FocusedBackground, &GUI_Widget::pset_FocusedBackground, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("pressedbackground", this, &GUI_Widget::pget_PressedBackground, &GUI_Widget::pset_PressedBackground, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("disabledbackground", this, &GUI_Widget::pget_DisabledBackground, &GUI_Widget::pset_DisabledBackground, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("backgroundstyle", this, &GUI_Widget::pget_BackgroundStyle, &GUI_Widget::pset_BackgroundStyle, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("color", this, &GUI_Widget::pget_Color, &GUI_Widget::pset_Color, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("focusedcolor", this, &GUI_Widget::pget_FocusedColor, &GUI_Widget::pset_FocusedColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("disabledcolor", this, &GUI_Widget::pget_DisabledColor, &GUI_Widget::pset_DisabledColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("pressedcolor", this, &GUI_Widget::pget_PressedColor, &GUI_Widget::pset_PressedColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("textalign", this, &GUI_Widget::pget_TextAlign, &GUI_Widget::pset_TextAlign, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("ongotfocus", this, &GUI_Widget::pget_OnGotFocus, &GUI_Widget::pset_OnGotFocus, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Widget > ("onlostfocus", this, &GUI_Widget::pget_OnLostFocus, &GUI_Widget::pset_OnLostFocus, false);
    AddProperty(mp);

    MemberMethodHandler < GUI_Widget > *mh;
    mh = new MemberMethodHandler < GUI_Widget > ("setfocus", this, 0, &GUI_Widget::m_SetFocus);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_Widget > ("move", this, 4, &GUI_Widget::m_Move);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_Widget > ("action", this, 1, &GUI_Widget::m_HandleEvent);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_Widget > ("transformscale", this, 3, &GUI_Widget::m_TransformScale);
    AddMethod(mh);
}

GUI_Widget::~GUI_Widget(void)
{
    for (int i = 0; i < IDX_SIZE; i++) {
        if (background.img[i]) {
            background.img[i]->DecRef();
        }
    }
    lmbox_free(ongotfocuscb);
    lmbox_free(onlostfocuscb);
}

SDL_Rect GUI_Widget::Adjust(const SDL_Rect * rp)
{
    Lock();
    SDL_Rect r = *rp;
    r.x += area.x;
    r.y += area.y;
    Unlock();
    return r;
}

void GUI_Widget::RemoveWidget(GUI_Widget * widget)
{
}

bool GUI_Widget::HandleKeyEvent(const SDL_Event *event)
{
    return 0;
}

bool GUI_Widget::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    int x, y, rc = 0;
    switch (event->type) {
     case SDL_MOUSEBUTTONDOWN:
         x = event->button.x - xoffset;
         y = event->button.y - yoffset;
         switch (event->button.button) {
          case SDL_BUTTON_LEFT:
              if (IsSelectable() && Inside(x, y, &area)) {
                  SetPressed(1);
                  SetChanged(1);
                  Uint32 ticks = SDL_GetTicks();
                  if (lastclicktime > 0 && ticks - lastclicktime < DOUBLECLICK_TIME) {
                      DoubleClicked(x, y);
                      lastclicktime = 0;
                  } else {
                      lastclicktime = ticks;
                  }
                  rc = 1;
              }
              break;

          case SDL_BUTTON_WHEELUP:
              break;

          case SDL_BUTTON_WHEELDOWN:
              break;
         }
         break;

     case SDL_MOUSEBUTTONUP:
         x = event->button.x - xoffset;
         y = event->button.y - yoffset;
         switch (event->button.button) {
          case SDL_BUTTON_LEFT:
              if (IsSelectable() && IsPressed() && Inside(x, y, &area)) {
                  Clicked(x, y);
                  rc = 1;
              }
              if (IsPressed()) {
                  SetPressed(0);
                  SetChanged(1);
                  rc = 1;
              }
              break;
         }
         break;

     case SDL_MOUSEMOTION:
         x = event->motion.x - xoffset;
         y = event->motion.y - yoffset;
         if (IsClickable()) {
             SetInside(Inside(x, y, &area));
             SetChanged(1);
         }
         break;
    }
    return rc;
}

void GUI_Widget::Clicked(int x, int y)
{
    if (!IsFocused()) {
        GUI_Page *page = GetPage();
        if (page) {
            page->SetFocusWidget(this);
            page->DecRef();
        }
    }
    SetChanged(1);
}

void GUI_Widget::DoubleClicked(int x, int y)
{
    FireEvent("OnDoubleClick");
}

int GUI_Widget::GetBackgroundIdx(void)
{
    int idx = IDX_NORMAL;
    if (IsDisabled() && background.img[IDX_DISABLED]) {
        idx = IDX_DISABLED;
    } else
    if (IsInside() || IsFocused()) {
        if (IsPressed() && background.img[IDX_PRESSED]) {
            idx = IDX_PRESSED;
        } else
        if (background.img[IDX_FOCUSED]) {
            idx = IDX_FOCUSED;
        }
    }
    return idx;
}

void GUI_Widget::Update(int force)
{
    if (!parent || !force) {
        return;
    }
    if (IsTransparent()) {
        parent->Erase(&area);
    }
    if (force) {
        UpdateBackground();
    }
}

void GUI_Widget::DoUpdate(int force)
{
    Lock();
    if (IsChanged()) {
        SetChanged(0);
        force = 1;
    }
    if (!IsVisible()) {
        force = 0;
    }
    Update(force);
    if (force) {
        UpdateBorder(-2);
    }
    FlushUpdates(force);
    Unlock();
}

void GUI_Widget::FlushUpdates(int force)
{
}

void GUI_Widget::Draw(GUI_Surface * image, const SDL_Rect * sr, const SDL_Rect * dr)
{
    if (parent) {
        SDL_Rect dest = Adjust(dr);
        parent->Draw(image, sr, &dest);
    }
}

void GUI_Widget::Fill(const SDL_Rect * dr, SDL_Color c)
{
    if (parent) {
        SDL_Rect dest = Adjust(dr);
        parent->Fill(&dest, c);
    }
}

void GUI_Widget::Erase(const SDL_Rect * dr)
{
    if (parent) {
        SDL_Rect dest = Adjust(dr);
        parent->Erase(&dest);
    }
}

void GUI_Widget::UpdateBackground(void)
{
    SDL_Rect r = { 0, 0, area.w, area.h };
    for (int i = 0; i < IDX_SIZE; i++) {
        switch (background.style) {
         case BACKGROUND_COLOR:
              if (background.changed[i]) {
                  if (background.img[i]) {
                      background.img[i]->DecRef();
                  }
                  background.changed[i] = 0;
                  background.img[i] = new GUI_Surface("normal", SDL_SWSURFACE, area.w, area.h, 32, RMask, GMask, BMask, AMask);
                  background.img[i]->Fill(&r, color[i]);
              }
              break;

         case BACKGROUND_FILL:
              if (background.changed[i]) {
                  if (background.img[i]) {
                      background.img[i]->DecRef();
                  }
                  background.changed[i] = 0;
                  background.img[i] = new GUI_Surface("fill", SDL_SWSURFACE, area.w, area.h, 32, RMask, GMask, BMask, AMask);
                  UpdateBorder(i);
                  FloodFill(background.img[i]->GetSurface(), r.w/2, r.h / 2, color[i]);
              }
              break;

         case BACKGROUND_GRADIENT:
              if (background.changed[i]) {
                  if (background.img[i]) {
                      background.img[i]->DecRef();
                  }
                  background.changed[i] = 0;
                  background.img[i] = new GUI_Surface("gradient", SDL_SWSURFACE, area.w, area.h, 32, RMask, GMask, BMask, AMask);
                  DrawVGradient2(background.img[i]->GetSurface(), r, color[i], color[IDX_DISABLED], alpha);
              }
              break;

         case BACKGROUND_SCALED:
              if (background.changed[i]) {
                  if (!background.img[i]) {
                      break;
                  }
                  background.changed[i] = 0;
                  float zw = (float) area.w / background.img[i]->GetWidth();
                  float zh = (float) area.h / background.img[i]->GetHeight();
                  SDL_Surface *surface = background.img[i]->zoomSurface(background.img[i]->GetSurface(), zw, zh, SMOOTHING_ON);
                  background.img[i]->SetSurface(surface);
              }
              break;
        }
    }
    int idx = GetBackgroundIdx();
    if (!background.img[idx]) {
        return;
    }
    switch (background.style) {
     case BACKGROUND_NONE:
          break;

     case BACKGROUND_TILED:
          parent->TileImage(background.img[idx], &area, parent->GetX(), parent->GetY());
          break;

     default:
          DrawClipped(background.img[idx], parent, area, area.x, area.y);
    }
}

void GUI_Widget::UpdateBorder(int idx)
{
    SDL_Rect r = { 0, 0, area.w, area.h };
    GUI_Surface *surface = 0;
    if (idx < -1) {
        surface = GetSurface(&r);
    } else {
        if (idx < 0) {
            idx = GetBackgroundIdx();
        }
        if (background.img[idx]) {
            surface = background.img[idx];
        }
    }
    if (!surface) {
        return;
    }
    SDL_Surface *dest = surface->GetSurface();
    SDL_Color color = IsFocused() ? border.color[1] : IsInside() ? border.color[2] : border.color[0];
    switch (border.style) {
     case BORDER_NORMAL:
          DrawRect(dest, r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, color);
          break;

     case BORDER_ROUND:
     case BORDER_ROUND_DBL:
          r.x += border.x;
          r.y += border.y;
          r.w = r.x + r.w - (border.x * 2) - 1;
          r.h = r.y + r.h - (border.y * 2) - 1;
          DrawRoundBorder(dest, r, color, border.style == BORDER_ROUND_DBL);
          break;
    }
}

void GUI_Widget::DrawPixel(SDL_Surface * surface, int x, int y, SDL_Color c)
{
    Uint32 color = SDL_MapRGB(surface->format, c.r, c.g, c.b);

    switch (surface->format->BytesPerPixel) {
     case 4: {
         Uint32 *bufp = (Uint32 *) surface->pixels + y * (surface->pitch >> 2) + x;
         *bufp = color;
         break;
     }

     case 1: {
         Uint8 *bufp = (Uint8 *) surface->pixels + y * surface->pitch + x;
         *bufp = color;
         break;
     }

     case 2: {
         Uint16 *bufp = (Uint16 *) surface->pixels + y * (surface->pitch >> 1) + x;
         *bufp = color;
         break;
     }

     case 3: {
         Uint8 *bufp = (Uint8 *) surface->pixels + y * surface->pitch + x * 3;
         if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
             bufp[0] = color;
             bufp[1] = color >> 8;
             bufp[2] = color >> 16;
         } else {
             bufp[2] = color;
             bufp[1] = color >> 8;
             bufp[0] = color >> 16;
         }
         break;
     }
    }
}

void GUI_Widget::DrawRect(SDL_Surface * screen, int x1, int y1, int x2, int y2, SDL_Color c)
{
    for (int y = y1; y <= y2; y++) {
         DrawPixel(screen, x1, y, c);
         DrawPixel(screen, x2, y, c);
    }
    for (int x = x1; x <= x2; x++) {
         DrawPixel(screen, x, y1, c);
         DrawPixel(screen, x, y2, c);
    }
}

void GUI_Widget::DrawLine(SDL_Surface * screen, int x1, int y1, int x2, int y2, SDL_Color c)
{
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    if (x1 > screen->w) {
        x1 = screen->w - 1;
    }
    if (x2 > screen->w) {
        x2 = screen->w - 1;
    }
    if (y1 > screen->h) {
        y1 = screen->h - 1;
    }
    if (y2 > screen->h) {
        y2 = screen->h - 1;
    }
    dx = x2 - x1;               /* the horizontal distance of the line */
    dy = y2 - y1;               /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = (dx < 0) ? -1 : ((dx > 0) ? 1 : 0);
    sdy = (dy < 0) ? -1 : ((dy > 0) ? 1 : 0);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    // The line is more horizontal than vertical
    if (dxabs >= dyabs) {
        for (i = 0; i < dxabs; i++) {
             y += dyabs;
             if (y >= dxabs) {
                 y -= dxabs;
                 py += sdy;
             }
             px += sdx;
             DrawPixel(screen, px, py, c);
        }
    } else {
        // The line is more vertical than horizontal
        for (i = 0; i < dyabs; i++) {
             x += dxabs;
             if (x >= dyabs) {
                 x -= dyabs;
                 px += sdx;
             }
             py += sdy;
             DrawPixel(screen, px, py, c);
        }
    }
}

void GUI_Widget::DrawHGradient2(SDL_Surface * surf, SDL_Rect gradRect, SDL_Color c1, SDL_Color c2, Uint8 alpha)
{
    int x, width, height;
    Uint8 r, g, b;
    SDL_Rect dest;
    Uint32 pixelcolor;

    width = gradRect.w;
    height = gradRect.h;

    if ((c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b)) {
        SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, c1.r, c1.g, c1.b, alpha));
    } else {
        for (x = 0; x < width; x++) {
             r = (c2.r * x / width) + (c1.r * (width - x) / width);
             g = (c2.g * x / width) + (c1.g * (width - x) / width);
             b = (c2.b * x / width) + (c1.b * (width - x) / width);
             dest.x = gradRect.x + x;
             dest.y = gradRect.y;
             dest.w = 1;
             dest.h = height;
             pixelcolor = SDL_MapRGBA(surf->format, r, g, b, alpha);
             SDL_FillRect(surf, &dest, pixelcolor);
        }
    }
}


void GUI_Widget::DrawVGradient(SDL_Surface * surf, SDL_Color c1, SDL_Color c2, Uint8 alpha)
{
    int y, width, height;
    Uint8 r, g, b;
    SDL_Rect dest;
    Uint32 pixelcolor;

    width = surf->w;
    height = surf->h;

    for (y = 0; y < height; y++) {
         r = (c2.r * y / height) + (c1.r * (height - y) / height);
         g = (c2.g * y / height) + (c1.g * (height - y) / height);
         b = (c2.b * y / height) + (c1.b * (height - y) / height);
         dest.x = 0;
         dest.y = y;
         dest.w = width;
         dest.h = 1;
         pixelcolor = SDL_MapRGBA(surf->format, r, g, b, alpha);
         SDL_FillRect(surf, &dest, pixelcolor);
    }
}

void GUI_Widget::DrawVGradient2(SDL_Surface * surf, SDL_Rect gradRect, SDL_Color c1, SDL_Color c2, Uint8 alpha)
{
    int y, width, height;
    Uint8 r, g, b;
    SDL_Rect dest;
    Uint32 pixelcolor;

    width = gradRect.w;
    height = gradRect.h;

    if ((c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b)) {
        SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, c1.r, c1.g, c1.b, alpha));
    } else {
        for (y = 0; y < height; y++) {
             r = (c2.r * y / height) + (c1.r * (height - y) / height);
             g = (c2.g * y / height) + (c1.g * (height - y) / height);
             b = (c2.b * y / height) + (c1.b * (height - y) / height);
             dest.x = gradRect.x;
             dest.y = gradRect.y + y;
             dest.w = width;
             dest.h = 1;
             pixelcolor = SDL_MapRGBA(surf->format, r, g, b, alpha);
             SDL_FillRect(surf, &dest, pixelcolor);
        }
    }
}

Uint32 GUI_Widget::GetPixel(SDL_Surface * surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
     case 1:
          return *p;

     case 2:
          return *(Uint16 *) p;

     case 3:
          if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
              return p[0] << 16 | p[1] << 8 | p[2];
          } else {
              return p[0] | p[1] << 8 | p[2] << 16;
          }

     case 4:
          return *(Uint32 *) p;

     default:
          return 0;
    }
}

/* Simple floodfill algorithm, credit goes to Paul Heckbert. */
void GUI_Widget::FloodFill(SDL_Surface * screen, int x, int y, SDL_Color c)
{
#define PUSH(Y, XL, XR, DY) if (sp < stack + stack_size && Y + (DY) >= wy1 && Y + (DY) <= wy2) { sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++; }
#define POP(Y, XL, XR, DY) {sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}

    const int stack_size = 10000;
    int wx1 = 0, wx2 = screen->w - 1;
    int wy1 = 0, wy2 = screen->h - 1;
    int l, x1, x2, dy;
    Uint32 ov;                  /* old pixel value */

    /* stack of filled segments */
    struct seg {
      short int y, xl, xr, dy;
    } stack[stack_size], *sp = stack;

    ov = GetPixel(screen, x, y);        /* read pv at seed point */
    if(ov == SDL_MapRGB(screen->format, c.r, c.g, c.b) || x < wx1 || x > wx2 || y < wy1 || y > wy2) {
        return;
    }
    PUSH(y, x, x, 1);           /* needed in some cases */
    PUSH(y + 1, x, x, -1);      /* seed segment (popped 1st) */

    while (sp > stack) {
        /* pop segment off stack and fill a neighboring scan line */
        POP(y, x1, x2, dy);
        /*
         * segment of scan line y-dy for x1<=x<=x2 was previously filled,
         * now explore adjacent pixels in scan line y
         */
        for (x = x1; x >= wx1 && GetPixel(screen, x, y) == ov; x--) {
            DrawPixel(screen, x, y, c);
        }
        if (x >= x1) {
            goto skip;
        }
        l = x + 1;
        if (l < x1) {
            PUSH(y, l, x1 - 1, -dy);    /* leak on left? */
        }
        x = x1 + 1;
        do {
            for (; x <= wx2 && GetPixel(screen, x, y) == ov; x++) {
                DrawPixel(screen, x, y, c);
            }
            PUSH(y, l, x - 1, dy);
            if (x > x2 + 1) {
                PUSH(y, x2 + 1, x - 1, -dy);    /* leak on right? */
            }
        skip:
            for (x++; x <= x2 && GetPixel(screen, x, y) != ov; x++);
            l = x;
        } while(x <= x2);
    }
}

void GUI_Widget::FillTriangle(SDL_Surface * s, int x[3], int y[3], SDL_Color color)
{
#define SWAP(a,b) { t = a; a = b; b = t; }

    int t, i, min;
    double dx1, dx2, dx3;
    double sx, sy, ex, ey;

    /* Sort the points vertically */
    min = 0;
    for (i = 1; i <= 2; i++) {
        if (y[i] < y[min]) {
            SWAP(x[i], x[min]);
            SWAP(y[i], y[min]);
            min = i;
        }
    }
    if (y[1] > y[2]) {
        SWAP(x[1], x[2]);
        SWAP(y[1], y[2]);
    }

    if (y[1] - y[0] > 0) {
        dx1 = ((double) x[1] - x[0]) / ((double) y[1] - y[0]);
    } else {
        dx1 = 0;
    }
    if (y[2] - y[0] > 0) {
        dx2 = ((double) x[2] - x[0]) / ((double) y[2] - y[0]);
    } else {
        dx2 = 0;
    }
    if (y[2] - y[1] > 0) {
        dx3 = ((double) x[2] - x[1]) / ((double) y[2] - y[1]);
    } else {
        dx3 = 0;
    }
    sx = ex = x[0];
    sy = ey = y[0];

    if (dx1 > dx2) {
        for (; sy <= y[1]; sy++, ey++, sx += dx2, ex += dx1) {
            DrawLine(s, (int) sx, (int) sy, (int) ex, (int) sy, color);
        }
        ex = x[1];
        ey = y[1];
        for (; sy <= y[2]; sy++, ey++, sx += dx2, ex += dx3) {
            DrawLine(s, (int) sx, (int) sy, (int) ex, (int) sy, color);
        }
    } else {
        for (; sy <= y[1]; sy++, ey++, sx += dx1, ex += dx2) {
            DrawLine(s, (int) sx, (int) sy, (int) ex, (int) sy, color);
        }
        sx = x[1];
        sy = y[1];
        for (; sy <= y[2]; sy++, ey++, sx += dx3, ex += dx2) {
            DrawLine(s, (int) sx, (int) sy, (int) ex, (int) sy, color);
        }
    }
}

void GUI_Widget::DrawRoundBorder(SDL_Surface *dest, SDL_Rect r, SDL_Color c, bool dbl)
{
    DrawPixel(dest, r.x + 2, r.y + 2, c);
    DrawPixel(dest, r.x + 2, r.h - 2, c);
    DrawPixel(dest, r.w - 2, r.y + 2, c);
    DrawPixel(dest, r.w - 2, r.h - 2, c);
    DrawLine(dest, r.x + 2, r.y + 1, r.w - 3, r.y + 1, c);
    DrawLine(dest, r.x + 1, r.y + 2, r.x + 1, r.h - 3, c);
    DrawLine(dest, r.w - 1, r.y + 2, r.w - 1, r.h - 3, c);
    DrawLine(dest, r.x + 2, r.h - 1, r.w - 3, r.h - 1, c);
    if (dbl) {
        DrawLine(dest, r.x + 2, r.y + 2, r.x + 2, r.h - 3, c);
        DrawLine(dest, r.w - 2, r.y + 2, r.w - 2, r.h - 3, c);
        DrawLine(dest, r.x + 2, r.y + 2, r.w - 3, r.y + 2, c);
        DrawLine(dest, r.x + 2, r.h - 2, r.w - 3, r.h - 2, c);
    }
}

void GUI_Widget::DrawTriangle(SDL_Surface * s, int x[3], int y[3], SDL_Color c)
{
    DrawLine(s, x[0], y[0], x[1], y[1], c);
    DrawLine(s, x[0], y[0], x[2], y[2], c);
    DrawLine(s, x[2], y[2], x[1], y[1], c);
}

void GUI_Widget::DrawArrow(SDL_Surface * s, int type, int x, int y, int a, SDL_Color color, bool fill, SDL_Color fillcolor)
{
    int ax[3], ay[3];

    switch (type) {
     case ARROW_UP:
          ax[0] = x - a / 2;
          ay[0] = y + a / 3;
          ax[1] = x;
          ay[1] = y - 2 * a / 3;
          ax[2] = x + a / 2;
          ay[2] = y + a / 3;
          break;

     case ARROW_DOWN:
          ax[0] = x - a / 2;
          ay[0] = y - 2 * a / 3;
          ax[1] = x;
          ay[1] = y + a / 3;
          ax[2] = x + a / 2;
          ay[2] = y - 2 * a / 3;
          break;

     case ARROW_LEFT:
          ax[0] = x - a / 3;
          ay[0] = y - a / 2;
          ax[1] = x + 2 * a / 3;
          ay[1] = y;
          ax[2] = x - a / 3;
          ay[2] = y + a / 2;
          break;

     case ARROW_RIGHT:
          ax[0] = x + a / 3;
          ay[0] = y - a / 2;
          ax[1] = x - 2 * a / 3;
          ay[1] = y;
          ax[2] = x + a / 3;
          ay[2] = y + a / 2;
          break;
    }
    DrawTriangle(s, ax, ay, color);
    if (fill) {
        FillTriangle(s, ax, ay, fillcolor);
    }
}

void GUI_Widget::TileImage(GUI_Surface * surface, const SDL_Rect * rp, int x_offset, int y_offset)
{
    SDL_Rect sr, dr;
    int xp, yp, bw, bh;

    bw = surface->GetWidth();
    bh = surface->GetHeight();

    for (xp = 0; xp < rp->w; xp += sr.w) {
        dr.x = rp->x + xp;
        sr.x = (dr.x + x_offset) % bw;
        sr.w = dr.w = bw - sr.x;
        if (dr.x + dr.w > rp->x + rp->w) {
            sr.w = dr.w = rp->x + rp->w - dr.x;
        }
        for (yp = 0; yp < rp->h; yp += sr.h) {
            dr.y = rp->y + yp;
            sr.y = (dr.y + y_offset) % bh;
            sr.h = dr.h = bh - sr.y;
            if (dr.y + dr.h > rp->y + rp->h) {
                sr.h = dr.h = rp->y + rp->h - dr.y;
            }
            Draw(surface, &sr, &dr);
        }
    }
}

void GUI_Widget::DrawClipped(GUI_Surface * source, GUI_Widget * dest, SDL_Rect destarea, int destx, int desty)
{
    SDL_Rect dr;
    SDL_Rect sr;

    sr.w = dr.w = source->GetWidth();
    sr.h = dr.h = source->GetHeight();
    sr.x = sr.y = 0;

    dr.x = destx;
    dr.y = desty;
    if (ClipRect(&sr, &dr, &destarea)) {
        dest->Draw(source, &sr, &dr);
    }
}

int GUI_Widget::ClipRect(SDL_Rect * sr, SDL_Rect * dr, SDL_Rect * clip)
{
    int dx = dr->x;
    int dy = dr->y;
    int dw = dr->w;
    int dh = dr->h;
    int cx = clip->x;
    int cy = clip->y;
    int cw = clip->w;
    int ch = clip->h;
    int adj = cx - dx;
    if (adj > 0) {
        if (adj > dw) {
            return 0;
        }
        dx += adj;
        dw -= adj;
        if (sr != NULL) {
            sr->x += adj;
            sr->w -= adj;
        }
    }
    adj = cy - dy;
    if (adj > 0) {
        if (adj > dh) {
            return 0;
        }
        dy += adj;
        dh -= adj;
        if (sr != NULL) {
            sr->y += adj;
            sr->h -= adj;
        }
    }
    adj = (dx + dw) - (cx + cw);
    if (adj > 0) {
        if (adj > dw) {
            return 0;
        }
        dw -= adj;
        if (sr != NULL) {
            sr->w -= adj;
        }
    }
    adj = (dy + dh) - (cy + ch);
    if (adj > 0) {
        if (adj > dh) {
            return 0;
        }
        dh -= adj;
        if (sr != NULL) {
            sr->h -= adj;
        }
    }
    dr->x = dx;
    dr->y = dy;
    dr->w = dw;
    dr->h = dh;
    return 1;
}

int GUI_Widget::Keep(Object ** target, Object * source)
{
    int rc = 0;
    Lock();
    if (source != *target) {
        if (source) {
            source->IncRef();
        }
        if (*target) {
            (*target)->SetParent(0);
            (*target)->DecRef();
        }
        if (source) {
            source->SetParent(this);
        }
        (*target) = source;
        rc = 1;
    }
    Unlock();
    SetChanged(1);
    return 1;
}

void GUI_Widget::SetBorderX(int pixels)
{
    this->border.x = pixels;
    SetChanged(1);
}

void GUI_Widget::SetBorderY(int pixels)
{
    this->border.y = pixels;
    SetChanged(1);
}

SDL_Rect GUI_Widget::GetArea(void)
{
    SDL_Rect r = GetAreaRaw();
    r.x = (int)(r.x / systemObject->GetXScale());
    r.y = (int)(r.y / systemObject->GetYScale());
    r.w = (int)(r.w / systemObject->GetXScale());
    r.h = (int)(r.h / systemObject->GetYScale());
    return r;
}

SDL_Rect GUI_Widget::GetAreaRaw(void)
{
    Lock();
    SDL_Rect r = area;
    Unlock();
    return r;
}

SDL_Rect GUI_Widget::GetAreaOrig(void)
{
    Lock();
    SDL_Rect r = areaorig;
    Unlock();
    return r;
}

void GUI_Widget::SetArea(SDL_Rect r)
{
    SetPosition(r.x, r.y, r.w, r.h);
    if (!IsBusy()) {
        OnMove();
        OnResize();
    }
}

void GUI_Widget::SetPosition(int x, int y, int w, int h)
{
    Lock();
    SDL_Rect r = area;
    if (x >= 0) {
        areaorig.x = x;
        area.x = (int)(x * systemObject->GetXScale());
    }
    if (y >= 0) {
        areaorig.y = y;
        area.y = (int)(y * systemObject->GetYScale());
    }
    if (w >= 0) {
        areaorig.w = w;
        area.w = (int)(w * systemObject->GetXScale());
    }
    if (h >= 0) {
        areaorig.h = h;
        area.h = (int)(h * systemObject->GetYScale());
    }
    if (background.style != BACKGROUND_NONE && (r.w != area.w || r.h != area.h)) {
        for (int i = 0; i < IDX_SIZE; i++) {
             background.changed[i] = 1;
        }
    }
    Unlock();
    SetChanged(1);
    if (!IsBusy()) {
        if (r.x != area.x || r.y != area.y) {
            OnMove();
        }
        if (r.w != area.w || r.h != area.h) {
            OnResize();
        }
    }
}

int GUI_Widget::GetAlpha(void)
{
    return alpha;
}

void GUI_Widget::SetAlpha(int a)
{
    alpha = a;
    SetChanged(1);
}

int GUI_Widget::GetX(void)
{
    return area.x;
}

int GUI_Widget::GetY(void)
{
    return area.y;
}

int GUI_Widget::GetWidth(void)
{
    return area.w;
}

int GUI_Widget::GetHeight(void)
{
    return area.h;
}

int GUI_Widget::GetContentX(void)
{
    return contentx;
}

int GUI_Widget::GetContentY(void)
{
    return contenty;
}

void GUI_Widget::SetX(int value)
{
    this->SetPosition(value, -1, -1, -1);
}

void GUI_Widget::SetY(int value)
{
    this->SetPosition(-1, value, -1, -1);
}

void GUI_Widget::SetContentX(int value)
{
    this->contentx = value;
    SetChanged(1);
}

void GUI_Widget::SetContentY(int value)
{
    this->contenty = value;
    SetChanged(1);
}

void GUI_Widget::SetWidth(int value)
{
    SetPosition(-1, -1, value, -1);
}

void GUI_Widget::SetHeight(int value)
{
    SetPosition(-1, -1, -1, value);
}

void GUI_Widget::SetBorderStyle(int atype)
{
    border.style = atype;
    SetChanged(1);
}

int GUI_Widget::GetBorderStyle(void)
{
    return border.style;
}

void GUI_Widget::SetBorderColor(SDL_Color c)
{
    border.color[0] = c;
    SetChanged(1);
}

SDL_Color GUI_Widget::GetBorderColor(void)
{
    return border.color[0];
}

void GUI_Widget::SetABorderColor(SDL_Color c)
{
    border.color[1] = c;
    SetChanged(1);
}

SDL_Color GUI_Widget::GetABorderColor(void)
{
    return border.color[1];
}

void GUI_Widget::SetOBorderColor(SDL_Color c)
{
    border.color[2] = c;
    SetChanged(1);
}

SDL_Color GUI_Widget::GetOBorderColor(void)
{
    return border.color[2];
}

void GUI_Widget::SetBackgroundStyle(int atype)
{
    Lock();
    background.style = atype;
    if (background.style != BACKGROUND_NONE) {
        for (int i = 0; i < IDX_SIZE; i++) {
             background.changed[i] = 1;
        }
    }
    Unlock();
    SetChanged(1);
}

int GUI_Widget::GetBackgroundStyle(void)
{
    return background.style;
}

GUI_Surface *GUI_Widget::GetBackground(void)
{
    return background.img[IDX_NORMAL];
}

void GUI_Widget::SetBackground(GUI_Surface * surface)
{
    Keep((Object **) & background.img[IDX_NORMAL], surface);
    background.changed[IDX_NORMAL] = 1;
}

void GUI_Widget::SetFocusedBackground(GUI_Surface * surface)
{
    Keep((Object **) & background.img[IDX_FOCUSED], surface);
    background.changed[IDX_FOCUSED] = 1;
}

void GUI_Widget::SetPressedBackground(GUI_Surface * surface)
{
    Keep((Object **) & background.img[IDX_PRESSED], surface);
    background.changed[IDX_PRESSED] = 1;
}

void GUI_Widget::SetDisabledBackground(GUI_Surface * surface)
{
    Keep((Object **) & background.img[IDX_DISABLED], surface);
    background.changed[IDX_DISABLED] = 1;
}

void GUI_Widget::SetColor(SDL_Color c)
{
    Lock();
    color[IDX_NORMAL] = c;
    background.changed[IDX_NORMAL] = 1;
    Unlock();
    SetFocusedColor(c);
}

SDL_Color GUI_Widget::GetColor(void)
{
    return color[IDX_NORMAL];
}

void GUI_Widget::SetFocusedColor(SDL_Color c)
{
    Lock();
    color[IDX_FOCUSED] = c;
    background.changed[IDX_FOCUSED] = 1;
    Unlock();
    SetChanged(1);
}

SDL_Color GUI_Widget::GetFocusedColor(void)
{
    return color[IDX_FOCUSED];
}

void GUI_Widget::SetPressedColor(SDL_Color c)
{
    Lock();
    color[IDX_PRESSED] = c;
    background.changed[IDX_PRESSED] = 1;
    Unlock();
    SetChanged(1);
}

SDL_Color GUI_Widget::GetPressedColor(void)
{
    return color[IDX_PRESSED];
}

void GUI_Widget::SetDisabledColor(SDL_Color c)
{
    Lock();
    color[IDX_DISABLED] = c;
    background.changed[IDX_DISABLED] = 1;
    Unlock();
    SetChanged(1);
}

SDL_Color GUI_Widget::GetDisabledColor(void)
{
    return color[IDX_DISABLED];
}

int GUI_Widget::OnGotFocus(void)
{
    FireEvent("OnGotFocus");
    SetInside(1);
    // Custom onetime focus callback
    if (ongotfocuscb) {
        Script script(name, ongotfocuscb);
        script.Execute();
        SetOnGotFocusCb(0);
    }
    SetChanged(1);
    return 1;
}

int GUI_Widget::OnLostFocus(void)
{
    FireEvent("OnLostFocus");
    SetInside(0);
    // Custom onetime focus callback
    if (onlostfocuscb) {
        Script script(name, onlostfocuscb);
        script.Execute();
        SetOnLostFocusCb(0);
    }
    SetChanged(1);
    return 1;
}

void GUI_Widget::OnResize(void)
{
}

void GUI_Widget::OnTransform(void)
{
}

void GUI_Widget::OnMove(void)
{
}

void GUI_Widget::OnClick(void)
{
}

int GUI_Widget::OnChangeFocus(void)
{
    return 0;
}

int GUI_Widget::OnMouseOver(void)
{
    return FireEvent("OnMouseOver");
}

int GUI_Widget::OnMouseOut(void)
{
    return FireEvent("OnMouseOut");
}

int GUI_Widget::IsSelectable()
{
    return (GetFlags() & WIDGET_SELECTABLE) && IsVisible() && !IsDisabled();
}

int GUI_Widget::IsClickable()
{
    return IsVisible() && !IsDisabled();
}

int GUI_Widget::IsVisible(void)
{
    return !(GetFlags() & WIDGET_HIDDEN);
}

int GUI_Widget::IsInside(void)
{
    return (GetFlags() & WIDGET_INSIDE);
}

int GUI_Widget::IsPressed(void)
{
    return (GetFlags() & WIDGET_PRESSED);
}

int GUI_Widget::IsDisabled(void)
{
    return (GetFlags() & WIDGET_DISABLED);
}

int GUI_Widget::IsFocused(void)
{
    return (GetFlags() & WIDGET_FOCUSED);
}

int GUI_Widget::IsTransparent(void)
{
    return (GetFlags() & WIDGET_TRANSPARENT);
}

void GUI_Widget::SetFocused(bool flag)
{
    if (flag) {
        if (!IsFocused()) {
            SetFlags(WIDGET_FOCUSED);
            OnChangeFocus();
        }
    } else {
        if (IsFocused()) {
            ClearFlags(WIDGET_FOCUSED);
            OnChangeFocus();
        }
    }
}

void GUI_Widget::SetInside(bool flag)
{
    if (flag) {
        if (!IsInside()) {
            SetFlags(WIDGET_INSIDE);
            OnMouseOver();
        }
    } else {
        if (IsInside()) {
            ClearFlags(WIDGET_INSIDE);
            OnMouseOut();
        }
    }
}

void GUI_Widget::SetPressed(bool flag)
{
    if (flag) {
        SetFlags(WIDGET_PRESSED);
    } else {
        ClearFlags(WIDGET_PRESSED);
    }
}

void GUI_Widget::SetVisible(bool flag)
{
    if (flag) {
        ClearFlags(WIDGET_HIDDEN);
    } else {
        SetFlags(WIDGET_HIDDEN);
    }
}

void GUI_Widget::SetSelectable(bool flag)
{
    if (flag) {
        SetFlags(WIDGET_SELECTABLE);
    } else {
        ClearFlags(WIDGET_SELECTABLE);
    }
}

void GUI_Widget::SetTransparent(bool flag)
{
    if (flag) {
        SetFlags(WIDGET_TRANSPARENT);
    } else {
        ClearFlags(WIDGET_TRANSPARENT);
    }
}

void GUI_Widget::SetEnabled(bool flag)
{
    if (flag) {
        ClearFlags(WIDGET_DISABLED);
    } else {
        SetFlags(WIDGET_DISABLED);
    }
}

int GUI_Widget::GetAlign(void)
{
    return align;
}

void GUI_Widget::SetAlign(int flag)
{
    align = flag;
}

void GUI_Widget::SetTextAlign(const char *textalign)
{
    if (!strcmp(textalign, "vtop")) {
        SetAlign((align & ~ALIGN_VERT_MASK)  | ALIGN_VERT_TOP);
    } else
    if (!strcmp(textalign, "vcenter")) {
        SetAlign((align & ~ALIGN_VERT_MASK) | ALIGN_VERT_CENTER);
    } else
    if (!strcmp(textalign, "vbottom")) {
        SetAlign((align & ~ALIGN_VERT_MASK) | ALIGN_VERT_BOTTOM);
    } else
    if (!strcmp(textalign, "hcenter")) {
        SetAlign((align & ~ALIGN_HORIZ_MASK) | ALIGN_HORIZ_CENTER);
    } else
    if (!strcmp(textalign, "hleft")) {
        SetAlign((align & ~ALIGN_HORIZ_MASK) | ALIGN_HORIZ_LEFT);
    } else
    if (!strcmp(textalign, "hright")) {
        SetAlign((align & ~ALIGN_HORIZ_MASK) | ALIGN_HORIZ_RIGHT);
    } else
    if (!strcmp(textalign, "topleft")) {
        SetAlign(ALIGN_VERT_TOP | ALIGN_HORIZ_LEFT);
    } else
    if (!strcmp(textalign, "topright")) {
        SetAlign(ALIGN_VERT_TOP | ALIGN_HORIZ_RIGHT);
    } else
    if (!strcmp(textalign, "bottomleft")) {
        SetAlign(ALIGN_VERT_BOTTOM | ALIGN_HORIZ_LEFT);
    } else
    if (!strcmp(textalign, "bottomright")) {
        SetAlign(ALIGN_VERT_TOP | ALIGN_HORIZ_RIGHT);
    } else
    if (!strcmp(textalign, "top")) {
        SetAlign(ALIGN_VERT_TOP | ALIGN_HORIZ_CENTER);
    } else
    if (!strcmp(textalign, "bottom")) {
        SetAlign(ALIGN_VERT_BOTTOM | ALIGN_HORIZ_CENTER);
    } else
    if (!strcmp(textalign, "center")) {
        SetAlign(ALIGN_VERT_CENTER | ALIGN_HORIZ_CENTER);
    } else
    if (!strcmp(textalign, "right")) {
        SetAlign(ALIGN_VERT_CENTER | ALIGN_HORIZ_RIGHT);
    } else {
        SetAlign(ALIGN_VERT_CENTER | ALIGN_HORIZ_LEFT);
    }
    SetChanged(1);
}

const char *GUI_Widget::GetTextAlign(void)
{
    int flag = GetAlign();
    return flag & (ALIGN_VERT_TOP | ALIGN_HORIZ_CENTER) ? "top" :
           flag & (ALIGN_VERT_BOTTOM | ALIGN_HORIZ_CENTER) ? "bottom" :
           flag & (ALIGN_VERT_CENTER | ALIGN_HORIZ_CENTER) ? "center" :
           flag & (ALIGN_VERT_TOP | ALIGN_HORIZ_RIGHT) ? "right" :
           flag & (ALIGN_VERT_TOP | ALIGN_HORIZ_LEFT) ? "left" :
           flag & (ALIGN_VERT_TOP | ALIGN_HORIZ_LEFT) ? "topleft" :
           flag & (ALIGN_VERT_TOP | ALIGN_HORIZ_RIGHT) ? "topright" :
           flag & (ALIGN_VERT_BOTTOM | ALIGN_HORIZ_LEFT) ? "bottomleft" :
           flag & (ALIGN_VERT_TOP | ALIGN_HORIZ_RIGHT) ? "bottomright" :
           flag & ALIGN_VERT_TOP ? "vtop" :
           flag & ALIGN_VERT_TOP ? "vbottom" :
           flag & ALIGN_VERT_TOP ? "vcenter" :
           flag & ALIGN_VERT_TOP ? "hleft" :
           flag & ALIGN_VERT_TOP ? "hright" :
           flag & ALIGN_VERT_TOP ? "hcenter" : "";
}

void GUI_Widget::ClearParent(void)
{
    GUI_Page *page = GetPage();
    if (page) {
        page->RemoveWidget(this);
        page->DecRef();
    }
}

GUI_Widget *GUI_Widget::GetParent(void)
{
    return parent;
}

void GUI_Widget::SetParent(Object *aparent)
{
    parent = (GUI_Widget*)aparent;
}

GUI_Page *GUI_Widget::GetPage(void)
{
    GUI_Widget *w = (GUI_Widget*)parent;
    while(w) {
      if (!strcmp(w->GetType(), "page")) {
          break;
      }
      w = w->GetParent();
    }
    if (w) {
        w->IncRef();
    }
    return (GUI_Page*)w;
}

GUI_Surface *GUI_Widget::GetSurface(SDL_Rect *dr)
{
    SDL_Rect dest = area;
    GUI_Widget *w = (GUI_Widget*)parent;
    while(w) {
      if (!strcmp(w->GetType(), "screen")) {
          break;
      }
      dest = w->Adjust(&dest);
      w = w->GetParent();
    }
    if (w) {
        dest = w->Adjust(&dest);
        if (dr) {
            *dr = dest;
        }
        return w->GetSurface(0);
    }
    return 0;
}

void GUI_Widget::SetOnGotFocusCb(const char *val)
{
    Lock();
    lmbox_free(ongotfocuscb);
    ongotfocuscb = lmbox_strdup(val);
    Unlock();
}

void GUI_Widget::SetOnLostFocusCb(const char *val)
{
    Lock();
    lmbox_free(onlostfocuscb);
    onlostfocuscb = lmbox_strdup(val);
    Unlock();
}

Variant GUI_Widget::pget_ParentName(void)
{
    return anytovariant(parent ? parent->GetName() : "");
}

Variant GUI_Widget::pget_OnGotFocus(void)
{
    return anytovariant(ongotfocuscb ? ongotfocuscb : "");
}

int GUI_Widget::pset_OnGotFocus(Variant value)
{
    SetOnGotFocusCb(value);
}

Variant GUI_Widget::pget_OnLostFocus()
{
   return anytovariant(onlostfocuscb ? onlostfocuscb : "");
}

int GUI_Widget::pset_OnLostFocus(Variant value)
{
    SetOnLostFocusCb(value);
}

Variant GUI_Widget::pget_Alpha(void)
{
    return anytovariant(GetAlpha());
}

int GUI_Widget::pset_Alpha(Variant value)
{
    SetAlpha(value);
    return 0;
}

Variant GUI_Widget::pget_Area(void)
{
    char rc[64];
    SDL_Rect r = GetArea();
    sprintf(rc, "%d %d %d %d", r.x, r.y, r.w, r.h);
    return anytovariant(rc);
}

int GUI_Widget::pset_Area(Variant value)
{
    SDL_Rect r;
    if (sscanf(value, "%u %u %u %u", &r.x, &r.y, &r.w, &r.h) == 4) {
        SetArea(r);
    }
    return 0;
}

Variant GUI_Widget::pget_AreaNoScale(void)
{
    char rc[64];
    SDL_Rect r = GetAreaRaw();
    sprintf(rc, "%d %d %d %d", r.x, r.y, r.w, r.h);
    return anytovariant(rc);
}

Variant GUI_Widget::pget_AreaOrig(void)
{
    char rc[64];
    SDL_Rect r = GetAreaOrig();
    sprintf(rc, "%d %d %d %d", r.x, r.y, r.w, r.h);
    return anytovariant(rc);
}

Variant GUI_Widget::pget_X(void)
{
    return anytovariant(GetX());
}

int GUI_Widget::pset_X(Variant value)
{
    SetX(value);
    return 0;
}

Variant GUI_Widget::pget_Y(void)
{
    return anytovariant(GetY());
}

int GUI_Widget::pset_Y(Variant value)
{
    SetY(value);
    return 0;
}

Variant GUI_Widget::pget_ContentX(void)
{
    return anytovariant(GetContentX());
}

int GUI_Widget::pset_ContentX(Variant value)
{
    SetContentX(value);
    return 0;
}

Variant GUI_Widget::pget_ContentY(void)
{
    return anytovariant(GetContentY());
}

int GUI_Widget::pset_ContentY(Variant value)
{
    SetContentY(value);
    return 0;
}

Variant GUI_Widget::pget_BorderStyle(void)
{
    return anytovariant(borderIntToStr(GetBorderStyle()));
}

int GUI_Widget::pset_BorderStyle(Variant value)
{
    SetBorderStyle(borderStrToInt(value));
    return 0;
}

Variant GUI_Widget::pget_BorderColor(void)
{
    return anytovariant(colorToStr(GetBorderColor()));
}

int GUI_Widget::pset_BorderColor(Variant value)
{
    SetBorderColor(strToColor(value));
    return 0;
}

Variant GUI_Widget::pget_ABorderColor(void)
{
    return anytovariant(colorToStr(GetABorderColor()));
}

int GUI_Widget::pset_ABorderColor(Variant value)
{
    SetABorderColor(strToColor(value));
    return 0;
}

Variant GUI_Widget::pget_OBorderColor(void)
{
    return anytovariant(colorToStr(GetOBorderColor()));
}

int GUI_Widget::pset_OBorderColor(Variant value)
{
    SetOBorderColor(strToColor(value));
    return 0;
}

Variant GUI_Widget::pget_Width(void)
{
    return anytovariant(area.w);
}

int GUI_Widget::pset_Width(Variant value)
{
    SetWidth(value);
    return 0;
}

Variant GUI_Widget::pget_Height(void)
{
    return anytovariant(area.h);
}

int GUI_Widget::pset_Height(Variant value)
{
    SetHeight(value);
    return 0;
}

Variant GUI_Widget::pget_Visible(void)
{
    return anytovariant(IsVisible());
}

int GUI_Widget::pset_Visible(Variant value)
{
    SetVisible(value);
    return 0;
}

Variant GUI_Widget::pget_Selectable(void)
{
    return anytovariant(IsSelectable());
}

int GUI_Widget::pset_Selectable(Variant value)
{
    SetSelectable(value);
    return 0;
}

Variant GUI_Widget::pget_Enabled(void)
{
    return anytovariant(!IsDisabled());
}

int GUI_Widget::pset_Enabled(Variant value)
{
    SetEnabled(value);
    return 0;
}

Variant GUI_Widget::pget_Focus(void)
{
    return anytovariant(IsFocused());
}

int GUI_Widget::pset_Focus(Variant value)
{
    int rc = value;
    GUI_Page *page = GetPage();
    if (page) {
        if (rc) {
            page->SetFocusWidget(this);
        } else {
            page->UpdateFocusWidget(1);
        }
        page->DecRef();
    }
    return anytovariant(IsFocused());
}

Variant GUI_Widget::pget_BorderX(void)
{
    return anytovariant(border.x);
}

int GUI_Widget::pset_BorderX(Variant value)
{
    SetBorderX(value);
    return 0;
}

Variant GUI_Widget::pget_BorderY(void)
{
    return anytovariant(border.y);
}

int GUI_Widget::pset_BorderY(Variant value)
{
    SetBorderY(value);
    return 0;
}

Variant GUI_Widget::pget_TextAlign(void)
{
    return anytovariant(GetTextAlign());
}

int GUI_Widget::pset_TextAlign(Variant value)
{
    SetTextAlign(value);
    return 0;
}

Variant GUI_Widget::pget_BackgroundStyle(void)
{
    return anytovariant(styleIntToStr(GetBackgroundStyle()));
}

Variant GUI_Widget::pget_Background(void)
{
    if (background.img[IDX_NORMAL]) {
        return anytovariant(background.img[IDX_NORMAL]->GetName());
    }
    return VARNULL;
}

int GUI_Widget::pset_Background(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    SetBackground(image);
    return 0;
}

Variant GUI_Widget::pget_FocusedBackground(void)
{
    if (background.img[IDX_FOCUSED]) {
        return anytovariant(background.img[IDX_FOCUSED]->GetName());
    }
    return VARNULL;
}

int GUI_Widget::pset_FocusedBackground(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    SetFocusedBackground(image);
    return 0;
}

Variant GUI_Widget::pget_PressedBackground(void)
{
    if (background.img[IDX_PRESSED]) {
        return anytovariant(background.img[IDX_PRESSED]->GetName());
    }
    return VARNULL;
}

int GUI_Widget::pset_PressedBackground(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    SetPressedBackground(image);
    return 0;
}

Variant GUI_Widget::pget_DisabledBackground(void)
{
    if (background.img[IDX_DISABLED]) {
        return anytovariant(background.img[IDX_DISABLED]->GetName());
    }
    return VARNULL;
}

int GUI_Widget::pset_DisabledBackground(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    SetDisabledBackground(image);
    return 0;
}

int GUI_Widget::pset_BackgroundStyle(Variant value)
{
    SetBackgroundStyle(styleStrToInt(value));
    return 0;
}

Variant GUI_Widget::pget_Color(void)
{
    return anytovariant(colorToStr(GetColor()));
}

int GUI_Widget::pset_Color(Variant value)
{
    SetColor(strToColor(value));
    return 0;
}

Variant GUI_Widget::pget_FocusedColor(void)
{
    return anytovariant(colorToStr(GetFocusedColor()));
}

int GUI_Widget::pset_FocusedColor(Variant value)
{
    SetFocusedColor(strToColor(value));
    return 0;
}

Variant GUI_Widget::pget_DisabledColor(void)
{
    return anytovariant(colorToStr(GetDisabledColor()));
}

int GUI_Widget::pset_DisabledColor(Variant value)
{
    SetDisabledColor(strToColor(value));
    return 0;
}

Variant GUI_Widget::pget_PressedColor(void)
{
    return anytovariant(colorToStr(GetPressedColor()));
}

int GUI_Widget::pset_PressedColor(Variant value)
{
    SetPressedColor(strToColor(value));
    return 0;
}

Variant GUI_Widget::m_SetFocus(int numargs, Variant args[])
{
    GUI_Page *page = GetPage();
    if (page) {
        page->SetFocusWidget(this);
        page->DecRef();
    }
    return VARNULL;
}

Variant GUI_Widget::m_Move(int numargs, Variant args[])
{
    SetPosition(args[0], args[1], args[2], args[3]);
    return VARNULL;
}

Variant GUI_Widget::m_HandleEvent(int numargs, Variant args[])
{
    HandleEvent(args[0]);
    return VARNULL;
}

Variant GUI_Widget::m_TransformScale(int numargs, Variant args[])
{
    int count = args[0], step = args[1], delay = args[2];

    for (int i = 0; i < count; i++) {
        SDL_Rect r = GetArea();
        r.x += -1*step;
        r.y += -1*step;
        r.w += 2*step;
        r.h += 2*step;
        SetArea(r);
        OnTransform();
        systemObject->DoUpdate(0);
        SDL_Delay(delay);
    }
    ToggleFlags(WIDGET_TRANSFORM);
    return VARNULL;
}

