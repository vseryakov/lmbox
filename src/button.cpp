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

GUI_AbstractButton::GUI_AbstractButton(const char *aname, int x, int y, int w, int h):GUI_Widget(aname, x, y, w, h)
{
    type = TYPE_BUTTON;
    caption = 0;
    action = 0;
    autosize = 0;
    defaultevent = "OnClick";
    SetFlags(WIDGET_SELECTABLE);
    SetTransparent(1);

    MemberFunctionProperty < GUI_AbstractButton > *mp;
    mp = new MemberFunctionProperty < GUI_AbstractButton > ("action", this, &GUI_AbstractButton::pget_Action, &GUI_AbstractButton::pset_Action, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_AbstractButton > ("autosize", this, &GUI_AbstractButton::pget_AutoSize, &GUI_AbstractButton::pset_AutoSize, false);
    AddProperty(mp);
}

GUI_AbstractButton::~GUI_AbstractButton()
{
    if (caption) {
        caption->DecRef();
    }
    lmbox_free(action);
}

void GUI_AbstractButton::Update(int force)
{
    if (!parent) {
        return;
    }
    GUI_Widget::Update(force);
    if (caption) {
        caption->Update(force);
    }
}

void GUI_AbstractButton::Clicked(int x, int y)
{
    GUI_Widget::Clicked(x, y);
    OnClick();
}

void GUI_AbstractButton::OnResize(void)
{
    UpdateLabel();
}

int GUI_AbstractButton::OnChangeFocus(void)
{
    UpdateLabel();
    return GUI_Widget::OnChangeFocus();
}

void GUI_AbstractButton::OnClick(void)
{
    FireEvent("OnClick");
}

int GUI_AbstractButton::Initialize(void)
{
    EventHandler *handler;
    if (action && GetDefaultEvent() && (handler = FindEventHandler(GetDefaultEvent()))) {
        AddEventHandler(action, handler->GetScript());
        handler->DecRef();
    }
    return GUI_Widget::Initialize();
}

void GUI_AbstractButton::UpdateLabel(void)
{
    if (!caption) {
        return;
    }
    SDL_Rect a = GetArea();
    SDL_Rect r = caption->GetArea();

    if (autosize) {
        if (caption->GetTextSize() >= a.w - r.x*2) {
            SetFlags(OBJECT_BUSY);
            SetWidth(caption->GetTextSize() + r.x*2);
            ClearFlags(OBJECT_BUSY);
        }
    }
    if (r.x != a.x || r.y != a.y || r.w != a.w || r.h != a.h) {
        caption->SetPosition(-1, -1, a.w - r.x*2, a.h - r.y*2);
    }
}

void GUI_AbstractButton::RemoveWidget(GUI_Widget * widget)
{
    if (widget == caption) {
        Keep((Object**)&caption, NULL);
    }
}

void GUI_AbstractButton::SetCaption(GUI_Label * widget)
{
    Keep((Object**)&caption, widget);
    UpdateLabel();
}

void GUI_AbstractButton::SetCaptionX(int x)
{
    if (caption) {
        caption->SetPosition(x, -1, -1, -1);
    }
    UpdateLabel();
    SetChanged(1);
}

void GUI_AbstractButton::SetCaptionY(int y)
{
    if (caption) {
        caption->SetPosition(-1, y, -1, -1);
    }
    UpdateLabel();
    SetChanged(1);
}

void GUI_AbstractButton::SetAction(const char *action)
{
    char *tmp = this->action;
    this->action = lmbox_strdup(action);
    lmbox_free(tmp);
}

const char *GUI_AbstractButton::GetAction()
{
    return action ? action : name;
}

Variant GUI_AbstractButton::pget_Action(void)
{
    return anytovariant(GetAction());
}

int GUI_AbstractButton::pset_Action(Variant value)
{
    SetAction(value.c_str());
    return 0;
}

Variant GUI_AbstractButton::pget_AutoSize(void)
{
    return anytovariant(autosize);
}

int GUI_AbstractButton::pset_AutoSize(Variant value)
{
    autosize = value;
    UpdateLabel();
    SetChanged(1);
    return 0;
}

GUI_Button::GUI_Button(const char *aname, int x, int y, int w, int h):GUI_AbstractButton(aname, x, y, w, h)
{
    fontnormalcolor = DEFAULT_FONTCOLOR;
    fonthighlightcolor = DEFAULT_FONTCOLOR;
    fontdisabledcolor = DEFAULT_FONTCOLOR;
    fontpressedcolor = DEFAULT_FONTCOLOR;

    captiondropx = 2;
    captiondropy = 2;
    down = false;

    SetCaption(new GUI_Label(name, 0, 0, w, h, 0, 0));
    caption->DecRef();
    caption->SetTextAlign("center");
    caption->SetTransparent(0);

    MemberFunctionProperty < GUI_Button > *mp;

    mp = new MemberFunctionProperty < GUI_Button > ("caption", this,  &GUI_Button::pget_Caption, &GUI_Button::pset_Caption, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("captionx", this, &GUI_Button::pget_CaptionX, &GUI_Button::pset_CaptionX, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("captiony", this, &GUI_Button::pget_CaptionY, &GUI_Button::pset_CaptionY, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("font", this,  &GUI_Button::pget_Font, &GUI_Button::pset_Font, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("fontsize", this, &GUI_Button::pget_FontSize, &GUI_Button::pset_FontSize, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("fontcolor", this, &GUI_Button::pget_FontColor, &GUI_Button::pset_FontColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("fontfocusedcolor", this, &GUI_Button::pget_FontFocusColor, &GUI_Button::pset_FontFocusColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("fontpressedcolor", this, &GUI_Button::pget_FontPressedColor, &GUI_Button::pset_FontPressedColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("fontdisabledcolor", this, &GUI_Button::pget_FontDisabledColor, &GUI_Button::pset_FontDisabledColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("captiondropx", this, &GUI_Button::pget_CaptionDropX, &GUI_Button::pset_CaptionDropX, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("captiondropy", this, &GUI_Button::pget_CaptionDropY, &GUI_Button::pset_CaptionDropY, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("wordwrap", this, &GUI_Button::pget_WordWrap, &GUI_Button::pset_WordWrap, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Button > ("autosize", this, &GUI_Button::pget_AutoSize, &GUI_Button::pset_AutoSize, false);
    AddProperty(mp);

    MemberMethodHandler < GUI_Button > *mh = new MemberMethodHandler < GUI_Button > ("click", this, 0, &GUI_Button::m_Click);
    AddMethod(mh);
}

GUI_Button::~GUI_Button()
{
}

void GUI_Button::SetBackground(GUI_Surface * surface)
{
    GUI_Widget::SetBackground(surface);
    GUI_Widget::SetBackgroundStyle(BACKGROUND_SCALED);
    if (background.img[IDX_NORMAL]) {
        SDL_Rect a = GetArea();
        SDL_Rect r = background.img[IDX_NORMAL]->GetArea();
        if (a.w != r.w || a.h != r.h) {
            SetPosition(-1, -1, r.w, r.h);
            UpdateLabel();
        }
    }
}

void GUI_Button::SetCaptionDropX(int amount)
{
    captiondropx = amount;
    UpdateLabel();
}

void GUI_Button::SetCaptionDropY(int amount)
{
    captiondropy = amount;
    UpdateLabel();
}

int GUI_Button::GetAlign(void)
{
    return caption ? caption->GetAlign() : 0;
}

void GUI_Button::SetAlign(int flag)
{
    if (caption) {
        caption->SetAlign(flag);
        UpdateLabel();
    }
}

bool GUI_Button::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    int rc = 0;
    switch (event->type) {
     case SDL_MOUSEBUTTONUP:
         if (caption && IsPressed() && down) {
             SDL_Rect r = caption->GetArea();
             caption->SetPosition(r.x - captiondropx, r.y - captiondropy, -1, -1);
             SetChanged(1);
             down = false;
             OnUpDown();
         }
         break;

     case SDL_MOUSEMOTION:
         if (caption && IsInside() && IsPressed()) {
             int x = event->motion.x - xoffset;
             int y = event->motion.y - yoffset;
             if (Inside(x, y, &area) && !down) {
                 SDL_Rect r = caption->GetArea();
                 caption->SetPosition(r.x + captiondropx, r.y + captiondropy, -1, -1);
                 SetChanged(1);
                 down = true;
                 OnUpDown();
             } else
             if (!Inside(x, y, &area) && down) {
                 SDL_Rect r = caption->GetArea();
                 caption->SetPosition(r.x - captiondropx, r.y - captiondropy, -1, -1);
                 SetChanged(1);
                 down = false;
                 OnUpDown();
             }
         }
         break;
    }
    rc = GUI_Widget::HandleMouseEvent(event, xoffset, yoffset);
    switch (event->type) {
     case SDL_MOUSEBUTTONDOWN:
         if (caption && IsPressed() && !down) {
             SDL_Rect r = caption->GetArea();
             caption->SetPosition(r.x + captiondropx, r.y + captiondropy, -1, -1);
             SetChanged(1);
             down = true;
             OnUpDown();
         }
         break;
    }
    return rc;
}

bool GUI_Button::IsDown(void)
{
    return down;
}

void GUI_Button::SimulateClick(void)
{
    SDL_Rect r;
    if (caption) {
        // Move caption down
        r = caption->GetArea();
        caption->SetPosition(r.x + captiondropx, r.y + captiondropy, -1, -1);
        SetChanged(1);
    }
    // Make it look pressed
    SetInside(1);
    SetPressed(1);
    systemObject->DoUpdate(0);
    // Delay for a bit so you can see the effect
    SDL_Delay(175);
    // Make it look normal
    SetPressed(0);
    if (caption) {
        // Move caption up
        r = caption->GetArea();
        caption->SetPosition(r.x - captiondropx, r.y - captiondropy, -1, -1);
        SetChanged(1);
    }
    Clicked(0, 0);
}

void GUI_Button::OnUpDown(void)
{
    UpdateLabel();
}

void GUI_Button::UpdateLabel(void)
{
    if (!caption) {
        return;
    }
    SDL_Color newcolor, oldcolor = caption->GetTextColor();
    if (!IsDisabled()) {
        if (IsDown()) {
            newcolor = fontpressedcolor;
        } else
        if (IsInside() || IsFocused()) {
            newcolor = fonthighlightcolor;
        } else {
            newcolor = fontnormalcolor;
        }
    } else {
        newcolor = fontdisabledcolor;
    }
    if (newcolor.r != oldcolor.r || newcolor.b != oldcolor.b || newcolor.g != oldcolor.g) {
        caption->SetTextColor(newcolor);
    }
    GUI_AbstractButton::UpdateLabel();
}

void GUI_Button::SetText(const char *s)
{
    if (caption) {
        caption->SetText(s);
    }
    UpdateLabel();
    SetChanged(1);
}

void GUI_Button::SetFont(GUI_Font *font)
{
    if (caption) {
        caption->SetFont(font);
    }
    UpdateLabel();
    SetChanged(1);
}

void GUI_Button::SetFontSize(int size)
{
    if (caption) {
        caption->SetFontSize(size);
    }
    UpdateLabel();
    SetChanged(1);
}

void GUI_Button::SetFontNormalColor(SDL_Color c)
{
    fontnormalcolor = c;
    UpdateLabel();
    SetChanged(1);
}

void GUI_Button::SetFontHighlightColor(SDL_Color c)
{
    fonthighlightcolor = c;
    UpdateLabel();
    SetChanged(1);
}

void GUI_Button::SetFontDisabledColor(SDL_Color c)
{
    fontdisabledcolor = c;
    UpdateLabel();
    SetChanged(1);
}

void GUI_Button::SetFontPressedColor(SDL_Color c)
{
    fontpressedcolor = c;
    UpdateLabel();
    SetChanged(1);
}

bool GUI_Button::HandleEvent(const char *action)
{
    if (GUI_Widget::HandleEvent(action)) {
        return true;
    }
    if (!strcmp(action, "accept")) {
        this->SimulateClick();
        return true;
    }
    return false;
}

Variant GUI_Button::pget_Caption(void)
{
    if (caption) {
        return anytovariant(caption->GetText());
    }
    return VARNULL;
}

int GUI_Button::pset_Caption(Variant value)
{
    SetText(value);
    return 0;
}

Variant GUI_Button::pget_Font(void)
{
    if (caption) {
        return anytovariant(caption->GetFont()->GetName());
    }
    return VARNULL;
}

int GUI_Button::pset_Font(Variant value)
{
    if (caption) {
        GUI_Font *font = systemObject->LoadFont(value, caption->GetFontSize());
        if (font) {
            SetFont(font);
        }
    }
    return 0;
}

Variant GUI_Button::pget_FontSize(void)
{
    if (caption) {
        return anytovariant(caption->GetFontSize());
    }
    return VARNULL;
}

int GUI_Button::pset_FontSize(Variant value)
{
    SetFontSize(value);
    return 0;
}

Variant GUI_Button::pget_FontColor(void)
{
    return anytovariant(colorToStr(fontnormalcolor));
}

int GUI_Button::pset_FontColor(Variant value)
{
    SetFontNormalColor(strToColor(value));
    return 0;
}

Variant GUI_Button::pget_FontFocusColor(void)
{
    return anytovariant(colorToStr(fonthighlightcolor));
}

int GUI_Button::pset_FontFocusColor(Variant value)
{
    SetFontHighlightColor(strToColor(value));
    return 0;
}

Variant GUI_Button::pget_FontPressedColor(void)
{
    return anytovariant(colorToStr(fontpressedcolor));
}

int GUI_Button::pset_FontPressedColor(Variant value)
{
    SetFontPressedColor(strToColor(value));
    return 0;
}

Variant GUI_Button::pget_FontDisabledColor(void)
{
    return anytovariant(colorToStr(fontdisabledcolor));
}

int GUI_Button::pset_FontDisabledColor(Variant value)
{
    SetFontDisabledColor(strToColor(value));
    return 0;
}

Variant GUI_Button::pget_CaptionX(void)
{
    if (caption) {
        return anytovariant(caption->GetX());
    }
    return anytovariant(0);
}

int GUI_Button::pset_CaptionX(Variant value)
{
    SetCaptionX(value);
    return 0;
}

Variant GUI_Button::pget_CaptionY(void)
{
    if (caption) {
        return anytovariant(caption->GetY());
    }
    return anytovariant(0);
}

int GUI_Button::pset_CaptionY(Variant value)
{
    SetCaptionY(value);
    return 0;
}

Variant GUI_Button::pget_CaptionDropX(void)
{
    return anytovariant(captiondropx);
}

int GUI_Button::pset_CaptionDropX(Variant value)
{
    SetCaptionDropX(value);
    return 0;
}

Variant GUI_Button::pget_CaptionDropY(void)
{
    return anytovariant(captiondropy);
}

int GUI_Button::pset_CaptionDropY(Variant value)
{
    SetCaptionDropY(value);
    return 0;
}

Variant GUI_Button::pget_WordWrap(void)
{
    return anytovariant(caption ? caption->GetWordWrap() : 0);
}

int GUI_Button::pset_WordWrap(Variant value)
{
    if (caption) {
        caption->SetWordWrap(value);
    }
    return 0;
}

Variant GUI_Button::m_Click(int numargs, Variant args[])
{
    if (!IsDisabled()) {
        SimulateClick();
    }
    return VARNULL;
}
