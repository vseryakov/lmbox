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

GUI_ClockLabel::GUI_ClockLabel(const char *aname, int x, int y, int w, int h, GUI_Font * afont, const char *s):GUI_Label(aname, x, y, w, h, afont, s)
{
    type = TYPE_CLOCKLABEL;
    clockformat = strdup("%a %b %d, %Y %H:%M");

    MemberFunctionProperty < GUI_ClockLabel > *mp;
    mp = new MemberFunctionProperty < GUI_ClockLabel > ("clockformat", this, &GUI_ClockLabel::pget_ClockFormat, &GUI_ClockLabel::pset_ClockFormat, false);
    AddProperty(mp);
}

GUI_ClockLabel::~GUI_ClockLabel()
{
    lmbox_free(clockformat);
}

void GUI_ClockLabel::SetClockFormat(const char *format)
{
    if (format && *format) {
        lmbox_free(clockformat);
        clockformat = lmbox_strdup(format);
    }
    SetChanged(1);
}

void GUI_ClockLabel::OnStart()
{
    SetTimer(30000,0);
    FireTimer();
}

void GUI_ClockLabel::FireTimer(void)
{
    struct tm ltm;
    char buffer[256];
    time_t now = time(0);
    localtime_r(&now, &ltm);
    Lock();
    strftime(buffer,sizeof(buffer),clockformat,&ltm);
    Unlock();
    SetText(buffer);
}

Variant GUI_ClockLabel::pget_ClockFormat(void)
{
    return anytovariant(clockformat);
}

int GUI_ClockLabel::pset_ClockFormat(Variant value)
{
    SetClockFormat(value);
    return 0;
}
