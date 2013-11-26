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

GUI_TextField::GUI_TextField(const char *aname, int x, int y, int w, int h, GUI_Font * afont, int maxlength):GUI_Widget(aname, x, y, w, h), font(afont)
{
    type = TYPE_TEXTFIELD;
    SetFlags(WIDGET_SELECTABLE);
    SetTransparent(1);

    fontnormalcolor = DEFAULT_FONTCOLOR;
    fontfocusedcolor = DEFAULT_FONTCOLOR;

    fontsize = DEFAULT_FONTSIZE;
    if (font) {
        font->IncRef();
    }

    cursorpos = 0;
    startoffset = 0;
    inpdigit = 0;
    inpdigitptr = 0;
    buffer_size = maxlength;
    buffer_index = 0;
    buffer = (char*)calloc(1, buffer_size);
    cbuffer = (char*)calloc(1, buffer_size);
    validchars = 0;

    MemberFunctionProperty < GUI_TextField > *mp;
    mp = new MemberFunctionProperty < GUI_TextField > ("text", this, &GUI_TextField::pget_Text, &GUI_TextField::pset_Text, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_TextField > ("font", this, &GUI_TextField::pget_Font, &GUI_TextField::pset_Font, true);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_TextField > ("fontsize", this, &GUI_TextField::pget_FontSize, &GUI_TextField::pset_FontSize, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_TextField > ("fontcolor", this, &GUI_TextField::pget_FontColor, &GUI_TextField::pset_FontColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_TextField > ("fontfocusedcolor", this, &GUI_TextField::pget_FontFocusedColor, &GUI_TextField::pset_FontFocusedColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_TextField > ("maxlength", this, &GUI_TextField::pget_MaxLength, &GUI_TextField::pset_MaxLength, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_TextField > ("cursorpos", this, &GUI_TextField::pget_CursorPos, &GUI_TextField::pset_CursorPos, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_TextField > ("validchars", this, &GUI_TextField::pget_ValidChars, &GUI_TextField::pset_ValidChars, false);
    AddProperty(mp);

    FindProperty("width")->SetRequired(true);
    FindProperty("height")->SetRequired(true);
}

GUI_TextField::~GUI_TextField()
{
    if (font) {
        font->DecRef();
    }
    lmbox_free(validchars);
    free(cbuffer);
    free(buffer);
}

void GUI_TextField::Update(int force)
{
    if (!parent || !font) {
        return;
    }
    GUI_Widget::Update(force);
    if (force) {
        GUI_Surface *surface;
        SDL_Rect r = area;
        SDL_Rect clip = area;
        clip.x = clip.x + border.x;
        clip.y = clip.y + border.y;
        clip.w = clip.w - (border.x * 2);
        clip.h = clip.h - (border.y * 2);
        SDL_Color color = IsFocused() || IsInside() ? fontfocusedcolor : fontnormalcolor;
        // Draw text
        surface = font->RenderQuality(buffer, color);
        if (surface != NULL) {
            switch (align & ALIGN_HORIZ_MASK) {
             case ALIGN_HORIZ_CENTER:
                  if (surface->GetWidth() < r.w) {
                      r.x = r.x + (r.w - surface->GetWidth()) / 2;
                  }
                  break;
             case ALIGN_HORIZ_RIGHT:
                  if (surface->GetWidth() < r.w) {
                      r.x = r.x + r.w - surface->GetWidth();
                  }
                  break;
            }
            DrawClipped(surface, parent, clip, r.x + border.x - startoffset, r.y + ((r.h - surface->GetHeight()) / 2));
            surface->DecRef();
        }
        // Draw cursor
        if (IsFocused()) {
            strcpy(cbuffer, buffer);
            cbuffer[cursorpos] = 0;
            SDL_Rect textsize = font->GetTextSize(cbuffer);
            int cursorx = r.x + border.x + textsize.w - startoffset;
            surface = new GUI_Surface("cursor", SDL_SWSURFACE, 2, clip.h, 32, RMask, GMask, BMask, AMask);
            surface->Fill(NULL, colorToInt(color));
            DrawClipped(surface, parent, clip, cursorx, r.y + ((r.h - surface->GetHeight()) / 2));
            surface->DecRef();
        }
    }
}

int GUI_TextField::OnGotFocus(void)
{
    if (!GUI_Widget::OnGotFocus()) {
        return 0;
    }
    SetCursorPos(GetTextLength());
    return 1;
}

bool GUI_TextField::HandleKeyEvent(const SDL_Event * event)
{
    if (IsFocused()) {
        int key = event->key.keysym.sym;
        int ch = event->key.keysym.unicode;
        switch (key) {
        case SDLK_RIGHT:
            SetCursorPos(cursorpos + 1);
            return 1;
        case SDLK_LEFT:
            SetCursorPos(cursorpos - 1);
            return 1;
        case SDLK_DELETE:
            DeleteCurrChar();
            return 1;
        case SDLK_BACKSPACE:
            Backspace();
            return 1;
        case SDLK_HOME:
            SetCursorPos(0);
            return 1;
        case SDLK_END:
            SetCursorPos(strlen(buffer));
            return 1;
        case SDLK_RETURN:
            break;
        }
        if (ch >= 32 && ch <= 126) {
            SendChar(ch, true);
            return 1;
        }
    }
    return GUI_Widget::HandleKeyEvent(event);
}

bool GUI_TextField::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    return GUI_Widget::HandleMouseEvent(event, xoffset, yoffset);
}

bool GUI_TextField::SendChar(char c, bool insert)
{
    if (buffer_index < buffer_size) {
        if (validchars) {
            if (!strchr(validchars, c)) {
                return false;
            }
        }
        if (cursorpos == buffer_index) {
            if (insert == false && buffer_index > 0) {
                buffer[buffer_index - 1] = c;
            } else {
                buffer[buffer_index++] = c;
            }
        } else {
            // Insert character
            char *bkup = strdup(buffer);
            if (insert == true) {
                strncpy(buffer + cursorpos + 1, bkup + cursorpos, buffer_index - cursorpos);
            }
            buffer[cursorpos] = c;
            buffer_index++;
            free(bkup);
        }
        buffer[buffer_index] = 0;
        SetCursorPos(cursorpos + 1);
        OnChange();
        SetChanged(1);
    }
    return true;
}

void GUI_TextField::Backspace(void)
{
    if (buffer_index > 0 && cursorpos > 0) {
        if (cursorpos < buffer_index) {
            char *bkup = strdup(buffer);
            strncpy(buffer + cursorpos - 1, bkup + cursorpos, buffer_index - cursorpos);
            free(bkup);
        }
        buffer[--buffer_index] = '\0';
        SetCursorPos(cursorpos - 1);
        OnChange();
        SetChanged(1);
    }
}

void GUI_TextField::DeleteCurrChar(void)
{
    if (cursorpos < buffer_index) {
        char *bkup = strdup(buffer);
        strncpy(buffer + cursorpos, bkup + cursorpos + 1, buffer_index - cursorpos);
        buffer[--buffer_index] = '\0';
        free(bkup);
        OnChange();
        SetChanged(1);
    }
}

void GUI_TextField::SetFont(GUI_Font * afont)
{
    Object::Keep((Object **) & font, afont);
    SetChanged(1);
}

void GUI_TextField::SetFontNormalColor(SDL_Color c)
{
    fontnormalcolor = c;
    SetChanged(1);
}

void GUI_TextField::SetFontFocusedColor(SDL_Color c)
{
    fontfocusedcolor = c;
    SetChanged(1);
}

void GUI_TextField::SetText(const char *text)
{
    if (text && strlen(text) < buffer_size) {
        strcpy(buffer, text);
        cursorpos = buffer_index = strlen(text);
    }
    SetChanged(1);
}

const char *GUI_TextField::GetText(void)
{
    return buffer;
}

int GUI_TextField::GetTextLength(void)
{
    return strlen(buffer);
}

int GUI_TextField::GetCursorPos(void)
{
    return cursorpos;
}

void GUI_TextField::SetCursorPos(int pos)
{
    if (pos >= 0 && pos != cursorpos) {
        if (pos <= strlen(buffer)) {
            this->cursorpos = pos;
        } else {
            this->cursorpos = strlen(buffer);
        }
        // Make sure cursor stays visible
        if (cursorpos == 0) {
            startoffset = 0;
        } else {
            // Get size of text up to cursor position
            SDL_Rect textsize = font->GetTextSize(buffer);
            if (textsize.w >= area.w - (border.x * 2) - 1 || textsize.w <= startoffset) {
                // Cursor will be outside visible area, change offset so
                // that doesn't happen
                startoffset = textsize.w - (area.w - (border.x * 2)) + 3;
            } else {
                startoffset = 0;
            }
        }
        SetChanged(1);
    }
}

void GUI_TextField::SetValidChars(const char *validchars)
{
    lmbox_free(this->validchars);
    this->validchars = lmbox_strdup(validchars);
}

const char *GUI_TextField::GetValidChars(void)
{
    return validchars ? validchars : "";
}

void GUI_TextField::OnChange(void)
{
    FireEvent("OnChange");
}

void GUI_TextField::InputDigit(int chr)
{
    timeval currtime;
    gettimeofday(&currtime, NULL);

    if (chr == -1) {
        input_time.tv_sec = 0;
        input_time.tv_usec = 0;
    } else
    if (inpdigit == chr && compareTimes(&currtime, &input_time)) {
        // We are already in digit input mode and the same button was pressed within the time limit
        // In this case, we rotate through the available digits for this character
        bool was_valid = false;
        const char *lastptr = inpdigitptr;
        bool insert = false;
        while (!was_valid) {
            inpdigitptr++;
            if (inpdigitptr[0] == '\0') {
                inpdigitptr = digitchars[inpdigit];     // back to start
            }
            if (inpdigitptr == lastptr) {
                insert = true;
            }
            was_valid = SendChar(inpdigitptr[0], insert);
        }
        // Reset timer
        gettimeofday(&input_time, NULL);
        input_time.tv_usec += TEXTINPUT_DELAY_TIME;
    } else {
        // New key
        inpdigit = chr;
        if (inpdigit != -1) {
            inpdigitptr = digitchars[inpdigit]; // back to start
            bool was_valid = false;
            while (true) {
                was_valid = SendChar(inpdigitptr[0], true);
                if (was_valid) {
                    break;
                }
                inpdigitptr++;
                if (inpdigitptr[0] == '\0') {
                    break;
                }
            }
            if (inpdigitptr[0] != '\0') {
                // Start timer
                gettimeofday(&input_time, NULL);
                input_time.tv_usec += TEXTINPUT_DELAY_TIME;
            }
        }
    }
}

bool GUI_TextField::HandleEvent(const char *action)
{
    if (!strcmp(action, "accept")) {
        GUI_Page *page = GetPage();
        if (page) {
            page->UpdateFocusWidget(1);
            page->DecRef();
        }
        return true;
    } else
    if (!strcmp(action, "prev")) {
        SetCursorPos(GetCursorPos() - 1);
        InputDigit(-1);
        return true;
    } else
    if (!strcmp(action, "next")) {
        SetCursorPos(GetCursorPos() + 1);
        InputDigit(-1);
        return true;
    } else
    if ((GetFlags() & WIDGET_FOCUSED &&
         (!strcmp(action, "stop") ||
          !strcmp(action, "cancel"))) ||
        !strcmp(action, "clear") ||
        !strcmp(action, "backspace")) {
        Backspace();
        InputDigit(-1);
        return true;
    } else
    if (!strcmp(action, "delete")) {
        DeleteCurrChar();
        InputDigit(-1);
        return true;
    } else
    if (!strcmp(action, "home") ||
        !strcmp(action, "first")) {
        SetCursorPos(0);
        InputDigit(-1);
        return true;
    } else
    if (!strcmp(action, "end") ||
        !strcmp(action, "last")) {
        SetCursorPos(GetTextLength());
        InputDigit(-1);
        return true;
    } else
    if (!strncmp(action, "key", 3) &&
        strlen(action) == 4) {
        SendChar(action[3], true);
        InputDigit(-1);
        return true;
    } else
    if (!strncmp(action, "number", 6) &&
        strlen(action) == 7 &&
        isdigit(action[6])) {
        InputDigit(atoi(action + 6));
        return true;
    }
    return GUI_Widget::HandleEvent(action);
}

Variant GUI_TextField::pget_Text(void)
{
    return anytovariant(GetText());
}

int GUI_TextField::pset_Text(Variant value)
{
    SetText(value);
    return 0;
}

Variant GUI_TextField::pget_Font(void)
{
    return anytovariant(font->GetName());
}

int GUI_TextField::pset_Font(Variant value)
{
    GUI_Font *font = systemObject->LoadFont(value, fontsize);
    if (font) {
        SetFont(font);
        return 0;
    }
    return -1;
}

Variant GUI_TextField::pget_FontSize(void)
{
    return anytovariant(fontsize);
}

int GUI_TextField::pset_FontSize(Variant value)
{
    fontsize = value;
    if (font) {
        return pset_Font(anytovariant(font->GetName()));
    }
    return 0;
}

Variant GUI_TextField::pget_FontColor(void)
{
    return anytovariant(colorToStr(fontnormalcolor));
}

int GUI_TextField::pset_FontColor(Variant value)
{
    SetFontNormalColor(strToColor(value));
    return 0;
}

Variant GUI_TextField::pget_FontFocusedColor(void)
{
    return anytovariant(colorToStr(fontfocusedcolor));
}

int GUI_TextField::pset_FontFocusedColor(Variant value)
{
    SetFontFocusedColor(strToColor(value));
    return 0;
}

Variant GUI_TextField::pget_MaxLength(void)
{
    return anytovariant((int)buffer_size);
}

int GUI_TextField::pset_MaxLength(Variant value)
{
    buffer_size = value;
    buffer = (char*)realloc(buffer, buffer_size + 1);
    cbuffer = (char*)realloc(cbuffer, buffer_size + 1);
    cursorpos = buffer_index = 0;
    buffer[0] = 0;
    return 0;
}

Variant GUI_TextField::pget_CursorPos(void)
{
    return anytovariant(GetCursorPos());
}

int GUI_TextField::pset_CursorPos(Variant value)
{
    SetCursorPos(value);
    return 0;
}

Variant GUI_TextField::pget_ValidChars(void)
{
    return anytovariant(GetValidChars());
}

int GUI_TextField::pset_ValidChars(Variant value)
{
    SetValidChars(value);
    return 0;
}

