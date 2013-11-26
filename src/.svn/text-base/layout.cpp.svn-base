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
GUI_Layout::GUI_Layout(const char *aname):Object(aname)
{
    type = TYPE_LAYOUT;
}

GUI_Layout::~GUI_Layout()
{
}

void GUI_Layout::Layout(GUI_Container * container)
{
}

GUI_VBoxLayout::GUI_VBoxLayout(const char *aname):GUI_Layout(aname)
{
    type = TYPE_VBOXLAYOUT;
}

GUI_VBoxLayout::~GUI_VBoxLayout(void)
{
}

void GUI_VBoxLayout::Layout(GUI_Container * container)
{
    SDL_Rect container_area = container->GetArea();
    int n = container->GetWidgetCount();
    int y, i;

    // sum the heights of all children, and center it
    y = container_area.h;
    for (i = 0; i < n; i++) {
        GUI_Widget *temp = container->GetWidget(i);
        y -= temp->GetArea().h;
        temp->DecRef();
    }
    y /= 2;

    // position the children
    for (i = 0; i < n; i++) {
        GUI_Widget *temp = container->GetWidget(i);
        SDL_Rect r = temp->GetArea();
        int x = (container_area.w - r.w) / 2;
        temp->SetPosition(x, y, -1, -1);
        temp->DecRef();
        y = y + r.h;
    }
}

GUI_HBoxLayout::GUI_HBoxLayout(const char *aname):GUI_Layout(aname)
{
    type = TYPE_HBOXLAYOUT;
}

GUI_HBoxLayout::~GUI_HBoxLayout(void)
{
}

void GUI_HBoxLayout::Layout(GUI_Container * container)
{
    SDL_Rect container_area = container->GetArea();
    int n = container->GetWidgetCount();
    int x, i;

    // sum the width of all children, and center it
    x = container_area.w;
    for (i = 0; i < n; i++) {
        GUI_Widget *temp = container->GetWidget(i);
        x -= temp->GetArea().w;
        temp->DecRef();
    }
    x /= 2;

    // position the children
    for (i = 0; i < n; i++) {
        GUI_Widget *temp = container->GetWidget(i);
        SDL_Rect r = temp->GetArea();
        int y = (container_area.h - r.h) / 2;
        temp->SetPosition(x, y, -1, -1);
        temp->DecRef();
        x = x + r.w;
    }
}

