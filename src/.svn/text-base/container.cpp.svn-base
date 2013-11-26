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

GUI_Container::GUI_Container(const char *aname, int x, int y, int w, int h):GUI_Widget(aname, x, y, w, h)
{
    type = TYPE_CONTAINER;
    SetTransparent(1);
    focus_index = -1;
    prevpage = 0;
    layout = 0;
    help = 0;

    MemberFunctionProperty < GUI_Container > *mp;
    mp = new MemberFunctionProperty < GUI_Container > ("widgetlist", this, &GUI_Container::pget_WidgetList, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Container > ("widgetcount", this, &GUI_Container::pget_WidgetCount, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Container > ("focusedindex", this, &GUI_Container::pget_FocusIndex, &GUI_Container::pset_FocusIndex, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Container > ("focusedwidget", this, &GUI_Container::pget_FocusWidget, &GUI_Container::pset_FocusWidget, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Container > ("help", this, &GUI_Container::pget_Help, &GUI_Container::pset_Help, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_Container > ("prevpage", this, &GUI_Container::pget_PrevPage, &GUI_Container::pset_PrevPage, false);
    AddProperty(mp);

    MemberMethodHandler < GUI_Container > *mh = new MemberMethodHandler < GUI_Container > ("updatefocus", this, 1, &GUI_Container::m_UpdateFocus);
    AddMethod(mh);
}

GUI_Container::~GUI_Container(void)
{
    lmbox_free(help);
    lmbox_free(prevpage);
    if (layout) {
        layout->DecRef();
    }
    while (widgets.size()) {
        GUI_Widget *widget = widgets.back();
        widgets.pop_back();
        if (widget->RefCount() > 1) {
            systemObject->Log(5, "%s/%s: free with refcount %d", widget->GetName(), widget->GetType(), widget->RefCount());
        }
        widget->DecRef();
    }
}

void GUI_Container::Update(int force)
{
    if (IsChanged()) {
        force = 1;
        SetChanged(0);
    }
    GUI_Widget::Update(force);
    for (int i = 0; i < widgets.size(); i++) {
        if (widgets[i]->IsVisible() && i != focus_index) {
            widgets[i]->DoUpdate(force);
        }
    }
    // Now the focused widget
    if (focus_index >= 0 && widgets[focus_index]->IsVisible()) {
        widgets[focus_index]->DoUpdate(force);
    }
}

bool GUI_Container::HandleEvent(const char *action)
{
    if (focus_index >= 0 && focus_index < widgets.size()) {
        systemObject->Log(8, "Event %s passed to focused %s", action, name);
        if (!widgets[focus_index]->IsDisabled()) {
            if (widgets[focus_index]->HandleEvent(action)) {
                systemObject->Log(2, "HandleEvent: %s: %s processed event %s", name, widgets[focus_index]->GetName(), action);
                return 1;
            }
        }
    }
    systemObject->Log(8, "Event %s unprocessed by focused in %s", action, name);
    if (!strcmp(action, "next") || !strcmp(action, "down")) {
        if (UpdateFocusWidget(1) != -1) {
            return 1;
        }
    } else
    if (!strcmp(action, "prev") || !strcmp(action, "up")) {
        if (UpdateFocusWidget(-1) != -1) {
            return 1;
        }
    }
    for (int i = 0; i < widgets.size(); i++) {
        if (!widgets[i]->IsDisabled() && i != focus_index) {
            if (widgets[i]->HandleEvent(action)) {
                systemObject->Log(2, "HandleEvent: %s: %s processed event %s", name, widgets[focus_index]->GetName(), action);
                return 1;
            }
        }
    }
    return GUI_Widget::HandleEvent(action);
}

bool GUI_Container::HandleKeyEvent(const SDL_Event * event)
{
    if (focus_index >= 0 && focus_index < widgets.size()) {
        if (widgets[focus_index]->HandleKeyEvent(event)) {
            return 1;
        }
    }
    for (int i = 0; i < widgets.size(); i++) {
        if (!widgets[i]->IsDisabled() && i != focus_index) {
            if (widgets[i]->HandleKeyEvent(event)) {
                return 1;
            }
        }
    }
    return GUI_Widget::HandleKeyEvent(event);
}

bool GUI_Container::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    if (focus_index >= 0 && focus_index < widgets.size()) {
        if (widgets[focus_index]->HandleMouseEvent(event, xoffset, yoffset)) {
            return 1;
        }
    }
    for (int i = 0; i < widgets.size(); i++) {
        if (widgets[i]->IsVisible() && i != focus_index) {
            if (widgets[i]->HandleMouseEvent(event, xoffset, yoffset)) {
                return 1;
            }
        }
    }
    return GUI_Widget::HandleMouseEvent(event, xoffset, yoffset);
}

void GUI_Container::Draw(GUI_Surface * image, const SDL_Rect * sr, const SDL_Rect * dr)
{
    if (parent) {
        SDL_Rect dest = Adjust(dr);
        SDL_Rect src;

        if (sr != NULL) {
            src = *sr;
        } else {
            src.x = src.y = 0;
            src.w = image->GetWidth();
            src.h = image->GetHeight();
        }
        if (ClipRect(&src, &dest, &area)) {
            parent->Draw(image, &src, &dest);
        }
    }
}

void GUI_Container::Erase(const SDL_Rect * rp)
{
    if (parent) {
        SDL_Rect dest = Adjust(rp);
        if (ClipRect(NULL, &dest, &area)) {
            if (IsTransparent()) {
                parent->Erase(&dest);
            }
            int idx = GetBackgroundIdx();
            if (background.img[idx]) {
                parent->TileImage(background.img[idx], &dest, 0, 0);
            }
        }
    }
}

void GUI_Container::UpdateLayout(void)
{
    if (layout) {
        layout->Layout(this);
    }
}

void GUI_Container::SetLayout(GUI_Layout * a_layout)
{
    if (Object::Keep((Object **) &layout, a_layout)) {
        UpdateLayout();
        SetChanged(1);
    }
}

GUI_Widget *GUI_Container::FindWidget(const char *aname)
{
    GUI_Widget *widget = 0;
    Lock();
    for (int i = 0; i < widgets.size(); i++) {
         if (!strcmp(aname, widgets[i]->GetName())) {
             widget = widgets[i];
             widget->IncRef();
             break;
         }
    }
    Unlock();
    return widget;
}

int GUI_Container::ContainsWidget(GUI_Widget * widget)
{
    int index = -1;
    if (widget) {
        Lock();
        for (int i = 0; i < widgets.size(); i++) {
            if (widgets[i] == widget) {
                index = i;
                break;
            }
        }
        Unlock();
    }
    return index;
}

void GUI_Container::AddWidget(GUI_Widget * widget)
{
    if (!widget || ContainsWidget(widget) > -1) {
        return;
    }
    // IncRef early, to prevent reparenting from freeing the child
    widget->IncRef();
    // reparent if necessary
    GUI_Widget *parent = widget->GetParent();
    if (parent) {
        parent->RemoveWidget(widget);
    }
    widget->SetParent(this);
    Lock();
    widgets.push_back(widget);
    Unlock();
    widget->FireEvent("OnAdd");
    UpdateLayout();
}

void GUI_Container::RemoveWidget(GUI_Widget * widget)
{
    int i, j;

    widget->SetParent(0);
    widget->DecRef();
    Lock();
    for (int i = 0; i < widgets.size(); i++) {
        if (widget == widgets[i]) {
            widgets.erase(widgets.begin() + i);
            break;
        }
    }
    Unlock();
    widget->FireEvent("OnRemove");
    UpdateLayout();
}

GUI_Widget *GUI_Container::GetWidget(int index)
{
    Lock();
    GUI_Widget *widget = 0;
    if (index >= 0 && index < widgets.size()) {
        widget = widgets[index];
        widget->IncRef();
    }
    Unlock();
    return widget;
}

int GUI_Container::GetWidgetCount(void)
{
    Lock();
    int rc = widgets.size();
    Unlock();
    return rc;
}

int GUI_Container::GetFocusIndex(void)
{
    Lock();
    int rc = focus_index;
    Unlock();
    return rc;
}

GUI_Widget *GUI_Container::GetFocusWidget()
{
    return GetWidget(GetFocusIndex());
}

int GUI_Container::SetFocusWidget(GUI_Widget *widget)
{
     if (widget->IsSelectable()) {
         return SetFocusWidget(ContainsWidget(widget));
     }
     return -1;
}

int GUI_Container::SetFocusWidget(int index)
{
    int locked = 1;
    Lock();
    // Take focus from current widget
    if (focus_index >= 0 && index != focus_index) {
        GUI_Widget *widget = widgets[focus_index];
        widget->IncRef();
        Unlock();
        locked = 0;
        widget->OnLostFocus();
        widget->SetFocused(0);
        systemObject->Log(5, "SetFocusWidget: %s: lost focus %s, %d", name, widget->GetName(), focus_index);
        widget->DecRef();
    }
    // Now give focus to new widget
    if (index >= 0 && index < widgets.size()) {
        focus_index = index;
        GUI_Widget *widget = widgets[focus_index];
        widget->IncRef();
        Unlock();
        locked = 0;
        widget->OnGotFocus();
        widget->SetFocused(1);
        systemObject->Log(5, "SetFocusWidget: %s: got focus %s, %d", name, widget->GetName(), focus_index);
        widget->DecRef();
    }
    if (locked) {
        Unlock();
    }
    SetChanged(1);
    return focus_index;
}

int GUI_Container::UpdateFocusWidget(int delta)
{
    Lock();
    int index = focus_index + delta;
    // Make sure the next widget is focusable
    for (int i = 0; i < widgets.size(); i++) {
        if (index < 0) {
            index = widgets.size() - 1;
        }
        if (index >= widgets.size()) {
            index = 0;
        }
        if (widgets[index]->IsSelectable()) {
            break;
        }
        index += delta < 0 ? -1 : 1;
    }
    // Widget found, swith focus to it
    if (index >= 0 && index < widgets.size() && widgets[index]->IsSelectable()) {
        Unlock();
        index = SetFocusWidget(index);
        Lock();
        if (index != -1) {
            focus_index = index;
        }
    }
    Unlock();
    return focus_index;
}

DynamicObject *GUI_Container::FindObject(const char *name)
{
    Lock();
    for (int i = 0; i < widgets.size(); i++) {
        if (!strcmp(widgets[i]->GetName(), name)) {
            widgets[i]->IncRef();
            Unlock();
            return widgets[i];
        }
    }
    Unlock();
    return DynamicObject::FindObject(name);
}

int GUI_Container::OnGotFocus(void)
{
    GUI_Widget::OnGotFocus();
    StartObjects();
    // If there is no focused widget on the page, advance the focus to the next widget
    UpdateFocusWidget(0);
    return 1;
}

int GUI_Container::OnLostFocus(void)
{
    StopObjects();
    // Unfocus current widget
    SetFocusWidget(-1);
    GUI_Widget::OnLostFocus();
    return 1;
}

void GUI_Container::StartObjects(void)
{
    DynamicObject::StartObjects();
    for (int i = 0; i < widgets.size(); i++) {
        widgets[i]->OnStart();
    }
}

void GUI_Container::StopObjects(void)
{
    DynamicObject::StopObjects();
    for (int i = 0; i < widgets.size(); i++) {
        widgets[i]->OnStop();
    }
}

string GUI_Container::GetWidgetList(void)
{
    string list;
    Lock();
    for (int i = 0; i < widgets.size(); i++) {
        list += widgets[i]->GetName();
        if (i < widgets.size() - 1) {
            list += " ";
        }
    }
    Unlock();
    return list;
}

void GUI_Container::SetPrevPage(const char *text)
{
    Lock();
    if (!prevpage || (strcmp(prevpage, text) && strcmp(name, text))) {
        lmbox_free(prevpage);
        prevpage = lmbox_strdup(text);
    }
    Unlock();
}

const char *GUI_Container::GetPrevPage()
{
    Lock();
    const char *rc = (prevpage && strcmp(name, prevpage) ? prevpage : "");
    Unlock();
    return rc;
}

void GUI_Container::SetHelp(const char *text)
{
    Lock();
    lmbox_free(help);
    help = strdup(text);
    Unlock();
}

Variant GUI_Container::pget_Help(void)
{
    return anytovariant(help ? help : "");
}

int GUI_Container::pset_Help(Variant value)
{
    SetHelp(value);
    return 0;
}

Variant GUI_Container::pget_PrevPage(void)
{
    return anytovariant(GetPrevPage());
}

int GUI_Container::pset_PrevPage(Variant value)
{
    SetPrevPage(value);
    return 0;
}


Variant GUI_Container::pget_WidgetCount(void)
{
    return anytovariant(GetWidgetCount());
}

Variant GUI_Container::pget_WidgetList(void)
{
    string rc = GetWidgetList().c_str();
    return anytovariant(rc.c_str());
}

Variant GUI_Container::pget_FocusIndex(void)
{
    return anytovariant(GetFocusIndex());
}

int GUI_Container::pset_FocusIndex(Variant value)
{
    SetFocusWidget(value);
    return 0;
}

Variant GUI_Container::pget_FocusWidget(void)
{
    Variant rc;
    GUI_Widget *widget = GetFocusWidget();
    if (widget) {
        rc = widget->GetName();
        widget->DecRef();
    }
    return rc;
}

int GUI_Container::pset_FocusWidget(Variant value)
{
    GUI_Widget *widget = FindWidget(value);
    if (widget) {
        SetFocusWidget(widget);
        widget->DecRef();
    }
    return 0;
}

Variant GUI_Container::m_UpdateFocus(int numargs, Variant args[])
{
    UpdateFocusWidget(args[0]);
    return anytovariant(focus_index);
}
