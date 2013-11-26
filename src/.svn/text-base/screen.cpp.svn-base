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

GUI_Screen::GUI_Screen(const char *aname, SDL_Surface * surface):GUI_Container(aname, 0, 0, 0, 0)
{
    type = TYPE_SCREEN;
    screen = 0;
    overlay = 0;
    n_updates = 0;
    updates = new SDL_Rect[MAX_UPDATES];
    area.w = surface->w;
    area.h = surface->h;
    screen = new GUI_Surface("screen", surface);
}

GUI_Screen::~GUI_Screen(void)
{
    if (overlay) {
        overlay->DecRef();
    }
    screen->DecRef();
    delete[]updates;
}

GUI_Surface *GUI_Screen::GetSurface(SDL_Rect *dr)
{
    if (dr) {
        *dr = area;
    }
    return screen;
}

void GUI_Screen::Draw(GUI_Surface * image, const SDL_Rect * src_r, const SDL_Rect * dst_r)
{
    SDL_Rect sr, dr;
    SDL_Rect *srp = 0, *drp = 0;

    if (src_r) {
        sr = *src_r;
        srp = &sr;
    }
    if (dst_r) {
        dr = *dst_r;
        drp = &dr;
    }
    Lock();
    image->Blit(srp, screen, drp);
    if (!screen->IsDoubleBuffered()) {
        UpdateRect(drp);
    }
    Unlock();
}

void GUI_Screen::Fill(const SDL_Rect * dst_r, SDL_Color c)
{
    Uint32 color = screen->MapRGB(c.r, c.g, c.b);
    SDL_Rect r = *dst_r;

    Lock();
    screen->Fill(&r, color);
    if (!screen->IsDoubleBuffered()) {
        UpdateRect(&r);
    }
    Unlock();
}

void GUI_Screen::Erase(const SDL_Rect * area)
{
    Lock();
    GUI_Surface *img = background.img[IDX_NORMAL];
    if (img) {
        if (img->GetWidth() < area->w || img->GetHeight() < area->h) {
            float zw = (float) area->w / img->GetWidth();
            float zh = (float) area->h / img->GetHeight();
            SDL_Surface *surface = img->zoomSurface(img->GetSurface(), zw, zh, SMOOTHING_ON);
            img->SetSurface(surface);
        }
        TileImage(img, area, 0, 0);
    } else {
        screen->Fill(area, screen->MapRGB(color[IDX_NORMAL].r, color[IDX_NORMAL].g, color[IDX_NORMAL].b));
    }
    if (!screen->IsDoubleBuffered()) {
        UpdateRect(area);
    }
    Unlock();
}

void GUI_Screen::FlushUpdates(int force)
{
    if (n_updates) {
        screen->UpdateRects(n_updates, updates);
        n_updates = 0;
    }
}

void GUI_Screen::UpdateRect(const SDL_Rect * r)
{
    if (r->x < 0 || r->y < 0 || r->x + r->w > screen->GetWidth() || r->y + r->h > screen->GetHeight()) {
        systemObject->Log(0, "UpdateRect: error: %dx%dx%dx%d screen 0x0%dx%d", r->x, r->y, r->w, r->h, screen->GetWidth(), screen->GetHeight());
        return;
    }

    for (int i = 0; i < n_updates; i++) {
        if (rectInside(r, &updates[i])) {
            return;
        }
        if (rectInside(&updates[i], r)) {
            updates[i] = *r;
            return;
        }
    }
    updates[n_updates++] = *r;
    if (n_updates >= MAX_UPDATES) {
        FlushUpdates(1);
    }
}

void GUI_Screen::Update(int force)
{
    if (screen->IsDoubleBuffered()) {
        force = 1;
    }
    if (IsChanged()) {
        force = 1;
        SetChanged(0);
    }
    if (force) {
        Erase(&area);
    }
    if (focus_index >= 0) {
        widgets[focus_index]->DoUpdate(force);
    }
    if (overlay) {
        overlay->DoUpdate(force);
    }
    FlushUpdates(force);
    if (screen->IsDoubleBuffered()) {
        screen->Flip();
    }
}

void GUI_Screen::Flip(void)
{
    Lock();
    FlushUpdates(1);
    if (screen->IsDoubleBuffered()) {
        screen->Flip();
    }
    Unlock();
}

void GUI_Screen::OnExpose(void)
{
    GUI_Container::OnExpose();
    if (overlay) {
        overlay->OnExpose();
    }
    SetChanged(1);
}

int GUI_Screen::ShowOverlay(GUI_Page *page)
{
    if (page == overlay) {
        return 0;
    }
    if (overlay) {
        overlay->OnLostFocus();
        overlay->SetFocused(0);
        overlay->ClearFlags(WIDGET_OVERLAY);
        overlay->DecRef();
    }
    overlay = page;
    if (overlay) {
        overlay->IncRef();
        overlay->SetFlags(WIDGET_OVERLAY);
        overlay->OnGotFocus();
        overlay->SetFocused(1);
    }
    return 0;
}

bool GUI_Screen::HandleKeyEvent(const SDL_Event * event)
{
    if (overlay) {
        if (overlay->HandleKeyEvent(event)) {
            return 1;
        }
    } else
    if (focus_index >= 0 && widgets.size()) {
        if (widgets[focus_index]->HandleKeyEvent(event)) {
            return 1;
        }
    }
    return GUI_Widget::HandleKeyEvent(event);
}

bool GUI_Screen::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    if (overlay) {
        if (overlay->HandleMouseEvent(event, overlay->GetX() + xoffset, overlay->GetY() + yoffset)) {
            return 1;
        }
    } else
    if (focus_index >= 0 && widgets.size()) {
        if (widgets[focus_index]->HandleMouseEvent(event, widgets[focus_index]->GetX() + xoffset, widgets[focus_index]->GetY() + yoffset)) {
            return 1;
        }
    }
    return GUI_Widget::HandleMouseEvent(event, xoffset, yoffset);
}

bool GUI_Screen::HandleEvent(const char *action)
{
    if (overlay) {
        if (overlay->HandleEvent(action)) {
            return 1;
        }
    } else
    if (focus_index >= 0 && widgets.size()) {
        if (widgets[focus_index]->HandleEvent(action)) {
            return 1;
        }
    }
    return GUI_Widget::HandleEvent(action);
}

