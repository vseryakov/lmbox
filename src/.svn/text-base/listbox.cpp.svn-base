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


GUI_ListBox::GUI_ListBox(const char *aname, int x, int y, int w, int h, GUI_Font * afont):GUI_Widget(aname, x, y, w, h)
{
    type = TYPE_LISTBOX;
    SetFlags(WIDGET_SELECTABLE|OBJECT_SEEKABLE);
    SetTransparent(1);

    fontsize = DEFAULT_FONTSIZE;
    textcolor.r = 255;
    textcolor.g = 255;
    textcolor.b = 255;
    textcolor.unused = 255;

    selitemtextcolor.r = 255;
    selitemtextcolor.g = 255;
    selitemtextcolor.b = 0;
    selitemtextcolor.unused = 255;

    selitemtextcolorfocus.r = 255;
    selitemtextcolorfocus.g = 255;
    selitemtextcolorfocus.b = 0;
    selitemtextcolorfocus.unused = 255;

    selected_item_image = NULL;
    selected_item_focus_image = NULL;
    selected_item_style = 0;

    font = afont;
    if (font) {
        font->IncRef();
    }
    wrap = 0;
    topindex = 0;
    searchtext[0] = 0;
    selectedindex = -1;
    oneclick = false;
    scrolling = false;
    wheelpagestep = 1;             // scroll by 1 when using mouse wheel
    visibleitems = 0;
    itemborderx = 0;
    itembordery = 0;
    selitemborderx = 0;
    selitembordery = 0;
    useritemheight = -1;        // automatic (text height)
    scrollbarmode = AUTOPROPERTY_AUTO;
    scrollbarvisible = false;

    scrollbar = new GUI_ScrollBar(name, x + w, y, 16, h);
    scrollbar->SetValue(0);
    scrollbar->ClearFlags(WIDGET_SELECTABLE);
    RecalcItemHeight();

    GUI_EventHandler < GUI_ListBox > *cb = new GUI_EventHandler < GUI_ListBox > (this, &GUI_ListBox::OnScrollBarMove);
    scrollbar->SetMovedCallback(cb);
    cb->DecRef();

    MemberFunctionProperty < GUI_ListBox > *mp;
    mp = new MemberFunctionProperty < GUI_ListBox > ("wrap", this, &GUI_ListBox::pget_Wrap, &GUI_ListBox::pset_Wrap, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("oneclick", this, &GUI_ListBox::pget_OneClick, &GUI_ListBox::pset_OneClick, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("font", this, &GUI_ListBox::pget_Font, &GUI_ListBox::pset_Font, true);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("fontsize", this, &GUI_ListBox::pget_FontSize, &GUI_ListBox::pset_FontSize, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("fontcolor", this, &GUI_ListBox::pget_FontColor, &GUI_ListBox::pset_FontColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selectedindex", this, &GUI_ListBox::pget_SelectedIndex, &GUI_ListBox::pset_SelectedIndex, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selectedtext", this, &GUI_ListBox::pget_SelectedText, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selecteddata", this, &GUI_ListBox::pget_SelectedData, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("searchtext", this, &GUI_ListBox::pget_SearchText, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("count", this, &GUI_ListBox::pget_Count, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitembackground", this, &GUI_ListBox::pget_SelItemBackground, &GUI_ListBox::pset_SelItemBackground, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitembackgroundfocus", this, &GUI_ListBox::pget_SelItemBackgroundFocus, &GUI_ListBox::pset_SelItemBackgroundFocus, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitembackgroundstyle", this, &GUI_ListBox::pget_SelItemBackgroundStyle, &GUI_ListBox::pset_SelItemBackgroundStyle, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitembackgroundcolor", this, &GUI_ListBox::pget_SelItemBackgroundColor, &GUI_ListBox::pset_SelItemBackgroundColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitembackgroundfocuscolor", this, &GUI_ListBox::pget_SelItemBackgroundFocusColor, &GUI_ListBox::pset_SelItemBackgroundFocusColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitemfontcolor", this, &GUI_ListBox::pget_SelItemFontColor, &GUI_ListBox::pset_SelItemFontColor, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitemfontcolorfocus", this, &GUI_ListBox::pget_SelItemFontColorFocus, &GUI_ListBox::pset_SelItemFontColorFocus, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox >  ("scrollbarbackground", this, &GUI_ListBox::pget_ScrollBarBackground, &GUI_ListBox::pset_ScrollBarBackground, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("scrollbarbackgroundstyle", this, &GUI_ListBox::pget_ScrollBarBackgroundStyle, &GUI_ListBox::pset_ScrollBarBackgroundStyle, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("scrollbarknob", this, &GUI_ListBox::pget_ScrollBarKnob, &GUI_ListBox::pset_ScrollBarKnob, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("scrollbarwidth", this, &GUI_ListBox::pget_ScrollBarWidth, &GUI_ListBox::pset_ScrollBarWidth, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("scrollbar", this, &GUI_ListBox::pget_ScrollBar, &GUI_ListBox::pset_ScrollBar, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitemborderx", this, &GUI_ListBox::pget_SelItemBorderX, &GUI_ListBox::pset_SelItemBorderX, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("selitembordery", this, &GUI_ListBox::pget_SelItemBorderY, &GUI_ListBox::pset_SelItemBorderY, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("itemborderx", this, &GUI_ListBox::pget_ItemBorderX, &GUI_ListBox::pset_ItemBorderX, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("itembordery", this, &GUI_ListBox::pget_ItemBorderY, &GUI_ListBox::pset_ItemBorderY, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("itemheight", this, &GUI_ListBox::pget_ItemHeight, &GUI_ListBox::pset_ItemHeight, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("itemlist", this, &GUI_ListBox::pget_ItemList, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("datalist", this, &GUI_ListBox::pget_DataList, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_ListBox > ("wheelpagestep", this, &GUI_ListBox::pget_WheelPageStep, &GUI_ListBox::pset_WheelPageStep, false);
    AddProperty(mp);

    MemberMethodHandler < GUI_ListBox > *mh;
    mh = new MemberMethodHandler < GUI_ListBox > ("additem", this, 1, &GUI_ListBox::m_AddItem);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("additemdata", this, 2, &GUI_ListBox::m_AddItemData);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("removeitem", this, 1, &GUI_ListBox::m_RemoveItem);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("removelastitem", this, 0, &GUI_ListBox::m_RemoveLastItem);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("clear", this, 0, &GUI_ListBox::m_Clear);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("sort", this, 0, &GUI_ListBox::m_Sort);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("setitemtext", this, 2, &GUI_ListBox::m_SetItemText);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("setitemdata", this, 2, &GUI_ListBox::m_SetItemData);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("itemtext", this, 1, &GUI_ListBox::m_ItemText);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("itemdata", this, 1, &GUI_ListBox::m_ItemData);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("searchitem", this, 1, &GUI_ListBox::m_SearchItem);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("matchitem", this, 1, &GUI_ListBox::m_MatchItem);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("searchitemdata", this, 1, &GUI_ListBox::m_SearchItemData);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_ListBox > ("matchitemdata", this, 1, &GUI_ListBox::m_MatchItemData);
    AddMethod(mh);
}

GUI_ListBox::~GUI_ListBox()
{
    Clear();
    if (font) {
        font->DecRef();
    }
    if (scrollbar) {
        scrollbar->DecRef();
    }
    if (selected_item_image) {
        selected_item_image->DecRef();
    }
    if (selected_item_focus_image) {
        selected_item_focus_image->DecRef();
    }
}

void GUI_ListBox::Update(int force)
{
    if (parent == 0) {
        return;
    }
    GUI_Widget::Update(force);
    if (force) {
        SDL_Rect clip;
        int itemx, itemy;
        int textx, texty;
        GUI_Surface *surface;
        SDL_Rect a = { area.x + contentx, area.y + contenty, area.w - contentx*2, area.h - contenty*2 };
        for (unsigned int i = 0; i + topindex < listitems.size() && i < visibleitems; i++) {
            itemx = a.x + border.x;
            itemy = a.y + border.y + (i * itemheight);
            clip.x = itemx + selitemborderx;
            clip.w = a.w - (border.x * 2) - (selitemborderx * 2);
            clip.y = itemy + selitembordery;
            clip.h = itemheight - (selitembordery * 2);
            if (clip.y + clip.h > a.y + a.h - border.y) {
                clip.h = a.h - (itemy - a.y) - border.y - (selitembordery * 2);
            }
            if (i + topindex == selectedindex) {
                // The selected item
                if (IsFocused() && (selected_item_focus_image != NULL)) {
                    surface = selected_item_focus_image;
                } else {
                    surface = selected_item_image;
                }
                if (surface) {
                    switch (selected_item_style) {
                     case BACKGROUND_NORMAL:
                          DrawClipped(surface, parent, clip, clip.x, clip.y);
                          break;

                     case BACKGROUND_TILED:
                          parent->TileImage(surface, &clip, 0, 0);
                          break;
                    }
                }
                SDL_Color c;
                if (IsFocused()) {
                    c = selitemtextcolorfocus;
                } else {
                    c = selitemtextcolor;
                }
                // Now render the text, as selected
                surface = font->RenderQuality(listitems[i + topindex], c);
            } else {
                // Now render the text, as normal
                surface = font->RenderQuality(listitems[i + topindex], textcolor);
            }
            if (surface != NULL) {
                // Ensure text is centred within available item height
                textx = itemx + itemborderx;
                texty = itemy + ((itemheight - surface->GetHeight()) / 2);
                // Draw the text
                clip.x = itemx + itemborderx;
                clip.y = itemy + itembordery;
                clip.w = a.w - (border.x * 2) - (itemborderx * 2);
                clip.h = itemheight;
                if (clip.y + clip.h > a.y + a.h - border.y) {
                    clip.h = a.h - (clip.y - a.y) - border.y;
                }
                DrawClipped(surface, parent, clip, textx, texty);
                surface->DecRef();
            }
        }
    }
}

int GUI_ListBox::OnGotFocus(void)
{
    if (!GUI_Widget::OnGotFocus()) {
        return 0;
    }
    SetSearchText(0);
    return 1;
}

int GUI_ListBox::OnLostFocus(void)
{
    return GUI_Widget::OnLostFocus();
}

void GUI_ListBox::OnStart(void)
{
    if (!scrollbar->GetParent() && scrollbar->GetWidth() && scrollbar->IsVisible()) {
        scrollbar->SetPosition(area.x + area.w, area.y, -1, area.h);
        GUI_Page *page = GetPage();
        if (page) {
            page->AddWidget(scrollbar);
            page->DecRef();
        }
    }
}

int GUI_ListBox::ItemAtPoint(int x, int y)
{
    if (y >= area.y + border.y && y < area.y + area.h - border.y)
        return ((y - area.y - border.y) / itemheight) + topindex;
    else
        return -1;
}

bool GUI_ListBox::HandleEvent(const char *action)
{
    if (GUI_Widget::HandleEvent(action)) {
        return true;
    }
    if (!strcmp(action, "accept")) {
        ChooseSelection();
        SetSearchText(0);
        return true;
    } else
    if (!strcmp(action, "up")) {
        SelectPrevious(1);
        SetSearchText(0);
        return true;
    } else
    if (!strcmp(action, "down")) {
        SelectNext(1);
        SetSearchText(0);
        return true;
    } else
    if (!strcmp(action, "prevpage")) {
        SelectPrevious(visibleitems);
        SetSearchText(0);
        return true;
    } else
    if (!strcmp(action, "nextpage")) {
        SelectNext(visibleitems);
        SetSearchText(0);
        return true;
    } else
    if (!strcmp(action, "first")) {
        SetSelectedIndex(0);
        SetSearchText(0);
        return true;
    } else
    if (!strcmp(action, "last")) {
        SetSelectedIndex(listitems.size() - 1);
        SetSearchText(0);
        return true;
    } else
    if (!strncmp(action, "number", 6) || !strncmp(action, "key", 3) || !strcmp(action, "space")) {
        if (!(GetFlags() & OBJECT_SEEKABLE)) {
            return false;
        }
        // Conversion map for phone-like keypad
        static char c, map[] = { '0', '1', 'a', 'd', 'g', 'j', 'm', 'p', 't', 'm' };
        switch (action[0]) {
        case 'n':
            c = map[action[6] - '0'];
            break;
        case 's':
            c = ' ';
            break;
        default:
            c = action[3];
        }
        int searchlen = strlen(searchtext);
        if (searchlen >= sizeof(searchtext) - 2) {
            return false;
        }
        searchtext[searchlen++] += c;
        searchtext[searchlen] = 0;
        for (int i = 0; i < listitems.size(); i++) {
            if (!strncasecmp(listitems[i], searchtext, searchlen)) {
                SetSelectedIndex(i);
                break;
            }
        }
        return true;
    }
    return false;
}

bool GUI_ListBox::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    int index;
    switch (event->type) {
     case SDL_MOUSEBUTTONDOWN:
         if (IsInside()) {
             switch (event->button.button) {
              case SDL_BUTTON_LEFT:
              case SDL_BUTTON_MIDDLE:
                  index = ItemAtPoint(event->button.x, event->button.y);
                  if (index >= 0 && index < listitems.size()) {
                      int lastselectedindex = selectedindex;
                      SetSelectedIndex(index);
                      if (oneclick || event->button.button == SDL_BUTTON_MIDDLE) {
                          OnChoose();
                          return 1;
                      }
                  }
                  break;

              case SDL_BUTTON_WHEELUP:
                  SelectPrevious(wheelpagestep ? wheelpagestep : visibleitems/2 + 1);
                  SetSearchText(0);
                  return 1;

              case SDL_BUTTON_WHEELDOWN:
                  SelectNext(wheelpagestep ? wheelpagestep : visibleitems/2 + 1);
                  SetSearchText(0);
                  return 1;
             }
         }
         break;

     case SDL_MOUSEBUTTONUP:
         break;

     case SDL_MOUSEMOTION:
         if (IsInside()) {
             if (oneclick) {
                 int index = ItemAtPoint(event->button.x, event->button.y);
                 if (index >= 0) {
                     SetSelectedIndex(index);
                 }
             }
         }
         break;
    }
    return GUI_Widget::HandleMouseEvent(event, xoffset, yoffset);
}

void GUI_ListBox::SetFont(GUI_Font * afont)
{
    Object::Keep((Object **) & font, afont);
    RecalcItemHeight();
    SetChanged(1);
}

void GUI_ListBox::SetTextColor(SDL_Color c)
{
    textcolor = c;
    SetChanged(1);
}

void GUI_ListBox::SetSelectedItemTextColor(SDL_Color c)
{
    selitemtextcolor = c;
    selitemtextcolorfocus = c;
    SetChanged(1);
}

void GUI_ListBox::SetSelectedItemTextColorFocus(SDL_Color c)
{
    selitemtextcolorfocus = c;
    SetChanged(1);
}

void GUI_ListBox::SetScrollBarMode(int scrollbarmode)
{
    this->scrollbarmode = scrollbarmode;
    CheckScrollBarVisible();
}

void GUI_ListBox::SetScrollBarWidth(int width)
{
    scrollbar->SetWidth(width);
    SetChanged(1);
}

void GUI_ListBox::SetBorderX(int pixels)
{
    this->border.x = pixels;
    RecalcItemHeight();
    SetChanged(1);
}

void GUI_ListBox::SetBorderY(int pixels)
{
    this->border.y = pixels;
    RecalcItemHeight();
    SetChanged(1);
}

void GUI_ListBox::SetSelItemBorderX(int pixels)
{
    this->selitemborderx = pixels;
    SetChanged(1);
}

void GUI_ListBox::SetSelItemBorderY(int pixels)
{
    this->selitembordery = pixels;
    SetChanged(1);
}

void GUI_ListBox::SetItemBorderX(int pixels)
{
    this->itemborderx = pixels;
    SetChanged(1);
}

void GUI_ListBox::SetItemBorderY(int pixels)
{
    this->itembordery = pixels;
    SetChanged(1);
}

void GUI_ListBox::SetItemHeight(int pixels)
{
    this->useritemheight = pixels;
    RecalcItemHeight();
    SetChanged(1);
}

void GUI_ListBox::RecalcItemHeight(void)
{
    Lock();
    if (useritemheight == -1 && font) {
        SDL_Rect textsize = font->GetTextSize("testing, 123");
        itemheight = textsize.h;
    } else {
        itemheight = useritemheight;
    }
    visibleitems = (area.h - (border.y * 2)) / itemheight;
    SetupScrollBar();
    Unlock();
}

char *GUI_ListBox::GetItemData(const char *item)
{
    return (char*)&item[strlen(item)+1];
}

char *GUI_ListBox::MakeItemData(const char *text, const char *data)
{
    char *item = (char*)calloc(1, strlen(text) + 2 + (data ? strlen(data) : 0) + 1);
    strcpy(item, text);
    if (data) {
        strcpy(&item[strlen(item)+1], data);
    }
    return item;
}

void GUI_ListBox::AddItemData(const char *text, const char *data)
{
    if (!text) {
        return;
    }
    Lock();
    int numitems = listitems.size();
    listitems.push_back(MakeItemData(text, data));
    SetupScrollBar();
    Unlock();
    if (numitems == 0) {
        SetSelectedIndex(0);
    }
    SetChanged(1);
}

void GUI_ListBox::RemoveItem(int index)
{
    if (index >= 0 && index < listitems.size()) {
        Lock();
        free((void*)listitems[index]);
        vector < const char *>::iterator iter = listitems.begin() + index;
        listitems.erase(iter);
        Unlock();
        SetupScrollBar();
        // Check if we need to move the selection
        if (listitems.size() > 0) {
            if (selectedindex > listitems.size() - 1) {
                // Removed item was the last item
                SelectLast();
            } else
            if (index < selectedindex) {
                // Removed item was before the current item
                SelectPrevious(1);
            }
            else
            if (index == selectedindex) {
                InternalSetSelectedIndex(index, true);
            }
        } else {
            selectedindex = -1;
        }
        SetChanged(1);
    }
}

void GUI_ListBox::Clear(void)
{
    Lock();
    while (listitems.size() > 0) {
        free((void*)listitems.back());
        listitems.pop_back();
    }
    topindex = 0;
    selectedindex = -1;
    Unlock();
    SetupScrollBar();
    SetChanged(1);
}

static int listitemsort(const void *a, const void *b)
{
    char **s1 = (char**)a, **s2 = (char**)b;
    return strcmp(*s1, *s2);
}

void GUI_ListBox::Sort(void)
{
    Lock();
    const char **slist = (const char**)malloc(sizeof(char*) * listitems.size());
    for (int i = 0; i < listitems.size(); i++) {
        slist[i] = listitems[i];
    }
    qsort(slist, listitems.size(), sizeof(char*), listitemsort);
    for (int i = 0; i < listitems.size(); i++) {
        listitems[i] = slist[i];
    }
    free(slist);
    Unlock();
    SetupScrollBar();
    // force to redraw listbox and call events
    InternalSetSelectedIndex(selectedindex, true);
    SetChanged(1);
}

void GUI_ListBox::SetItemText(int index, const char *text)
{
    Lock();
    if (index >= 0 && index < listitems.size()) {
        char *data = GetItemData(listitems[index]);
        char *item = MakeItemData(text, data);
        free((void*)listitems[index]);
        listitems[index] = item;
        if (index >= topindex && index < topindex + visibleitems) {
            SetChanged(1);
        }
    }
    Unlock();
}

void GUI_ListBox::SetSearchText(const char *data)
{
    Lock();
    strcpy(searchtext, data ? data : "");
    Unlock();
}

void GUI_ListBox::SetItemData(int index, const char *data)
{
    Lock();
    if (index >= 0 && index < listitems.size()) {
        char *item = MakeItemData(listitems[index], data);
        free((void*)listitems[index]);
        listitems[index] = item;
    }
    Unlock();
}

void GUI_ListBox::SetSelectedItemImage(GUI_Surface * surface)
{
    Object::Keep((Object **) & selected_item_image, surface);
}

void GUI_ListBox::SetSelectedItemFocusImage(GUI_Surface * surface)
{
    Object::Keep((Object **) & selected_item_focus_image, surface);
}

void GUI_ListBox::SetSelectedItemBackgroundColor(SDL_Color c)
{
    GUI_Surface *bg = new GUI_Surface("selitem", SDL_SWSURFACE, 100, 100, 32, RMask, GMask, BMask, AMask);
    bg->Fill(NULL, bg->MapRGB(c.r, c.g, c.b));
    SetSelectedItemImage(bg);
    bg->DecRef();
}

void GUI_ListBox::SetSelectedItemBackgroundFocusColor(SDL_Color c)
{
    GUI_Surface *bg = new GUI_Surface("selitemfocus", SDL_SWSURFACE, 100, 100, 32, RMask, GMask, BMask, AMask);
    bg->Fill(NULL, bg->MapRGB(c.r, c.g, c.b));
    SetSelectedItemFocusImage(bg);
    bg->DecRef();
}

void GUI_ListBox::SetSelectedItemStyle(int style)
{
    selected_item_style = style;
    SetChanged(1);
}

void GUI_ListBox::SetScrollBarKnobImage(GUI_Surface * surface)
{
    scrollbar->SetKnobImage(surface);
}

void GUI_ListBox::SetScrollBarBackgroundImage(GUI_Surface * surface)
{
    scrollbar->SetBackground(surface);
}

void GUI_ListBox::SetScrollBarBackgroundStyle(int style)
{
    scrollbar->SetBackgroundStyle(style);
}

void GUI_ListBox::SetSelectedIndex(int index)
{
    InternalSetSelectedIndex(index, false);
}

void GUI_ListBox::InternalSetSelectedIndex(int index, bool forceevent)
{
    Lock();
    int lastindex = selectedindex;
    if (!scrolling) {
        scrolling = true;
        if (index >= 0 && index < listitems.size()) {
            selectedindex = index;
            if (selectedindex > topindex + (visibleitems - 1)) {
                topindex = selectedindex - (visibleitems - 1) / 2;
            } else
            if (selectedindex < topindex) {
                topindex = selectedindex;
            }
            scrollbar->SetValue(topindex);
            SetChanged(1);
            if ((selectedindex != lastindex || forceevent)) {
                OnSelectionChange();
            }
        }
        scrolling = false;
    }
    Unlock();
}

int GUI_ListBox::GetSelectedIndex(void)
{
    int rc = -1;
    if (listitems.size() > 0) {
        rc = selectedindex;
    }
    return rc;
}

int GUI_ListBox::GetVisibleItems(void)
{
    return visibleitems;
}

void GUI_ListBox::SetTopIndex(int index)
{
    Lock();
    if (listitems.size() > visibleitems) {
        if (index < 0) {
            topindex = 0;
        } else
        if (index > listitems.size() - visibleitems) {
            topindex = listitems.size() - visibleitems;
        } else {
            topindex = index;
        }
        scrollbar->SetValue(topindex);
        SetChanged(1);
    }
    Unlock();
}

void GUI_ListBox::SelectNext(int count)
{
    Lock();
    if (selectedindex < listitems.size() - 1 && listitems.size() > 0) {
        // Select the next item
        if (selectedindex + count > listitems.size() - 1) {
            SetSelectedIndex(listitems.size() - 1);
        } else {
            SetSelectedIndex(selectedindex + count);
        }
    } else {
        if (wrap) {
            SetSelectedIndex(0);
        }
    }
    Unlock();
}

void GUI_ListBox::SelectPrevious(int count)
{
    Lock();
    if (selectedindex > 0) {
        // Select the previous item
        if (selectedindex - count < 0) {
            SetSelectedIndex(0);
        } else {
            SetSelectedIndex(selectedindex - count);
        }
    } else {
        if (wrap) {
            SetSelectedIndex(listitems.size() - 1);
        }
    }
    Unlock();
}

void GUI_ListBox::SelectFirst(void)
{
    if (selectedindex != 0) {
        SetSelectedIndex(0);
    }
}

void GUI_ListBox::SelectLast(void)
{
    if (selectedindex != listitems.size() - 1) {
        SetSelectedIndex(listitems.size() - 1);
    }
}

void GUI_ListBox::OnScrollBarMove(Object * sender)
{
    SetTopIndex(scrollbar->GetValue());
}

void GUI_ListBox::SetupScrollBar(void)
{
    if (listitems.size() > visibleitems) {
        scrollbar->SetMaximum(listitems.size() - visibleitems);
    } else {
        scrollbar->SetMaximum(0);
        scrollbar->SetValue(0);
    }
    scrollbar->SetPageStep(visibleitems);
    CheckScrollBarVisible();
}

void GUI_ListBox::CheckScrollBarVisible(void)
{
    if (scrollbarmode == AUTOPROPERTY_OFF ||
        (scrollbarmode == AUTOPROPERTY_AUTO &&
         listitems.size() <= visibleitems)) {
        scrollbarvisible = false;
        scrollbar->SetVisible(0);
        scrollbar->SetChanged(1);
    } else {
        scrollbarvisible = true;
        scrollbar->SetVisible(1);
        scrollbar->SetChanged(1);
    }
    SetChanged(1);
}

void GUI_ListBox::ChooseSelection(void)
{
    if (selectedindex >= 0 && listitems.size() > 0) {
        FireEvent("OnChoose");
    }
}

void GUI_ListBox::OnChoose(void)
{
    ChooseSelection();
}

void GUI_ListBox::OnSelectionChange(void)
{
    FireEvent("OnSelectionChange");
}

void GUI_ListBox::DoubleClicked(int x, int y)
{
    ChooseSelection();
}

Variant GUI_ListBox::pget_Wrap(void)
{
    return anytovariant(wrap);
}

int GUI_ListBox::pset_Wrap(Variant value)
{
    wrap = value;
    return 0;
}

Variant GUI_ListBox::pget_WheelPageStep(void)
{
    return anytovariant(wheelpagestep);
}

int GUI_ListBox::pset_WheelPageStep(Variant value)
{
    wheelpagestep = value;
    return 0;
}

Variant GUI_ListBox::pget_OneClick(void)
{
    return anytovariant(oneclick);
}

int GUI_ListBox::pset_OneClick(Variant value)
{
    oneclick = (bool) value;
    return 0;
}

Variant GUI_ListBox::pget_Font(void)
{
    return anytovariant(font->GetName());
}

int GUI_ListBox::pset_Font(Variant value)
{
    GUI_Font *font = systemObject->LoadFont(value, fontsize);
    if (font) {
        SetFont(font);
        return 0;
    }
    return -1;
}

Variant GUI_ListBox::pget_FontSize(void)
{
    return anytovariant(fontsize);
}

int GUI_ListBox::pset_FontSize(Variant value)
{
    fontsize = value;
    if (font)
        return pset_Font(anytovariant(font->GetName()));
    else
        return 0;
}

Variant GUI_ListBox::pget_FontColor(void)
{
    return anytovariant(colorToStr(textcolor));
}

int GUI_ListBox::pset_FontColor(Variant value)
{
    SetTextColor(strToColor(value));
    return 0;
}

Variant GUI_ListBox::pget_SelItemBackgroundColor(void)
{
     return VARNULL;
}

int GUI_ListBox::pset_SelItemBackgroundColor(Variant value)
{
    SetSelectedItemBackgroundColor(strToColor(value));
    return 0;
}
Variant GUI_ListBox::pget_SelItemBackgroundFocusColor(void)
{
    return VARNULL;
}

int GUI_ListBox::pset_SelItemBackgroundFocusColor(Variant value)
{
    SetSelectedItemBackgroundFocusColor(strToColor(value));
    return 0;
}

Variant GUI_ListBox::pget_SelectedIndex(void)
{
    return anytovariant(GetSelectedIndex());
}

int GUI_ListBox::pset_SelectedIndex(Variant value)
{
    int idx = value;
    if (idx > listitems.size() - 1 || idx < 0) {
        return -1;
    } else {
        SetSelectedIndex(value);
        return 0;
    }
}

Variant GUI_ListBox::pget_SearchText(void)
{
    return anytovariant(searchtext ? searchtext : "");
}

Variant GUI_ListBox::pget_SelectedText(void)
{
    if (listitems.size() > 0)
        return anytovariant(listitems[selectedindex]);
    else
        return VARNULL;
}

Variant GUI_ListBox::pget_SelectedData(void)
{
    if (selectedindex < listitems.size()) {
        char *data = GetItemData(listitems[selectedindex]);
        return anytovariant(data ? data : "");
    }
    return VARNULL;
}

Variant GUI_ListBox::pget_Count(void)
{
    return anytovariant((int)listitems.size());
}

Variant GUI_ListBox::pget_SelItemBackground(void)
{
    if (selected_item_image)
        return anytovariant(selected_item_image->GetName());
    else
        return VARNULL;
}

int GUI_ListBox::pset_SelItemBackground(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    if (image) {
        SetSelectedItemImage(image);
    }
    return 0;
}

Variant GUI_ListBox::pget_SelItemBackgroundFocus(void)
{
    if (selected_item_focus_image)
        return anytovariant(selected_item_focus_image->GetName());
    else
        return VARNULL;
}

int GUI_ListBox::pset_SelItemBackgroundFocus(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    if (image) {
        SetSelectedItemFocusImage(image);
    }
    return 0;
}

Variant GUI_ListBox::pget_SelItemBackgroundStyle(void)
{
    return anytovariant(styleIntToStr(selected_item_style));
}

int GUI_ListBox::pset_SelItemBackgroundStyle(Variant value)
{
    SetSelectedItemStyle(styleStrToInt(value));
    return 0;
}

Variant GUI_ListBox::pget_SelItemFontColor(void)
{
    return anytovariant(colorToStr(selitemtextcolor));
}

int GUI_ListBox::pset_SelItemFontColor(Variant value)
{
    SetSelectedItemTextColor(strToColor(value));
    return 0;
}

Variant GUI_ListBox::pget_SelItemFontColorFocus(void)
{
    return anytovariant(colorToStr(selitemtextcolorfocus));
}

int GUI_ListBox::pset_SelItemFontColorFocus(Variant value)
{
    SetSelectedItemTextColorFocus(strToColor(value));
    return 0;
}

Variant GUI_ListBox::pget_ScrollBarBackground(void)
{
    GUI_Surface *s = scrollbar->GetBackground();
    if (s) {
        return anytovariant(s->GetName());
    }
    return VARNULL;
}

int GUI_ListBox::pset_ScrollBarBackground(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    if (image) {
        SetScrollBarBackgroundImage(image);
    }
    return 0;
}

Variant GUI_ListBox::pget_ScrollBarBackgroundStyle(void)
{
    return anytovariant(styleIntToStr(scrollbar->GetBackgroundStyle()));
}

int GUI_ListBox::pset_ScrollBarBackgroundStyle(Variant value)
{
    SetScrollBarBackgroundStyle(styleStrToInt(value));
    return 0;
}

Variant GUI_ListBox::pget_ScrollBarKnob(void)
{
    GUI_Surface *s = scrollbar->GetKnobImage();
    if (s) {
        return anytovariant(s->GetName());
    }
    return VARNULL;
}

int GUI_ListBox::pset_ScrollBarKnob(Variant value)
{
    GUI_Surface *image = systemObject->LoadImage(value, true);
    if (image) {
        SetScrollBarKnobImage(image);
    }
    return 0;
}

Variant GUI_ListBox::pget_ScrollBarWidth(void)
{
    return anytovariant(scrollbar->GetWidth());
}

int GUI_ListBox::pset_ScrollBarWidth(Variant value)
{
    SetScrollBarWidth(value);
    return 0;
}

Variant GUI_ListBox::pget_ScrollBar(void)
{
    const char *sb = "auto";
    if (scrollbarmode == AUTOPROPERTY_ON) {
        sb = "on";
    }
    if (scrollbarmode == AUTOPROPERTY_OFF) {
        sb = "off";
    }
    return anytovariant(sb);
}

int GUI_ListBox::pset_ScrollBar(Variant value)
{
    int sb = AUTOPROPERTY_AUTO;
    if ((const char *) value == "on") {
        sb = AUTOPROPERTY_ON;
    }
    if ((const char *) value == "off") {
        sb = AUTOPROPERTY_OFF;
    }
    SetScrollBarMode(sb);
    return 0;
}

Variant GUI_ListBox::pget_SelItemBorderX(void)
{
    return anytovariant(selitemborderx);
}

int GUI_ListBox::pset_SelItemBorderX(Variant value)
{
    SetSelItemBorderX(value);
    return 0;
}

Variant GUI_ListBox::pget_SelItemBorderY(void)
{
    return anytovariant(selitembordery);
}

int GUI_ListBox::pset_SelItemBorderY(Variant value)
{
    SetSelItemBorderY(value);
    return 0;
}

Variant GUI_ListBox::pget_ItemBorderX(void)
{
    return anytovariant(itemborderx);
}

int GUI_ListBox::pset_ItemBorderX(Variant value)
{
    SetItemBorderX(value);
    return 0;
}

Variant GUI_ListBox::pget_ItemBorderY(void)
{
    return anytovariant(itembordery);
}

int GUI_ListBox::pset_ItemBorderY(Variant value)
{
    SetItemBorderY(value);
    return 0;
}

Variant GUI_ListBox::pget_ItemHeight(void)
{
    return anytovariant(itemheight);
}

int GUI_ListBox::pset_ItemHeight(Variant value)
{
    SetItemHeight(value);
    return 0;
}

Variant GUI_ListBox::pget_ItemList(void)
{
    string rc;
    for (int i = 0; i < listitems.size(); i++) {
        rc += listitems[i];
        rc += "|";
    }
    return anytovariant(rc.c_str());
}

Variant GUI_ListBox::pget_DataList(void)
{
    string rc;
    for (int i = 0; i < listitems.size(); i++) {
        rc += GetItemData(listitems[i]);
        rc += "|";
    }
    return anytovariant(rc.c_str());
}

Variant GUI_ListBox::m_AddItem(int numargs, Variant args[])
{
    AddItemData(args[0],0);
    return VARNULL;
}

Variant GUI_ListBox::m_AddItemData(int numargs, Variant args[])
{
    AddItemData(args[0], args[1]);
    return VARNULL;
}

Variant GUI_ListBox::m_RemoveItem(int numargs, Variant args[])
{
    RemoveItem(args[0]);
    return VARNULL;
}

Variant GUI_ListBox::m_RemoveLastItem(int numargs, Variant args[])
{
    RemoveItem(listitems.size() - 1);
    return VARNULL;
}

Variant GUI_ListBox::m_Clear(int numargs, Variant args[])
{
    Clear();
    return VARNULL;
}

Variant GUI_ListBox::m_Sort(int numargs, Variant args[])
{
    Sort();
    return VARNULL;
}

Variant GUI_ListBox::m_SetItemText(int numargs, Variant args[])
{
    SetItemText(args[0], args[1]);
    return VARNULL;
}

Variant GUI_ListBox::m_SetItemData(int numargs, Variant args[])
{
    SetItemData(args[0], args[1]);
    return VARNULL;
}

Variant GUI_ListBox::m_SearchItem(int numargs, Variant args[])
{
    if (listitems.size() <= 0) {
        return VARNULL;
    }
    const char *filter = args[0];
    for (int i = 0; i < listitems.size(); i++) {
        if (!strcmp(filter, listitems[i])) {
            return anytovariant(i);
        }
    }
    return VARNULL;
}

Variant GUI_ListBox::m_SearchItemData(int numargs, Variant args[])
{
    if (listitems.size() <= 0) {
        return VARNULL;
    }
    const char *filter = args[0];
    for (int i = 0; i < listitems.size(); i++) {
        if (!strcmp(filter, GetItemData(listitems[i]))) {
            return anytovariant(i);
        }
    }
    return VARNULL;
}

Variant GUI_ListBox::m_MatchItem(int numargs, Variant args[])
{
    if (listitems.size() <= 0) {
        return VARNULL;
    }
    const char *filter = args[0];
    for (int i = 0; i < listitems.size(); i++) {
        if (!matchString(listitems[i], filter)) {
            return anytovariant(i);
        }
    }
    return VARNULL;
}

Variant GUI_ListBox::m_MatchItemData(int numargs, Variant args[])
{
    if (listitems.size() <= 0) {
        return VARNULL;
    }
    const char *filter = args[0];
    for (int i = 0; i < listitems.size(); i++) {
        if (!matchString(GetItemData(listitems[i]), filter)) {
            return anytovariant(i);
        }
    }
    return VARNULL;
}

Variant GUI_ListBox::m_ItemText(int numargs, Variant args[])
{
    int index = args[0];
    if (listitems.size() > index) {
        return anytovariant(listitems[index]);
    }
    return VARNULL;
}

Variant GUI_ListBox::m_ItemData(int numargs, Variant args[])
{
    int index = args[0];
    if (listitems.size() > index) {
        char *data = GetItemData(listitems[index]);
        return anytovariant(data ? data : "");
    }
    return VARNULL;
}
