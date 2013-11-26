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

GUI_Page::GUI_Page(const char *aname, int x, int y, int w, int h):GUI_Container(aname, x, y, w, h)
{
    type = TYPE_PAGE;
    background.style = BACKGROUND_SCALED;
    window = None;
    display = 0;
    ximage = 0;
    gc = 0;
}

GUI_Page::~GUI_Page()
{
    if (window != None) {
        XDestroyWindow(display, window);
    }
    if (ximage) {
        XDestroyImage(ximage);
        surface->GetSurface()->pixels = NULL;
    }
    if (gc) {
        XFreeGC(display, gc);
    }
    if (display) {
        XCloseDisplay(display);
    }
}

void GUI_Page::FlushUpdates(int force)
{
    if (!(GetFlags() & WIDGET_OVERLAY)) {
        return;
    }
    if (window == None) {
        display = XOpenDisplay(systemObject->configGetValue("display", 0));
        if (!display) {
            return;
        }
        if (!area.w) {
            area.w = systemObject->GetScreenWidth();
        }
        if (!area.h) {
            area.h = systemObject->GetScreenHeight();
        }
        visual = systemObject->FindVisual(display, &depth);
        window = systemObject->CreateWindow(display, visual, depth, area.x, area.y, area.w, area.h, 0);
        if (window == None) {
            XCloseDisplay(display);
            display = 0;
            return;
        }
        surface = new GUI_Surface("window", SDL_SWSURFACE, area.w, area.h, 32, visual->red_mask, visual->green_mask, visual->blue_mask, AMask);
        XLockDisplay(display);
        ximage = XCreateImage(display, visual, depth, ZPixmap, 0, (char *)surface->GetSurface()->pixels, area.w, area.h, 32, 0);
        if (ximage) {
            ximage->byte_order = SDL_BYTEORDER == SDL_BIG_ENDIAN ? MSBFirst : LSBFirst;
        }
        XGCValues gcv;
        gcv.graphics_exposures = False;
        gc = XCreateGC(display, window, GCGraphicsExposures, &gcv);
        XMoveResizeWindow(display, window, area.x, area.y, area.w, area.h);
        XUnlockDisplay(display);
    }
    SDL_Rect r = { 0, 0, area.w, area.h};
    systemObject->GetScreen()->GetSurface(0)->Blit(&area, surface, &r);
    XLockDisplay(display);
    XPutImage(display, window, gc, ximage, 0, 0, 0, 0, area.w, area.h);
    XSync(display, False);
    XUnlockDisplay(display);
}

int GUI_Page::OnGotFocus(void)
{
    if (GetFlags() & WIDGET_OVERLAY && window != None) {
        FlushUpdates(1);
        XLockDisplay(display);
        XMapRaised(display, window);
        XSync(display, False);
        XUnlockDisplay(display);
        systemObject->OnExpose();
    }
    return GUI_Container::OnGotFocus();
}

int GUI_Page::OnLostFocus(void)
{
    if (GetFlags() & WIDGET_OVERLAY && window != None) {
        XLockDisplay(display);
        XUnmapWindow(display, window);
        XSync(display, False);
        XUnlockDisplay(display);
        systemObject->OnExpose();
    }
    return GUI_Container::OnLostFocus();
}

