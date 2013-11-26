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

GUI_Label::GUI_Label(const char *aname, int x, int y, int w, int h, GUI_Font * afont, const char *s):GUI_Widget(aname, x, y, w, h)
{
    type = TYPE_LABEL;
    SetTransparent(1);

    text = 0;
    font = afont;
    textcolor = DEFAULT_FONTCOLOR;
    fontsize = DEFAULT_FONTSIZE;
    textstring = 0;
    textbuffer = 0;
    wordwrap = false;
    autosize = false;
    lineoffset = 0;
    linespacing = 2;

    if (textstring) {
        SetText(textstring);
    }

    MemberFunctionProperty < GUI_Label > *mp;
    mp = new MemberFunctionProperty < GUI_Label > ("caption", this, &GUI_Label::pget_Caption, &GUI_Label::pset_Caption, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("font", this, &GUI_Label::pget_Font, &GUI_Label::pset_Font, true);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("fontsize", this, &GUI_Label::pget_FontSize, &GUI_Label::pset_FontSize, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("textsize", this, &GUI_Label::pget_TextSize, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("fontcolor", this, &GUI_Label::pget_FontColor, &GUI_Label::pset_FontColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("wordwrap", this, &GUI_Label::pget_WordWrap, &GUI_Label::pset_WordWrap, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("autosize", this, &GUI_Label::pget_AutoSize, &GUI_Label::pset_AutoSize, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("lineoffset", this, &GUI_Label::pget_LineOffset, &GUI_Label::pset_LineOffset, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("linespacing", this, &GUI_Label::pget_LineSpacing, &GUI_Label::pset_LineSpacing, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Label > ("linecount", this, &GUI_Label::pget_LineCount, NULL, false);
    AddProperty(mp);

    defaultproperty = FindProperty("caption");
}

GUI_Label::~GUI_Label()
{
    if (text) {
        text->DecRef();
    }
    if (font) {
        font->DecRef();
    }
    lmbox_free(textstring);
    lmbox_free(textbuffer);
}

void GUI_Label::Update(int force)
{
    if (!parent) {
        return;
    }

    GUI_Widget::Update(force);

    if (!force) {
        return;
    }

    if (!text) {
        if (font && textstring) {
            SDL_Color fg = textcolor;
            text = font->RenderQuality(textstring, fg);
        }
    }
    if (!text) {
        return;
    }

    SDL_Color fg = textcolor;
    SDL_Rect dr = { 0, 0, 0, 0 };
    SDL_Rect sr = { 0, 0, 0, 0 };
    SDL_Rect clip = parent->GetAreaRaw();
    SDL_Rect rect = { area.x + contentx, area.y + contenty, area.w - contentx*2, area.h - contenty*2 };

    clip.x = clip.y = 0;
    char *bufend, *bufptr = 0;
    GUI_Surface *line;
    int maxwidth = 0, numlines = 0;

    strcpy(textbuffer, textstring);
    if (wordwrap && strchr(textstring, ' ') != NULL) {
        char *nextline = textbuffer;
        char *prevline = textbuffer;
        while (true) {
            nextline = strchr(nextline + 1, '\n');
            if (nextline) {
                nextline[0] = '\0';
            }
            bufptr = prevline;
            char *lineptr = bufptr;
            char *prevword = bufptr;
            while (true) {
                bufptr = strchr(bufptr + 1, ' ');
                // we're only interested in the line to this point
                if (bufptr) {
                    bufptr[0] = '\0';
                }
                // Render the line, so we can find out how wide it is
                if (lineptr && strlen(lineptr) > 0) {
                    int lw = font->GetTextWidth(lineptr);
                    if (bufptr) {
                        bufptr[0] = ' ';
                    }
                    if (lw > rect.w - 1) {
                        if (prevword == textbuffer && bufptr != NULL) {
                            // The first word is really long, so ignore it
                            bufptr[0] = '\n';
                            lineptr = bufptr;
                            maxwidth = MAX(maxwidth, lw);
                        } else {
                            // Wrap at the start of the last word
                            prevword[0] = '\n';
                            lineptr = prevword;
                        }
                    } else {
                         maxwidth = MAX(maxwidth, lw);
                    }
                }
                prevword = bufptr;
                if (!bufptr) {
                    break;
                }
            }
            if (!nextline) {
                break;
            }
            nextline[0] = '\n';
            prevline = nextline;
            numlines++;
        }
        dr.w = maxwidth;
        dr.h = text->GetHeight() * (countString(textbuffer, '\n') + 1);
    } else {
        numlines = countString(textbuffer, '\n') + 1;
        if (numlines > 1) {
            // Manual-newline
            dr.h = text->GetHeight() * numlines;
            bufptr = textbuffer;
            while (true) {
                bufend = strsep(&bufptr, "\n");
                if (!bufend) {
                    break;
                }
                if (strlen(bufend) > 0) {
                    maxwidth = MAX(maxwidth, font->GetTextWidth(bufend));
                }
            }
            dr.w = maxwidth;
            strcpy(textbuffer, textstring);
        } else {
            // Single line
            dr.h = text->GetHeight();
            dr.w = text->GetWidth();
        }
    }

    if (autosize) {
        if (!wordwrap) {
             area.w = dr.w;
             rect.w = area.w - contentx*2;
        }
        area.h = dr.h;
        rect.h = area.h - contenty*2;
    } else {
        if (numlines <= 1) {
            // Single line must fit in
            if (dr.w > rect.w && rect.w > 0) {
                dr.w = rect.w;
            }
            if (dr.h > rect.h && rect.h > 0) {
                dr.h = rect.h;
            }
        }
    }
    sr.w = rect.w;
    sr.h = rect.h;

    // Set first position
    switch (align & ALIGN_HORIZ_MASK) {
     case ALIGN_HORIZ_CENTER:
          dr.x = rect.x + (rect.w - dr.w) / 2;
          break;
     case ALIGN_HORIZ_RIGHT:
          dr.x = rect.x + rect.w - dr.w;
          break;
     case ALIGN_HORIZ_LEFT:
     default:
          dr.x = rect.x;
          break;
    }
    switch (align & ALIGN_VERT_MASK) {
     case ALIGN_VERT_BOTTOM:
          dr.y = rect.y + rect.h - dr.h;
          break;
     case ALIGN_VERT_CENTER:
          dr.y = rect.y + (rect.h - dr.h) / 2;
          break;
     case ALIGN_VERT_TOP:
     default:
          dr.y = rect.y;
          break;
    }
    // Draw the text
    if (ClipRect(&sr, &dr, &clip)) {
        // Multi-line
        if (strchr(textbuffer, '\n')) {
            int nline = 0;
            bufptr = textbuffer;
            while (true) {
                bufend = strsep(&bufptr, "\n");
                if (!bufend || dr.y > (rect.y + rect.h)) {
                    break;
                }
                nline++;
                if (lineoffset > 0 && nline < lineoffset) {
                    continue;
                }
                if (strlen(bufend) > 0) {
                    GUI_Surface **lines = font->RenderColorList(bufend, &fg);
                    for (int x = 0, i = 0; lines && lines[i]; i++) {
                        line = lines[i];
                        // Figure out positioning
                        dr.w = line->GetWidth();
                        dr.h = line->GetHeight();
                        switch (align & ALIGN_HORIZ_MASK) {
                         case ALIGN_HORIZ_CENTER:
                              dr.x = rect.x + (rect.w - dr.w) / 2;
                              break;
                         case ALIGN_HORIZ_RIGHT:
                              dr.x = rect.x + (rect.w - dr.w);
                              break;
                         case ALIGN_HORIZ_LEFT:
                         default:
                              dr.x = rect.x;
                              break;
                        }
                        // Make adjustments for lines that are too wide
                        if (dr.x - rect.x < 0) {
                            sr.x = rect.x - dr.x;
                            dr.x += (rect.x - dr.x);
                        } else {
                            sr.x = 0;
                        }
                        // Make adjustments for lines that are too tall
                        if (dr.y - rect.y < 0) {
                            sr.y = rect.y - dr.y;
                            dr.y += (rect.y - dr.y);
                        } else {
                            sr.y = 0;
                        }
                        sr.h = rect.h - (dr.y - rect.y);
                        sr.w = rect.w - (dr.x - rect.x);
                        // Draw the font to the screen
                        dr.x += x;
                        //parent->Draw(line, &sr, &dr);
                        DrawClipped(line, parent, rect, dr.x, dr.y);
                        // Done with it, so release it
                        x += line->GetWidth();
                        line->DecRef();
                    }
                    lmbox_free(lines);
                } else {
                    // Space between lines
                    dr.h = linespacing;
                }
                // Increment the vertical
                dr.y += dr.h;
            }
        } else {
            // Single line
            DrawClipped(text, parent, rect, dr.x, dr.y);
        }
    }
}

void GUI_Label::SetFontSize(int size)
{
    fontsize = size;
    if (font) {
        GUI_Font *fn = systemObject->LoadFont(font->GetName(), fontsize);
        SetFont(fn);
    }
}

GUI_Font *GUI_Label::GetFont(void)
{
    return font;
}

int GUI_Label::GetFontSize(void)
{
    return fontsize;
}

int GUI_Label::GetLineOffset(void)
{
    return lineoffset;
}

int GUI_Label::GetLineSpacing(void)
{
    return linespacing;
}

int GUI_Label::GetLineCount(void)
{
    return countString(textstring ? textstring : "", '\n') + 1;
}

int GUI_Label::GetAutoSize(void)
{
    return autosize;
}

int GUI_Label::GetWordWrap(void)
{
    return wordwrap;
}

SDL_Color GUI_Label::GetTextColor(void)
{
    return textcolor;
}

void GUI_Label::SetFont(GUI_Font * afont)
{
    Lock();
    if (Object::Keep((Object **) & font, afont)) {
        if (text) {
            text->DecRef();
            text = 0;
        }
    }
    Unlock();
    SetChanged(1);
}

void GUI_Label::SetTextColor(SDL_Color c)
{
    Lock();
    textcolor = c;
    if (text) {
        text->DecRef();
        text = 0;
    }
    Unlock();
    SetChanged(1);
}

void GUI_Label::SetText(const char *s)
{
    Lock();
    lineoffset = 0;
    lmbox_free(textstring);
    lmbox_free(textbuffer);
    textstring = lmbox_strdup(s);
    textbuffer = lmbox_strdup(s);
    if (text) {
        text->DecRef();
        text = 0;
    }
    Unlock();
    SetChanged(1);
}

void GUI_Label::SetAutoSize(bool autosize)
{
    this->autosize = autosize;
    SetChanged(1);
}

void GUI_Label::SetLineOffset(int line)
{
    if ((this->lineoffset = line) < 0) {
        this->lineoffset = 0;
    }
    SetChanged(1);
}

void GUI_Label::SetLineSpacing(int line)
{
    this->lineoffset = line;
    SetChanged(1);
}

void GUI_Label::SetWordWrap(bool wordwrap)
{
    this->wordwrap = wordwrap;
    SetChanged(1);
}

void GUI_Label::FireTimer(void)
{
    Script *script = 0;
    Lock();
    if (timer_script) {
        script = new Script(GetName(), timer_script);
    }
    Unlock();
    if (script) {
        script->Execute();
        SetText(script->GetResult());
        delete script;
    }
}

int GUI_Label::GetTextSize(void)
{
    SDL_Rect r = { 0, 0, 0, 0};
    if (text) {
        r = text->GetArea();
    }
    return r.w;
}

const char *GUI_Label::GetText(void)
{
    return textstring ? textstring : "";
}

Variant GUI_Label::pget_Caption(void)
{
    Lock();
    Variant rc = anytovariant(GetText());
    Unlock();
    return rc;
}

int GUI_Label::pset_Caption(Variant value)
{
    SetText(value);
    return 0;
}

Variant GUI_Label::pget_Font(void)
{
    return anytovariant(font ? font->GetName() : "");
}

int GUI_Label::pset_Font(Variant value)
{
    GUI_Font *fn = systemObject->LoadFont(value, fontsize);
    if (fn) {
        SetFont(fn);
    }
    return 0;
}

Variant GUI_Label::pget_LineOffset(void)
{
    return anytovariant(GetLineOffset());
}

int GUI_Label::pset_LineOffset(Variant value)
{
    SetLineOffset(value);
    return 0;
}

Variant GUI_Label::pget_LineCount(void)
{
    return anytovariant(GetLineCount());
}

Variant GUI_Label::pget_LineSpacing(void)
{
    return anytovariant(GetLineSpacing());
}

int GUI_Label::pset_LineSpacing(Variant value)
{
    SetLineSpacing(value);
    return 0;
}

Variant GUI_Label::pget_FontSize(void)
{
    return anytovariant(fontsize);
}

int GUI_Label::pset_FontSize(Variant value)
{
    SetFontSize(value);
    return 0;
}

Variant GUI_Label::pget_FontColor(void)
{
    return anytovariant(colorToStr(GetTextColor()));
}

int GUI_Label::pset_FontColor(Variant value)
{
    SetTextColor(strToColor(value));
    return 0;
}

Variant GUI_Label::pget_TextSize(void)
{
    return anytovariant(GetTextSize());
}

Variant GUI_Label::pget_AutoSize(void)
{
    return anytovariant(autosize);
}

int GUI_Label::pset_AutoSize(Variant value)
{
    SetAutoSize(value);
    return 0;
}

Variant GUI_Label::pget_WordWrap(void)
{
    return anytovariant(wordwrap);
}

int GUI_Label::pset_WordWrap(Variant value)
{
    SetWordWrap(value);
    return 0;
}
