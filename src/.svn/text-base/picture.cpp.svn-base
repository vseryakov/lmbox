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
 *
 *  rotozoomer for 32bit surfaces by (c) A. Schiffler
 *
 */

#include "lmbox.h"

GUI_Picture::GUI_Picture(const char *aname, int x, int y, int w, int h, GUI_Surface * an_image):GUI_Widget(aname, x, y, w, h), image(an_image)
{
    type = TYPE_PICTURE;
    caption = 0;
    SetTransparent(1);

    if (image) {
        image->IncRef();
    }

    MemberFunctionProperty < GUI_Picture > *mp;
    mp = new MemberFunctionProperty < GUI_Picture > ("image", this, &GUI_Picture::pget_Image, &GUI_Picture::pset_Image, true);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Picture > ("caption", this,  &GUI_Picture::pget_Caption, &GUI_Picture::pset_Caption, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Picture > ("captionx", this, &GUI_Picture::pget_CaptionX, &GUI_Picture::pset_CaptionX, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Picture > ("captiony", this, &GUI_Picture::pget_CaptionY, &GUI_Picture::pset_CaptionY, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Picture > ("font", this,  &GUI_Picture::pget_Font, &GUI_Picture::pset_Font, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Picture > ("fontsize", this, &GUI_Picture::pget_FontSize, &GUI_Picture::pset_FontSize, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Picture > ("fontcolor", this, &GUI_Picture::pget_FontColor, &GUI_Picture::pset_FontColor, false);
    AddProperty(mp);

    MemberMethodHandler < GUI_Picture > *mh;
    mh = new MemberMethodHandler < GUI_Picture > ("reloadimage", this, 0, &GUI_Picture::m_ReloadImage);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_Picture > ("zoomimage", this, 3, &GUI_Picture::m_ZoomImage);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_Picture > ("fadeimage", this, 1, &GUI_Picture::m_FadeImage);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_Picture > ("rotateimage", this, 1, &GUI_Picture::m_RotateImage);
    AddMethod(mh);
}

GUI_Picture::~GUI_Picture()
{
    if (image) {
        image->DecRef();
    }
    if (caption) {
        caption->DecRef();
    }
}

void GUI_Picture::OnStart()
{
    Zoom(area.w, area.h);
}

void GUI_Picture::OnTransform()
{
    if (image) {
        GUI_Surface *img = systemObject->LoadImage(image->GetName(), false);
        if (img) {
            ZoomImage(img, 0, 0);
        }
    }
}

void GUI_Picture::Update(int force)
{
    if (!parent) {
        return;
    }
    if (force) {
        GUI_Widget::Update(force);
        if (image) {
            SDL_Rect r;
            r.w = image->GetWidth();
            r.h = image->GetHeight();
            r.x = area.x + (area.w - r.w) / 2;
            r.y = area.y + (area.h - r.h) / 2;
            parent->Draw(image, NULL, &r);
        }
    }
    if (caption) {
        caption->DoUpdate(force);
    }
}

void GUI_Picture::Erase(const SDL_Rect * rp)
{
    SDL_Rect dest = Adjust(rp);

    if (IsTransparent()) {
        parent->Erase(&dest);
    }
    if (image) {
        SDL_Rect sr, dr;
        sr.w = dr.w = image->GetWidth();
        sr.h = dr.h = image->GetHeight();
        dr.x = area.x + (area.w - dr.w) / 2;
        dr.y = area.y + (area.h - dr.h) / 2;
        sr.x = sr.y = 0;
        if (ClipRect(&sr, &dr, &dest)) {
            parent->Draw(image, &sr, &dr);
        }
    }
}

bool GUI_Picture::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    if (caption) {
        if (caption->HandleMouseEvent(event, xoffset + area.x, yoffset + area.y)) {
            return 1;
        }
    }
    return GUI_Widget::HandleMouseEvent(event, xoffset, yoffset);
}

void GUI_Picture::SetImage(GUI_Surface * an_image)
{
    Object::Keep((Object **) & image, an_image);
}

void GUI_Picture::ZoomImage(GUI_Surface * an_image, int iw, int ih)
{
    Lock();
    if (iw) {
        area.w = iw;
    }
    if (ih) {
        area.h = ih;
    }
    float zw = (float) area.w / an_image->GetWidth();
    float zh = (float) area.h / an_image->GetHeight();
    SDL_Surface *surface = image->zoomSurface(an_image->GetSurface(), zw, zh, SMOOTHING_ON);
    an_image->SetSurface(surface);
    Unlock();
    SetImage(an_image);
}

void GUI_Picture::Zoom(int iw, int ih)
{
    if (!image) {
        return;
    }
    Lock();
    if (iw) {
        area.w = iw;
    }
    if (ih) {
        area.h = ih;
    }
    float zw = (float) area.w / image->GetWidth();
    float zh = (float) area.h / image->GetHeight();
    SDL_Surface *surface = image->zoomSurface(image->GetSurface(), zw, zh, SMOOTHING_ON);
    image->SetSurface(surface);
    Unlock();
    SetChanged(1);
}

void GUI_Picture::FadeImage(GUI_Surface * an_image)
{
    float zw = (float) area.w / an_image->GetWidth();
    float zh = (float) area.h / an_image->GetHeight();
    SDL_Surface *surface = image->zoomSurface(an_image->GetSurface(), zw, zh, SMOOTHING_ON);
    an_image->SetSurface(surface);
    int step = area.w * area.h / 30000;
    if (step < 2) {
        step = 2;
    }
    surface->format->Amask = 0;
    Lock();
    for (int i = 2; i < 0x100; i += step) {
        parent->Draw(image, NULL, &area);
    	SDL_SetAlpha(surface, SDL_SRCALPHA, i);
        parent->Draw(an_image, NULL, &area);
    	systemObject->Flip();
    }
    Unlock();
    SDL_SetAlpha(surface, 0, SDL_ALPHA_OPAQUE);
    surface->format->Amask = AMask;
    SetImage(an_image);
    SetChanged(1);
}

void GUI_Picture::Reload(void)
{
    GUI_Surface *img = systemObject->LoadImage(image->GetName(), false);
    if (img) {
        ZoomImage(img, area.w, area.h);
    }
}

void GUI_Picture::Rotate(double angle, double zoom)
{
    if (!image) {
        return;
    }
    Lock();
    SDL_Surface *surface = image->rotozoomSurface(image->GetSurface(), angle, zoom, SMOOTHING_ON);
    image->Fill(NULL, 0xFF000000);
    SDL_BlitSurface(surface, NULL, image->GetSurface(), NULL);
    Unlock();
    SDL_FreeSurface(surface);
    SetChanged(1);
}

void GUI_Picture::SetCaption(GUI_Widget * a_caption)
{
    Keep((Object**)&caption, a_caption);
}

Variant GUI_Picture::pget_Image(void)
{
    if (this->image) {
        return anytovariant(this->image->GetName());
    }
    return VARNULL;
}

int GUI_Picture::pset_Image(Variant value)
{
    GUI_Surface *img = systemObject->LoadImage(value, false);
    if (img) {
        SetImage(img);
    }
    return 0;
}

Variant GUI_Picture::pget_Caption(void)
{
    return anytovariant(caption ? caption->GetText() : "");
}

int GUI_Picture::pset_Caption(Variant value)
{
    if (!caption) {
        SetCaption(new GUI_Label(name, 0, 0, area.w, area.h, 0, 0));
    }
    caption->SetText(value);
    return 0;
}

Variant GUI_Picture::pget_Font(void)
{
    if (caption) {
        return anytovariant(caption->GetFont()->GetName());
    }
    return VARNULL;
}

int GUI_Picture::pset_Font(Variant value)
{
    if (caption) {
        GUI_Font *font = systemObject->LoadFont(value, caption->GetFontSize());
        if (font) {
            caption->SetFont(font);
        }
    }
    return 0;
}

Variant GUI_Picture::pget_FontSize(void)
{
    if (caption) {
        return anytovariant(caption->GetFontSize());
    }
    return VARNULL;
}

int GUI_Picture::pset_FontSize(Variant value)
{
    if (caption) {
        caption->SetFontSize(value);
    }
    return 0;
}

Variant GUI_Picture::pget_FontColor(void)
{
    if (caption) {
        return anytovariant(colorToStr(caption->GetTextColor()));
    }
    return VARNULL;
}

int GUI_Picture::pset_FontColor(Variant value)
{
    if (caption) {
        caption->SetTextColor(strToColor(value));
    }
    return 0;
}

Variant GUI_Picture::pget_CaptionX(void)
{
    if (caption) {
        return anytovariant(caption->GetX());
    }
    return anytovariant(0);
}

int GUI_Picture::pset_CaptionX(Variant value)
{
    if (caption) {
        caption->SetPosition(value, -1, -1, -1);
    }
    return 0;
}

Variant GUI_Picture::pget_CaptionY(void)
{
    if (caption) {
        return anytovariant(caption->GetY());
    }
    return anytovariant(0);
}

int GUI_Picture::pset_CaptionY(Variant value)
{
    if (caption) {
        caption->SetPosition(-1, value, -1, -1);
    }
    return 0;
}

Variant GUI_Picture::m_ReloadImage(int numargs, Variant args[])
{
    Reload();
    return VARNULL;
}

Variant GUI_Picture::m_ZoomImage(int numargs, Variant args[])
{
    GUI_Surface *img = systemObject->LoadImage(args[0], false);
    if (img) {
        int iw = args[1];
        int ih = args[2];
        ZoomImage(img, iw, ih);
    }
    return VARNULL;
}

Variant GUI_Picture::m_FadeImage(int numargs, Variant args[])
{
    GUI_Surface *img = systemObject->LoadImage(args[0], false);
    if (img) {
        FadeImage(img);
    }
    return VARNULL;
}

Variant GUI_Picture::m_RotateImage(int numargs, Variant args[])
{
    Rotate(args[0], 1);
    return VARNULL;
}

