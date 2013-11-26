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

#define IDX_OFF  0
#define IDX_ON   1
#define IDX_FOFF 2
#define IDX_FON  3

GUI_ToggleButton::GUI_ToggleButton(const char *aname, int x, int y, int w, int h):GUI_AbstractButton(aname, x, y, w, h)
{
    SDL_Rect in;

    in.x = 4;
    in.y = 4;
    in.w = area.w - 8;
    in.h = area.h - 8;
    checked = 0;

    type = TYPE_TOGGLEBUTTON;
    defaultevent = "OnClick";
    SetFlags(WIDGET_SELECTABLE);

    background.img[IDX_ON] = new GUI_Surface("on0", SDL_SWSURFACE, w, h, 32, RMask, GMask, BMask, AMask);
    background.img[IDX_FON] = new GUI_Surface("on1", SDL_SWSURFACE, w, h, 32, RMask, GMask, BMask, AMask);
    background.img[IDX_OFF] = new GUI_Surface("off0", SDL_SWSURFACE, w, h, 32, RMask, GMask, BMask, AMask);
    background.img[IDX_FOFF] = new GUI_Surface("off1", SDL_SWSURFACE, w, h, 32, RMask, GMask, BMask, AMask);

    background.img[IDX_OFF]->Fill(NULL, 0xFF000000);
    background.img[IDX_OFF]->Fill(&in, 0x007F0000);

    background.img[IDX_FOFF]->Fill(NULL, 0x00FFFFFF);
    background.img[IDX_FOFF]->Fill(&in, 0x007F0000);

    background.img[IDX_ON]->Fill(NULL, 0xFF000000);
    background.img[IDX_ON]->Fill(&in, 0x00007F00);

    background.img[IDX_FON]->Fill(NULL, 0x00FFFFFF);
    background.img[IDX_FON]->Fill(&in, 0x00007F00);

    MemberFunctionProperty < GUI_ToggleButton > *mp;
    mp = new MemberFunctionProperty < GUI_ToggleButton > ("normalimage", this, &GUI_ToggleButton::pget_NormalImage, &GUI_ToggleButton::pset_NormalImage, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ToggleButton > ("normalfocusimage", this, &GUI_ToggleButton::pget_NormalFocusImage, &GUI_ToggleButton::pset_NormalFocusImage, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ToggleButton > ("checkedimage", this, &GUI_ToggleButton::pget_CheckedImage, &GUI_ToggleButton::pset_CheckedImage, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ToggleButton > ("checkedfocusimage", this, &GUI_ToggleButton::pget_CheckedFocusImage, &GUI_ToggleButton::pset_CheckedFocusImage, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ToggleButton > ("checked", this, &GUI_ToggleButton::pget_Checked, &GUI_ToggleButton::pset_Checked, false);
    AddProperty(mp);
}

GUI_ToggleButton::~GUI_ToggleButton()
{
}

void GUI_ToggleButton::Clicked(int x, int y)
{
    checked = 1 - checked;
    SetChanged(1);
    GUI_AbstractButton::Clicked(x, y);
}

int GUI_ToggleButton::GetBackgroundIdx(void)
{
    if (IsInside() || IsFocused()) {
        if (checked) {
            return IDX_FON;
        } else {
            return IDX_ON;
        }
    }
    if (checked) {
        return IDX_FOFF;
    }
    return IDX_OFF;
}

void GUI_ToggleButton::SetBackground(GUI_Surface * surface)
{
    GUI_Widget::SetBackground(surface);
    if (background.img[IDX_OFF]) {
        SDL_Rect r = background.img[IDX_OFF]->GetArea();
        if (area.w != r.w || area.h != r.h) {
            SetPosition(-1, -1, r.w, r.h);
        }
    }
}

bool GUI_ToggleButton::HandleEvent(const char *action)
{
    if (GUI_Widget::HandleEvent(action)) {
        return true;
    }
    if (!strcmp(action, "accept")) {
        Clicked(0, 0);
        FireEvent("OnClick");
        return true;
    }
    return false;
}

Variant GUI_ToggleButton::pget_NormalImage(void)
{
     return anytovariant(background.img[IDX_OFF]->GetName());
}

int GUI_ToggleButton::pset_NormalImage(Variant value)
{
    GUI_Surface *img = systemObject->LoadImage(value, true);
    if (img) {
        SetBackground(img);
    }
    return 0;
}

Variant GUI_ToggleButton::pget_NormalFocusImage(void)
{
    return anytovariant(background.img[IDX_ON]->GetName());
    return VARNULL;
}

int GUI_ToggleButton::pset_NormalFocusImage(Variant value)
{
    GUI_Surface *img = systemObject->LoadImage(value, true);
    if (img) {
        SetFocusedBackground(img);
    }
    return 0;
}

Variant GUI_ToggleButton::pget_CheckedImage(void)
{
    return anytovariant(background.img[IDX_FOFF]->GetName());
    return VARNULL;
}

int GUI_ToggleButton::pset_CheckedImage(Variant value)
{
    GUI_Surface *img = systemObject->LoadImage(value, true);
    if (img) {
        SetPressedBackground(img);
    }
    return 0;
}

Variant GUI_ToggleButton::pget_CheckedFocusImage(void)
{
    return anytovariant(background.img[IDX_FON]->GetName());
    return VARNULL;
}

int GUI_ToggleButton::pset_CheckedFocusImage(Variant value)
{
    GUI_Surface *img = systemObject->LoadImage(value, true);
    if (img) {
        SetDisabledBackground(img);
    }
    return 0;
}

Variant GUI_ToggleButton::pget_Checked(void)
{
    return anytovariant(checked);
}

int GUI_ToggleButton::pset_Checked(Variant value)
{
    checked = (int)value ? 1 : 0;
    SetChanged(1);
    OnClick();
    return 0;
}

