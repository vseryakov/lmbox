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

GUI_FileBrowser::GUI_FileBrowser(const char *name, int x, int y, int w, int h, GUI_Font * font):GUI_ListBox(name, x, y, w, h, font)
{
    this->lastpath = "";
    this->rootpath = "/";
    this->curpath = "/";
    this->type = TYPE_FILEBROWSER;

    MemberFunctionProperty < GUI_FileBrowser > *mp;
    mp = new MemberFunctionProperty < GUI_FileBrowser > ("rootpath", this, &GUI_FileBrowser::pget_RootPath, &GUI_FileBrowser::pset_RootPath, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_FileBrowser > ("curpath", this, &GUI_FileBrowser::pget_CurPath, &GUI_FileBrowser::pset_RootPath, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < GUI_FileBrowser > ("filename", this, &GUI_FileBrowser::pget_FileName, NULL, false);
    AddProperty(mp);

    MemberMethodHandler < GUI_FileBrowser > *mh;
    mh = new MemberMethodHandler < GUI_FileBrowser > ("refresh", this, 0, &GUI_FileBrowser::m_Refresh);
    AddMethod(mh);
    mh = new MemberMethodHandler < GUI_FileBrowser > ("choose", this, 1, &GUI_FileBrowser::m_Choose);
    AddMethod(mh);

    DoRefresh();
}

GUI_FileBrowser::~GUI_FileBrowser(void)
{
}

void GUI_FileBrowser::DoRefresh()
{
    DIR *dp;
    struct stat statbuf;
    string fullname, item;
    struct dirent *entry, **files = 0;

    Clear();
    if (curpath == "" || curpath.rfind("/") != curpath.length() - 1) {
        curpath += "/";
    }
    int count = scandir(curpath.c_str(), &files, NULL, (int (*)(const void *, const void *)) sortcmp);
    for (int i = 0; i < count; ++i) {
        if (!strcmp(files[i]->d_name, ".")) {
            free(files[i]);
            continue;
        }
        if (!strcmp(files[i]->d_name, "..") && curpath == rootpath) {
            free(files[i]);
            continue;
        }
        /* Build full name of file */
        fullname = curpath;
        fullname += files[i]->d_name;
        /* Get info on file entry */
        if (lstat(fullname.c_str(), &statbuf)) {
            free(files[i]);
            continue;
        }
        item = files[i]->d_name;
        if (S_ISDIR(statbuf.st_mode)) {
            item += "/";
        }
        AddItemData(item.c_str(), 0);
        if (lastpath == item) {
            SetSelectedIndex(listitems.size() - 1);
        }
        free(files[i]);
    }
    lmbox_free(files);
}

void GUI_FileBrowser::ChooseSelection(void)
{
    if (!listitems.size()) {
        return;
    }
    string selectedtext = listitems[selectedindex];
    // Regular file
    if (selectedtext.length() > 0 && selectedtext.rfind("/") != selectedtext.length() - 1) {
        GUI_ListBox::ChooseSelection();
        return;
    }
    // Directory
    string oldpath = curpath;
    if (selectedtext == "../") {
        curpath.resize(curpath.length() - 1);
        int pos = curpath.rfind("/");
        if (pos >= 0) {
            curpath = curpath.substr(0, pos);
        }
    } else {
        lastpath = selectedtext;
        curpath += selectedtext;
    }
    DoRefresh();
    if (listitems.size() > 0) {
        FireEvent("OnPathChange");
        return;
    }
    // Something wrong, might be permissions
    curpath = oldpath;
    DoRefresh();
}

Variant GUI_FileBrowser::pget_RootPath(void)
{
    return anytovariant(rootpath.c_str());
}

Variant GUI_FileBrowser::pget_CurPath(void)
{
    return anytovariant(curpath.c_str());
}

int GUI_FileBrowser::pset_RootPath(Variant value)
{
    rootpath = (char*)value;
    if (rootpath == "" || rootpath.rfind("/") != rootpath.length() - 1) {
        rootpath += "/";
    }
    curpath = rootpath;
    DoRefresh();
    return 0;
}

Variant GUI_FileBrowser::m_Refresh(int numargs, Variant args[])
{
    DoRefresh();
    return VARNULL;
}

Variant GUI_FileBrowser::m_Choose(int numargs, Variant args[])
{
    SetSelectedIndex(args[0]);
    ChooseSelection();
    return VARNULL;
}

Variant GUI_FileBrowser::pget_FileName(void)
{
    if (listitems.size() == 0) {
        return VARNULL;
    }
    string filename = curpath;
    filename += listitems[selectedindex];
    return anytovariant(filename.c_str());
}
