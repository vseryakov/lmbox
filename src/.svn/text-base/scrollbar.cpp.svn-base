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

GUI_ScrollBar::GUI_ScrollBar(const char *aname, int x, int y, int w, int h):GUI_Widget(aname, x, y, w, h)
{
    type = TYPE_SCROLLBAR;
    SetTransparent(1);
    SetFlags(WIDGET_SELECTABLE);

    moved_callback = 0;
    position = 0;
    tracking_on = 0;
    tracking_pos = 0;
    tracking_start = 0;
    tracking_update = true;
    maximum = 100;
    minimum = 0;
    value = 0;
    oldvalue = 0;
    page_step = 10;
    knob = 0;
    knob_focus = 0;
    reversed = false;
    w = w < 16 ? 16 : w;
    knob = new GUI_Surface("knob", SDL_SWSURFACE, w, w, 32, RMask, GMask, BMask, AMask);
    knob->Fill(0, knob->MapRGB(255, 255, 255));

    MemberFunctionProperty < GUI_ScrollBar > *mp;
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("knob", this, &GUI_ScrollBar::pget_Knob, &GUI_ScrollBar::pset_Knob, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("knobfocus", this, &GUI_ScrollBar::pget_KnobFocus, &GUI_ScrollBar::pset_KnobFocus, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("value", this, &GUI_ScrollBar::pget_Value, &GUI_ScrollBar::pset_Value, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("maximum", this, &GUI_ScrollBar::pget_Maximum, &GUI_ScrollBar::pset_Maximum, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("minimum", this, &GUI_ScrollBar::pget_Minimum, &GUI_ScrollBar::pset_Minimum, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("pagestep", this, &GUI_ScrollBar::pget_PageStep, &GUI_ScrollBar::pset_PageStep, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("reversed", this, &GUI_ScrollBar::pget_Reversed, &GUI_ScrollBar::pset_Reversed, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ScrollBar > ("trackingupdate", this, &GUI_ScrollBar::pget_TrackingUpdate, &GUI_ScrollBar::pset_TrackingUpdate, false);
    AddProperty(mp);

    FindProperty("width")->SetRequired(true);
    FindProperty("height")->SetRequired(true);

    GUI_EventHandler < GUI_ScrollBar > *cb = new GUI_EventHandler < GUI_ScrollBar > (this, &GUI_ScrollBar::OnChange);
    SetMovedCallback(cb);
    cb->DecRef();
}

GUI_ScrollBar::~GUI_ScrollBar(void)
{
    if (moved_callback) {
        moved_callback->DecRef();
    }
    if (knob) {
        knob->DecRef();
    }
    if (knob_focus) {
        knob_focus->DecRef();
    }
}

void GUI_ScrollBar::Update(int force)
{
    GUI_Surface *surface = NULL;

    if (!parent || !force) {
        return;
    }
    GUI_Widget::Update(force);
    if (IsFocused() && knob_focus) {
        surface = knob_focus;
    } else {
        surface = knob;
    }
    if (surface) {
        if (area.h > area.w) {
            DrawClipped(surface, parent, area, contentx + area.x, contenty + area.y + position);
        } else {
            DrawClipped(surface, parent, area, contentx + area.x + position, contenty + area.y);
        }
    }
}

void GUI_ScrollBar::SetKnobImage(GUI_Surface * image)
{
    Object::Keep((Object **) & knob, image);
}

void GUI_ScrollBar::SetKnobFocusImage(GUI_Surface * image)
{
    Object::Keep((Object **) & knob_focus, image);
}

GUI_Surface *GUI_ScrollBar::GetKnobImage(void)
{
    return knob;
}

GUI_Surface *GUI_ScrollBar::GetKnobFocusImage(void)
{
    return knob_focus;
}

void GUI_ScrollBar::SetMovedCallback(GUI_Callback * callback)
{
    Object::Keep((Object **) & moved_callback, callback);
}

void GUI_ScrollBar::SetValue(int value)
{
    if (value >= minimum && value <= maximum) {
        int oldvalue = this->value;
        this->value = value;
        if (value != oldvalue) {
            FireEvent("OnValueChange");
        }
        RecalcPosition();
        SetChanged(1);
    }
}

int GUI_ScrollBar::GetValue(void)
{
    return value;
}

void GUI_ScrollBar::SetPageStep(int step)
{
    if (step > 0) {
        this->page_step = step;
    }
}

void GUI_ScrollBar::SetReversed(bool reversed)
{
    this->reversed = reversed;
    RecalcPosition();
    SetChanged(1);
}

int GUI_ScrollBar::GetMaximum(void)
{
    return maximum;
}

void GUI_ScrollBar::SetMaximum(int maximum)
{
    this->maximum = maximum;
    RecalcPosition();
    SetChanged(1);
}

int GUI_ScrollBar::GetMinimum(void)
{
    return minimum;
}

void GUI_ScrollBar::SetMinimum(int minimum)
{
    this->minimum = minimum;
    RecalcPosition();
    SetChanged(1);
}

void GUI_ScrollBar::RecalcPosition(void)
{
    int max_visual;

    if (area.h > area.w) {
        max_visual = area.h - contenty*2;
        if (knob) {
            max_visual -= knob->GetHeight();
        }
    } else {
        max_visual = area.w - contentx*2;
        if (knob) {
            max_visual -= knob->GetWidth();
        }
    }
    if (reversed) {
        position = (int) ((((float) (maximum - minimum) - (value - minimum)) / (maximum - minimum)) * max_visual);
    } else {
        position = (int) (((float) (value - minimum) / (maximum - minimum)) * max_visual);
    }
}

void GUI_ScrollBar::RecalcValue(void)
{
    int max_visual;

    if (area.h > area.w) {
        max_visual = area.h - contenty*2;
        if (knob) {
            max_visual -= knob->GetHeight();
        }
    } else {
        max_visual = area.w - contentx*2;
        if (knob) {
            max_visual -= knob->GetWidth();
        }
    }
    value = (int) (((float) position / max_visual) * (maximum - minimum)) + minimum;
    if (reversed) {
        value = (maximum - value) + minimum;
    }
}

void GUI_ScrollBar::OnChange(Object * sender)
{
    if (oldvalue != value) {
        oldvalue = value;
        FireEvent("OnChange");
    }
}

bool GUI_ScrollBar::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    int x = event->button.x - area.x;
    int y = event->motion.y - area.y;

    switch (event->type) {
     case SDL_MOUSEMOTION:
         if (tracking_on) {
             int positionmax;
             if (area.h > area.w) {
                 position = tracking_pos + y - tracking_start;
                 positionmax = area.h - contenty*2 - knob->GetHeight();
             } else {
                 position = tracking_pos + x - tracking_start;
                 positionmax = area.w - contentx*2 - knob->GetWidth();
             }
             if (position < 0) {
                 position = 0;
             }
             if (position > positionmax) {
                 position = positionmax;
             }
             RecalcValue();
             SetChanged(1);
             if (tracking_update && moved_callback) {
                 moved_callback->Call(this);
             }
             return 1;
         }
         break;

     case SDL_MOUSEBUTTONDOWN:
         if (IsInside() && x > 0 && x < area.w) {
             switch (event->button.button) {
              case SDL_BUTTON_LEFT:
                  // if the cursor is inside the knob, start tracking
                  if (area.h > area.w) {
                      if (y >= position && y < position + knob->GetHeight()) {
                          tracking_on = 1;
                          tracking_start = y;
                          tracking_pos = position;
                      }
                  } else {
                      if (x >= position && x < position + knob->GetWidth()) {
                          tracking_on = 1;
                          tracking_start = x;
                          tracking_pos = position;
                      }
                  }
                  return 1;

              case SDL_BUTTON_WHEELUP:
                  // Scroll wheel up
                  value -= 3;
                  if (value < minimum) {
                      value = minimum;
                  }
                  RecalcPosition();
                  if (moved_callback) {
                      moved_callback->Call(this);
                  }
                  SetChanged(1);
                  return 1;

              case SDL_BUTTON_WHEELDOWN:
                  // Scroll wheel down
                  value += 3;
                  if (value > maximum) {
                      value = maximum;
                  }
                  RecalcPosition();
                  if (moved_callback) {
                      moved_callback->Call(this);
                  }
                  SetChanged(1);
                  return 1;
             }
         }
         break;

     case SDL_MOUSEBUTTONUP:
         if (tracking_on) {
             tracking_on = 0;
             if (moved_callback) {
                 moved_callback->Call(this);
             }
         } else
         if (IsInside() &&
             Inside(event->button.x, event->button.y, &area) &&
             event->button.button == SDL_BUTTON_LEFT) {
             bool above_knob = false, below_knob = false;
             if ((area.h > area.w && y < position) || (area.h < area.w && x < position)) {
                 above_knob = true;
             } else
             if ((area.h > area.w && y >= position + knob->GetHeight()) ||
                 (area.h < area.w && x >= position + knob->GetWidth())) {
                 below_knob = true;
             }
             if ((above_knob && !reversed) || (below_knob && reversed)) {
                 value -= page_step;
                 if (value < minimum) {
                     value = minimum;
                 }
             } else
             if ((below_knob && !reversed) || (above_knob && reversed)) {
                 value += page_step;
                 if (value > maximum) {
                     value = maximum;
                 }
             }
             RecalcPosition();
             if (moved_callback) {
                 moved_callback->Call(this);
             }
             SetChanged(1);
             return 1;
         }
         break;
    }
    return GUI_Widget::HandleMouseEvent(event, xoffset, yoffset);
}

void GUI_ScrollBar::Increment(void)
{
    int newvalue = value + 1;

    if (newvalue <= maximum)
        SetValue(newvalue);
    else
        SetValue(maximum);
}

void GUI_ScrollBar::Decrement(void)
{
    int newvalue = value - 1;

    if (newvalue >= minimum)
        SetValue(newvalue);
    else
        SetValue(minimum);
}

void GUI_ScrollBar::IncrementPage(void)
{
    int newvalue = value + page_step;

    if (newvalue <= maximum)
        SetValue(newvalue);
    else
        SetValue(maximum);
}

void GUI_ScrollBar::DecrementPage(void)
{
    int newvalue = value - page_step;

    if (newvalue >= minimum)
        SetValue(newvalue);
    else
        SetValue(minimum);
}

bool GUI_ScrollBar::HandleEvent(const char *action)
{
    if (GUI_Widget::HandleEvent(action)) {
        return true;
    }
    if (!strcmp(action, "up")) {
        Decrement();
        return true;
    } else
    if (!strcmp(action, "down")) {
        Increment();
        return true;
    } else
    if (!strcmp(action, "prevpage")) {
        DecrementPage();
        return true;
    } else
    if (!strcmp(action, "nextpage")) {
        IncrementPage();
        return true;
    } else;
    if (!strcmp(action, "first")) {
        SetValue(minimum);
        return true;
    } else
    if (!strcmp(action, "last")) {
        SetValue(maximum);
        return true;
    }
    return false;
}

Variant GUI_ScrollBar::pget_Knob(void)
{
    if (knob) {
        return anytovariant(knob->GetName());
    }
    return VARNULL;
}

int GUI_ScrollBar::pset_Knob(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    if (image) {
        SetKnobImage(image);
        return 0;
    }
    return -1;
}

Variant GUI_ScrollBar::pget_KnobFocus(void)
{
    if (knob_focus) {
        return anytovariant(knob_focus->GetName());
    }
    return VARNULL;
}

int GUI_ScrollBar::pset_KnobFocus(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    if (image) {
        SetKnobFocusImage(image);
        return 0;
    }
    return -1;
}

Variant GUI_ScrollBar::pget_Value(void)
{
    return anytovariant(this->value);
}

int GUI_ScrollBar::pset_Value(Variant value)
{
    SetValue(value);
    return 0;
}

Variant GUI_ScrollBar::pget_Maximum(void)
{
    return anytovariant(maximum);
}

int GUI_ScrollBar::pset_Maximum(Variant value)
{
    SetMaximum(value);
    return 0;
}

Variant GUI_ScrollBar::pget_Minimum(void)
{
    return anytovariant(minimum);
}

int GUI_ScrollBar::pset_Minimum(Variant value)
{
    SetMinimum(value);
    return 0;
}

Variant GUI_ScrollBar::pget_PageStep(void)
{
    return anytovariant(page_step);
}

int GUI_ScrollBar::pset_PageStep(Variant value)
{
    SetPageStep(value);
    return 0;
}

Variant GUI_ScrollBar::pget_Reversed(void)
{
    return anytovariant(reversed);
}

int GUI_ScrollBar::pset_Reversed(Variant value)
{
    SetReversed(value);
    return 0;
}

Variant GUI_ScrollBar::pget_TrackingUpdate(void)
{
    return anytovariant(tracking_update);
}

int GUI_ScrollBar::pset_TrackingUpdate(Variant value)
{
    tracking_update = value;
    return 0;
}
