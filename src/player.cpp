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

PlayerObject::PlayerObject(const char *aname):GUI_Widget(aname, 0, 0, 0, 0)
{
    type = TYPE_PLAYER;
    vo_width = 0;
    vo_height = 0;
    vo_border = 0;
    window = None;
    display = 0;
    aspect = 1;
    audio_channel = -1;
    spu_channel = -1;
    osd_flag = 0;
    has_video = 0;
    has_audio = 0;
    playlist_shuffle = 0;
    playlist_repeat = 0;
    playlist_index = 0;
    seek_time = pos_time = end_time = 0;
    broadcast_port = 0;
    video_driver = 0;
    audio_driver = 0;
    visual_type = 0;
    volume = 0;
    amp_level = 0;
    playlist_title = 0;
    playlist_artist = 0;
    playlist_genre = 0;
    playlist_album = 0;
    playlist_comment = 0;

    MemberFunctionProperty < PlayerObject > *mp;
    mp = new MemberFunctionProperty < PlayerObject > ("playlist", this, &PlayerObject::pget_PlayList, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("playlistsize", this, &PlayerObject::pget_PlayListSize, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("index", this, &PlayerObject::pget_PlayListIdx, &PlayerObject::pset_PlayListIdx, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("comment", this, &PlayerObject::pget_Comment, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("genre", this, &PlayerObject::pget_Genre, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("album", this, &PlayerObject::pget_Album, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("artist", this, &PlayerObject::pget_Artist, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("title", this, &PlayerObject::pget_Title, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("file", this, &PlayerObject::pget_File, &PlayerObject::pset_File, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("shuffle", this, &PlayerObject::pget_Shuffle, &PlayerObject::pset_Shuffle, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("repeat", this, &PlayerObject::pget_Repeat, &PlayerObject::pset_Repeat, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("mute", this, &PlayerObject::pget_Mute, &PlayerObject::pset_Mute, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("ampmute", this, &PlayerObject::pget_AmpMute, &PlayerObject::pset_AmpMute, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("volume", this, &PlayerObject::pget_Volume, &PlayerObject::pset_Volume, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("amplevel", this, &PlayerObject::pget_AmpLevel, &PlayerObject::pset_AmpLevel, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("audiochannel", this, &PlayerObject::pget_AudioChannel, &PlayerObject::pset_AudioChannel, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("spuchannel", this, &PlayerObject::pget_SPU_Channel, &PlayerObject::pset_SPU_Channel, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("aspect", this, &PlayerObject::pget_Aspect, &PlayerObject::pset_Aspect, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("length", this, &PlayerObject::pget_Length, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("position", this, &PlayerObject::pget_TimePosition, &PlayerObject::pset_TimePosition, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("avoffset", this, &PlayerObject::pget_AV_Offset, &PlayerObject::pset_AV_Offset, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("spuoffset", this, &PlayerObject::pget_SPU_Offset, &PlayerObject::pset_SPU_Offset, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("speed", this, &PlayerObject::pget_Speed, &PlayerObject::pset_Speed, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("status", this, &PlayerObject::pget_Status, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("hue", this, &PlayerObject::pget_Hue, &PlayerObject::pset_Hue, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("contrast", this, &PlayerObject::pget_Contrast, &PlayerObject::pset_Contrast, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("saturation", this, &PlayerObject::pget_Saturation, &PlayerObject::pset_Saturation, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("brightness", this, &PlayerObject::pget_Brightness, &PlayerObject::pset_Brightness, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("windowborder", this, NULL, &PlayerObject::pset_WindowBorder, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("hasvideo", this, &PlayerObject::pget_HasVideo, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("hasaudio", this, &PlayerObject::pget_HasAudio, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("videowidth", this, &PlayerObject::pget_VideoWidth, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("videoheight", this, &PlayerObject::pget_VideoHeight, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("videoborder", this, &PlayerObject::pget_VideoBorder, &PlayerObject::pset_VideoBorder, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("deinterlacefilters", this, &PlayerObject::pget_DeinterlaceFilters, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("visualizationfilters", this, &PlayerObject::pget_VisualizationFilters, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("videofilter", this, &PlayerObject::pget_VideoFilter, &PlayerObject::pset_VideoFilter, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("audiofilter", this, &PlayerObject::pget_AudioFilter, &PlayerObject::pset_AudioFilter, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("info", this, &PlayerObject::pget_Info, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("videodriver", this, &PlayerObject::pget_VideoDriver, &PlayerObject::pset_VideoDriver, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("audiodriver", this, &PlayerObject::pget_AudioDriver, &PlayerObject::pset_AudioDriver, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < PlayerObject > ("broadcastport", this, &PlayerObject::pget_BroadcastPort, &PlayerObject::pset_BroadcastPort, false);
    AddProperty(mp);

    MemberMethodHandler < PlayerObject > *mh;
    mh = new MemberMethodHandler < PlayerObject > ("play", this, 0, &PlayerObject::m_Play);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("hide", this, 0, &PlayerObject::m_Hide);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("show", this, 0, &PlayerObject::m_Show);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("stop", this, 0, &PlayerObject::m_Stop);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("pause", this, 0, &PlayerObject::m_Pause);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("clear", this, 0, &PlayerObject::m_Clear);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("find", this, 1, &PlayerObject::m_Find);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("add", this, 1, &PlayerObject::m_Add);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("addfiles", this, 2, &PlayerObject::m_AddFiles);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("addskip", this, 2, &PlayerObject::m_AddSkip);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("prev", this, 0, &PlayerObject::m_Prev);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("next", this, 0, &PlayerObject::m_Next);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("sync", this, 0, &PlayerObject::m_Sync);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("seek", this, 1, &PlayerObject::m_Seek);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("position", this, 1, &PlayerObject::m_Position);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("osd", this, 1, &PlayerObject::m_Osd);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("osdtext", this, 3, &PlayerObject::m_OsdText);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("osdinfo", this, 1, &PlayerObject::m_OsdInfo);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("osdslider", this, 3, &PlayerObject::m_OsdSlider);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("event", this, 1, &PlayerObject::m_Event);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("info", this, 1, &PlayerObject::m_Info);
    AddMethod(mh);
    mh = new MemberMethodHandler < PlayerObject > ("snapshot", this, 1, &PlayerObject::m_Snapshot);
    AddMethod(mh);
}

PlayerObject::~PlayerObject(void)
{
    Stop();
    PlaylistClear();
    Destroy();
    InfoDestroy();
    lmbox_free(audio_driver);
    lmbox_free(video_driver);
}

Display *PlayerObject::GetDisplay()
{
    if (display) {
        return display;
    }
    return DynamicObject::GetDisplay();
}

Window PlayerObject::GetWindow()
{
    if (window != None) {
        return window;
    }
    return DynamicObject::GetWindow();
}

int PlayerObject::InfoInit(void)
{
    return 0;
}

void PlayerObject::InfoDestroy(void)
{
    Unlock();
}

void PlayerObject::SetPosition(int x, int y, int w, int h)
{
    if (!w) {
        w = systemObject->configGetInt("video_width", (int)(systemObject->GetScreenWidth() / systemObject->GetXScale()));
    }
    if (!h) {
        h = systemObject->configGetInt("video_height", (int)(systemObject->GetScreenHeight() / systemObject->GetYScale()));
    }
    GUI_Widget::SetPosition(x, y, w, h);
    if (w > 0 || h > 0) {
        OsdFree();
        OsdInit();
    }
    WindowSync();
}

int PlayerObject::WindowInit(void)
{
    if (window != None) {
        return 0;
    }
    if (!area.w) {
        area.w = systemObject->configGetInt("video_width", (int)(systemObject->GetScreenWidth() / systemObject->GetXScale()));
    }
    if (!area.h) {
        area.h = systemObject->configGetInt("video_height", (int)(systemObject->GetScreenHeight() / systemObject->GetYScale()));
    }
    if (!display) {
        display = XOpenDisplay(systemObject->configGetValue("display", 0));
    }
    if (!display) {
        systemObject->Log(0, "Error opening X11 display");
        return -1;
    }
    int depth;
    Visual *visual = systemObject->FindVisual(display, &depth);
    window = systemObject->CreateWindow(display, visual, depth, area.x, area.y, area.w, area.h, vo_border);
    if (window == None) {
        systemObject->Log(0, "PlayerInit: unable to create window %dx%dx%dx%d", area.x, area.y, area.w, area.h);
        return -1;
    }
    return 0;
}

int PlayerObject::WindowShow(void)
{
    DisplayLock();
    if (window != None && has_video) {
        XSetWindowBorder(display, window, vo_border);
        XMoveResizeWindow(display, window, area.x, area.y, area.w, area.h);
        XMapRaised(display, window);
        while (!systemObject->IsWindowViewable(display, window)) {
            SDL_Delay(100);
        }
        XSync(display, False);
    }
    DisplayUnlock();
    return 0;
}

int PlayerObject::WindowHide(void)
{
    DisplayLock();
    if (window != None && has_video) {
        XUnmapWindow(display, window);
        XSync(display, False);
    }
    DisplayUnlock();
    return 0;
}

int PlayerObject::WindowDestroy(void)
{
    DisplayLock();
    if (window != None) {
        XDestroyWindow(display, window);
        window = None;
    }
    DisplayUnlock();
    if (display) {
        XCloseDisplay(display);
        display = 0;
    }
    return 0;
}

int PlayerObject::WindowSync(void)
{
    DisplayLock();
    if (window != None && has_video) {
        XMoveResizeWindow(display, window, area.x, area.y, area.w, area.h);
        XSync(display, False);
    }
    DisplayUnlock();
    OnExpose();
    return 0;
}

int PlayerObject::IsPlaying(void)
{
    return 0;
}

int PlayerObject::PlayerInit(void)
{
    if (WindowInit()) {
        return -1;
    }
    // Reset video flag so on play we will find out about existence of video in the stream
    has_video = has_audio = 0;
    vo_width = vo_height = 0;
    OnResize();
    return 0;
}

int PlayerObject::Play(void)
{
    WindowShow();
    PlayerExpose();
    return 0;
}

void PlayerObject::Pause(void)
{
}

void PlayerObject::Stop(void)
{
    audio_channel = -1;
    spu_channel = -1;
    WindowHide();
}

void PlayerObject::Destroy(void)
{
}

void PlayerObject::SetStartPosition(int val)
{
    if (!TryLock()) {
        pos_time = val;
        Unlock();
    }
}

int PlayerObject::GetTimePosition(int *len)
{
    return 0;
}

int PlayerObject::Seek(int offset, bool absolute)
{
    return 0;
}

void PlayerObject::FileAdd(const char *path, const char *pattern, time_t start, time_t duration, const char *skip)
{
    DIR *dir;
    FILE *file;
    string name;
    char line[256];
    struct stat st;
    struct dirent *ent;

    if (stat(path, &st)) {
        // mp3 playlist
        if (!matchString(path, "http://*.m3u")) {
            char *data;
            vector<char*> urls;
            int size, status = sock_geturl(path, 30, 0, &data, &size, 0);
            if (status == 200 && size > 0) {
                strSplit(&urls, data, "\r\n");
                for (int i = 0;i < urls.size(); i++) {
                    if (urls[i] != "") {
                        playlist_files.push_back(new FileEntry(urls[i], 0, 0, NULL));
                        urls[i] = 0;
                    }
                }
                while (urls.size() > 0) {
                    lmbox_free(urls.back());
                    urls.pop_back();
                }
            }
            lmbox_free(data);
            return;
        } else
        // Playlist
        if (!matchString(path, "http://*.pls")) {
            char *data;
            vector<char*> urls;
            int size, status = sock_geturl(path, 30, 0, &data, &size, 0);
            if (status == 200 && size > 0) {
                strSplit(&urls, data, "\r\n");
                for (int i = 0;i < urls.size(); i++) {
                    if (!strncasecmp(urls[i], "File", 4)) {
                        char *url = strchr(urls[i], '=');
                        if (url) {
                            url = trimString(++url);
                            playlist_files.push_back(new FileEntry(url, 0, 0, NULL));
                            free(url);
                        }
                    }
                }
                while (urls.size() > 0) {
                    lmbox_free(urls.back());
                    urls.pop_back();
                }
            }
            lmbox_free(data);
            return;
        }
        playlist_files.push_back(new FileEntry(path, start, duration, NULL));
        return;
    }
    if (!S_ISDIR(st.st_mode)) {
        // mp3 playlist
        if (!matchString(path, "*.m3u")) {
            if (!(file = fopen(path, "r"))) {
                return;
            }
            while (!feof(file)) {
                if (!fgets(line, sizeof(line), file) || line[0] == '#') {
                    continue;
                }
                char *url = trimString(line);
                playlist_files.push_back(new FileEntry(url, 0, 0, NULL));
                free(url);
            }
            fclose(file);
            return;
        } else
        // Playlist
        if (!matchString(path, "*.pls")) {
            if (!(file = fopen(path, "r"))) {
                return;
            }
            while (!feof(file)) {
                if (!fgets(line, sizeof(line), file) || line[0] == '#') {
                    continue;
                }
                if (!strncasecmp(line, "File", 4)) {
                    char *url = strchr(line, '=');
                    if (url) {
                        url = trimString(++url);
                        playlist_files.push_back(new FileEntry(url, 0, 0, NULL));
                        free(url);
                    }
                }
            }
            fclose(file);
            return;
        }
        // Single regular file
        playlist_files.push_back(new FileEntry(path, 0, 0, skip));
        return;
    }
    if ((dir = opendir(path)) == NULL) {
        return;
    }
    regex_t *reg = pattern ? parseRegex(pattern) : 0;
    // If we have directory here, tell player to play all files
    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.') {
            continue;
        }
        name = path;
        name += "/";
        name += ent->d_name;
        if (stat(name.c_str(), &st)) {
            continue;
        }
        // File name matching
        if (!S_ISDIR(st.st_mode) && reg && matchRegex(ent->d_name, reg)) {
            continue;
        }
        FileAdd(name.c_str(), pattern, 0, 0, NULL);
    }
    closedir(dir);
    freeRegex(reg);
}

void PlayerObject::PlaylistClear(void)
{
    while (playlist_files.size() > 0) {
        delete playlist_files.back();
        playlist_files.pop_back();
    }
    lmbox_destroy((void**)&playlist_title);
    lmbox_destroy((void**)&playlist_artist);
    lmbox_destroy((void**)&playlist_genre);
    lmbox_destroy((void**)&playlist_album);
    lmbox_destroy((void**)&playlist_comment);
    playlist_index = 0;
    pos_time = end_time = 0;
}

void PlayerObject::PlayerExpose(void)
{
}

int PlayerObject::PlaylistNext(void)
{
    if (TryLock()) {
        return -1;
    }
    PlaylistIndex(1);
    int rc = Play();
    Unlock();
    return rc;
}

int PlayerObject::PlaylistPrev(void)
{
    if (TryLock()) {
        return -1;
    }
    PlaylistIndex(-1);
    int rc = Play();
    Unlock();
    return rc;
}

char *PlayerObject::PlayerEvent(const char *action)
{
    return tlsStr(0, "");
}

void PlayerObject::PlaylistIndex(int step)
{
    if (playlist_files.size() > 1) {
        playlist_index += step;
        if (playlist_index >= playlist_files.size()) {
            playlist_index = 0;
        }
        if (playlist_index < 0) {
            playlist_index = playlist_files.size() - 1;
        }
        pos_time = end_time = 0;
    }
}

int PlayerObject::FileFind(const char *file)
{
    if (TryLock()) {
        return -1;
    }
    int rc = -1;
    for (int i = 0; i < playlist_files.size(); i++) {
        if (!strcmp(file, playlist_files[i]->name())) {
            rc = i;
            break;
        }
    }
    Unlock();
    return rc;
}

char *PlayerObject::FileInfo(const char *mrl)
{
    return tlsStr(0, "");
}

int PlayerObject::Snapshot(const char *filename)
{
    return 0;
}

void PlayerObject::OsdInit(void)
{
}

void PlayerObject::OsdFree(void)
{
}

int PlayerObject::OsdSet(const char *cmd)
{
    return 0;
}

void PlayerObject::OsdText(int x, int y, const char *text)
{
}

void PlayerObject::OsdSlider(int val, int max, const char *str)
{
}

void PlayerObject::OsdInfo(const char *text)
{
}

void PlayerObject::DisplayLock(void)
{
    if (display) {
        XLockDisplay(display);
    }
}

void PlayerObject::DisplayUnlock(void)
{
    if (display) {
        XUnlockDisplay(display);
    }
}

void PlayerObject::SetVideoDriver(const char *dname)
{
    if (TryLock()) {
        return;
    }
    lmbox_free(video_driver);
    video_driver = dname && *dname ? lmbox_strdup(dname) : 0;
    Unlock();
}

void PlayerObject::SetAudioDriver(const char *dname)
{
    if (TryLock()) {
        return;
    }
    lmbox_free(audio_driver);
    audio_driver = dname && *dname ? lmbox_strdup(dname) : 0;
    Unlock();
}

int PlayerObject::GetVideoBorder(void)
{
    return vo_border;
}

void PlayerObject::SetVideoBorder(int pixels)
{
    this->vo_border = pixels;
}

void PlayerObject::OnEvent(void)
{
}

void PlayerObject::OnExpose()
{
    PlayerExpose();
}

void PlayerObject::OnResize(void)
{
    // Adjust video aspect
    double video_aspect = systemObject->configGetDouble("video_aspect", -1);
    if (video_aspect > 0) {
        aspect = video_aspect;
    } else
    if (video_aspect == 0) {
        aspect = area.w * 1.0 / area.h * 1.0;
        if (fabs(aspect - 1.0) < 0.01) {
            aspect = 1.0;
        }
    }
}

bool PlayerObject::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    return 0;
}

void PlayerObject::SetWindowBorder(int border)
{
    if (display && window != None) {
        systemObject->SetWindowBorder(display, window, border);
    }
}

SDL_Rect PlayerObject::GetAreaRaw(void)
{
    return area;
}

double PlayerObject::GetAspect(void)
{
    return aspect;
}

void PlayerObject::SetAspect(double value)
{
    if (TryLock()) {
        return;
    }
    aspect = value;
    if (fabs(aspect - 1.0) < 0.01) {
        aspect = 1.0;
    }
    Unlock();
}

int PlayerObject::GetPlaylistSize(void)
{
    if (TryLock()) {
        return -1;
    }
    int rc = playlist_files.size();
    Unlock();
    return rc;
}

int PlayerObject::GetPlaylistIndex(void)
{
    if (TryLock()) {
        return -1;
    }
    int rc = playlist_index;
    Unlock();
    return rc;
}

char *PlayerObject::GetPlaylist(void)
{
    char *rc = tlsStr(0, "");
    if (TryLock()) {
        return rc;
    }
    for (int i = 0; i < playlist_files.size(); i++) {
        if (i) {
            rc = tlsStrAppend(0, " ");
        }
        rc = tlsStrAppend(0, "{");
        rc = tlsStrAppend(0, playlist_files[i]->name());
        rc = tlsStrAppend(0, "}");
    }
    Unlock();
    return rc;
}

void PlayerObject::SetPlaylistIndex(int idx)
{
    if (TryLock()) {
        return;
    }
    if (idx >= 0 && idx < playlist_files.size()) {
        playlist_index = idx;
    }
    Unlock();
}

char *PlayerObject::GetPlaylistFile(void)
{
    char *file = tlsStr(0, "");
    if (TryLock()) {
        return file;
    }
    if (playlist_files.size() > 0 && playlist_index >= 0 && playlist_index < playlist_files.size()) {
        file = tlsStr(0, playlist_files[playlist_index]->name());
    }
    Unlock();
    return file;
}

void PlayerObject::SetPlaylistFile(const char *value)
{
    if (TryLock()) {
        return;
    }
    for (int i = 0;i < playlist_files.size(); i++) {
        if (!strcmp(value, playlist_files[i]->name())) {
            playlist_index = i;
            break;
        }
    }
    Unlock();
}

char *PlayerObject::GetPlaylistTitle(void)
{
    char *rc = "";
    if (TryLock()) {
        return rc;
    }
    if (playlist_title) {
        rc = tlsStr(0, playlist_title);
    } else
    if (playlist_files.size() > 0 && playlist_index < playlist_files.size()) {
        rc = tlsStr(0, playlist_files[playlist_index]->name());
    }
    Unlock();
    return rc;
}

void *PlayerObject::SetPlaylistTitle(const char *value)
{
    if (TryLock()) {
        return 0;
    }
    playlist_title = lmbox_strdup(value);
    Unlock();
}

int PlayerObject::GetPlaylistShuffle(void)
{
    if (TryLock()) {
        return -1;
    }
    int rc = playlist_shuffle;
    Unlock();
    return rc;
}

void PlayerObject::SetPlaylistShuffle(int value)
{
    if (TryLock()) {
        return;
    }
    playlist_shuffle = value;
    if (playlist_shuffle) {
        for (int i = 0; i < playlist_files.size(); i++) {
            int j = (int) ((playlist_files.size() - 1) * (rand() / (RAND_MAX + 1.0)));
            if (i == j) {
                continue;
            }
            FileEntry *file = playlist_files[j];
            playlist_files[j] = playlist_files[i];
            playlist_files[i] = file;
        }
    }
    Unlock();
}

int PlayerObject::GetPlaylistRepeat(void)
{
    if (TryLock()) {
        return -1;
    }
    int rc = playlist_repeat;
    Unlock();
    return rc;
}

void PlayerObject::SetPlaylistRepeat(int val)
{
    if (TryLock()) {
        return;
    }
    playlist_repeat = val;
    Unlock();
}

char *PlayerObject::GetPlaylistAlbum(void)
{
    if (TryLock()) {
        return "";
    }
    char *rc = tlsStr(0, playlist_album);
    Unlock();
    return rc;
}

void *PlayerObject::SetPlaylistAlbum(const char *value)
{
    if (TryLock()) {
        return 0;
    }
    playlist_album = lmbox_strdup(value);
    Unlock();
}

char *PlayerObject::GetPlaylistArtist(void)
{
    if (TryLock()) {
        return "";
    }
    char *rc = tlsStr(0, playlist_artist);
    Unlock();
    return rc;
}

void *PlayerObject::SetPlaylistArtist(const char *value)
{
    if (TryLock()) {
        return 0;
    }
    playlist_artist = lmbox_strdup(value);
    Unlock();
}

char *PlayerObject::GetPlaylistGenre(void)
{
    if (TryLock()) {
        return "";
    }
    char *rc = tlsStr(0, playlist_genre);
    Unlock();
    return rc;
}

void *PlayerObject::SetPlaylistGenre(const char *value)
{
    if (TryLock()) {
        return 0;
    }
    playlist_genre = lmbox_strdup(value);
    Unlock();
}

char *PlayerObject::GetPlaylistComment(void)
{
    if (TryLock()) {
        return "";
    }
    char *rc = tlsStr(0, playlist_comment);
    Unlock();
    return rc;
}

void *PlayerObject::SetPlaylistComment(const char *value)
{
    if (TryLock()) {
        return 0;
    }
    playlist_comment = lmbox_strdup(value);
    Unlock();
}

char *PlayerObject::GetVideoDriver(void)
{
    if (TryLock()) {
        return "";
    }
    char *rc = tlsStr(0, video_driver);
    Unlock();
    return rc;
}

char *PlayerObject::GetAudioDriver(void)
{
    if (TryLock()) {
        return "";
    }
    char *rc = tlsStr(0, audio_driver);
    Unlock();
    return rc;
}

int PlayerObject::GetAVOffset(void)
{
    return 0;
}

void PlayerObject::SetAVOffset(int val)
{
}

int PlayerObject::GetSPUOffset(void)
{
    return 0;
}

void PlayerObject::SetSPUOffset(int val)
{
}

int PlayerObject::GetVolume(void)
{
    return 0;
}

void PlayerObject::SetVolume(int val)
{
    volume = val;
}

int PlayerObject::GetMute(void)
{
    return 0;
}

void PlayerObject::SetMute(int val)
{
}

int PlayerObject::GetAmpMute(void)
{
    return 0;
}

void PlayerObject::SetAmpMute(int val)
{
}

int PlayerObject::GetAmpLevel(void)
{
    return 0;
}

void PlayerObject::SetAmpLevel(int val)
{
    amp_level = val;
}

int PlayerObject::GetHue(void)
{
    return 0;
}

void PlayerObject::SetHue(int val)
{
}

int PlayerObject::GetSaturation(void)
{
    return 0;
}

void PlayerObject::SetSaturation(int val)
{
}

int PlayerObject::GetBrightness(void)
{
    return 0;
}

void PlayerObject::SetBrightness(int val)
{
}

int PlayerObject::GetContrast(void)
{
    return 0;
}

void PlayerObject::SetContrast(int val)
{
}

int PlayerObject::GetAudioChannel(void)
{
    return audio_channel;
}

void PlayerObject::SetAudioChannel(int val)
{
    audio_channel = val;
}

int PlayerObject::GetSPUChannel(void)
{
    return spu_channel;
}

void PlayerObject::SetSPUChannel(int val)
{
    spu_channel = val;
}

int PlayerObject::GetSpeed(void)
{
    return 0;
}

void PlayerObject::SetSpeed(char *value)
{
}

int PlayerObject::GetLength(void)
{
    return 0;
}

char *PlayerObject::GetDeinterlaceFilters(void)
{
    return tlsStr(0, "");
}

char *PlayerObject::GetVideoFilter(void)
{
    return tlsStr(0, "");
}

void PlayerObject::SetVideoFilter(const char *value)
{
}

char *PlayerObject::GetAudioFilter(void)
{
    return tlsStr(0, "");
}

void PlayerObject::SetAudioFilter(const char *value)
{
}

int PlayerObject::GetBroadcastPort(void)
{
    if (TryLock()) {
        return -1;
    }
    int rc = broadcast_port;
    Unlock();
    return rc;
}

void PlayerObject::SetBroadcastPort(int value)
{
    if (TryLock()) {
        return;
    }
    broadcast_port = value;
    Unlock();
}

int PlayerObject::HasVideo(void)
{
    if (TryLock()) {
        return 0;
    }
    int rc = has_video;
    Unlock();
    return rc;
}

int PlayerObject::HasAudio(void)
{
    if (TryLock()) {
        return 0;
    }
    int rc = has_audio;
    Unlock();
    return rc;
}

char *PlayerObject::GetVisualizationFilters(void)
{
    return tlsStr(0, "");
}

const char *PlayerObject::GetStatus(void)
{
    return "idle";
}

int PlayerObject::GetVideoWidth(void)
{
    int val = vo_width ? vo_width : area.w;
    return (int)(val * systemObject->GetXScale());
}

int PlayerObject::GetVideoHeight(void)
{
    int val = vo_height ? vo_height : area.h;
    return (int)(val * systemObject->GetYScale());
}

void PlayerObject::GetAutoPlay(const char *mrl)
{
}

Variant PlayerObject::pget_PlayList(void)
{
    return anytovariant(GetPlaylist());
}

Variant PlayerObject::pget_PlayListIdx(void)
{
    return anytovariant(GetPlaylistIndex());
}

Variant PlayerObject::pget_PlayListSize(void)
{
    return anytovariant(GetPlaylistSize());
}

int PlayerObject::pset_PlayListIdx(Variant value)
{
    SetPlaylistIndex(value);
    return 0;
}

Variant PlayerObject::pget_Repeat(void)
{
    return anytovariant(GetPlaylistRepeat());
}

int PlayerObject::pset_Repeat(Variant value)
{
    SetPlaylistRepeat(value);
    return 0;
}

Variant PlayerObject::pget_Title(void)
{
    return anytovariant(GetPlaylistTitle());
}

Variant PlayerObject::pget_Album(void)
{
    return anytovariant(GetPlaylistAlbum());
}

Variant PlayerObject::pget_Artist(void)
{
    return anytovariant(GetPlaylistArtist());
}

Variant PlayerObject::pget_Genre(void)
{
    return anytovariant(GetPlaylistGenre());
}

Variant PlayerObject::pget_Comment(void)
{
    return anytovariant(GetPlaylistComment());
}

Variant PlayerObject::pget_VideoDriver(void)
{
    return anytovariant(GetVideoDriver());
}

int PlayerObject::pset_VideoDriver(Variant value)
{
    SetVideoDriver(value);
    return 0;
}

Variant PlayerObject::pget_AudioDriver(void)
{
    return anytovariant(GetAudioDriver());
}

int PlayerObject::pset_AudioDriver(Variant value)
{
    SetAudioDriver(value);
    return 0;
}

Variant PlayerObject::pget_Shuffle(void)
{
    return anytovariant(GetPlaylistShuffle());
}

int PlayerObject::pset_Shuffle(Variant value)
{
    SetPlaylistShuffle(value);
    return 0;
}

Variant PlayerObject::pget_VideoBorder(void)
{
    return anytovariant(GetVideoBorder());
}

int PlayerObject::pset_VideoBorder(Variant value)
{
    SetVideoBorder(value);
    return 0;
}

Variant PlayerObject::pget_Aspect(void)
{
    return anytovariant(GetAspect());
}

int PlayerObject::pset_Aspect(Variant value)
{
    SetAspect(value);
    return 0;
}

Variant PlayerObject::pget_AV_Offset(void)
{
    return anytovariant(GetAVOffset());
}

int PlayerObject::pset_AV_Offset(Variant value)
{
    SetAVOffset(value);
    return 0;
}

Variant PlayerObject::pget_SPU_Offset(void)
{
    return anytovariant(GetSPUOffset());
}

int PlayerObject::pset_SPU_Offset(Variant value)
{
    SetSPUOffset(value);
    return 0;
}

Variant PlayerObject::pget_Volume(void)
{
    return anytovariant(GetVolume());
}

int PlayerObject::pset_Volume(Variant value)
{
    SetVolume(value);
    return 0;
}

Variant PlayerObject::pget_Mute(void)
{
    return anytovariant(GetMute());
}

int PlayerObject::pset_Mute(Variant value)
{
    SetMute(value);
    return 0;
}

Variant PlayerObject::pget_AmpMute(void)
{
    return anytovariant(GetAmpMute());
}

int PlayerObject::pset_AmpMute(Variant value)
{
    SetAmpMute(value);
    return 0;
}

Variant PlayerObject::pget_AmpLevel(void)
{
    return anytovariant(GetAmpLevel());
}

int PlayerObject::pset_AmpLevel(Variant value)
{
    SetAmpLevel(value);
    return 0;
}

Variant PlayerObject::pget_Hue(void)
{
    return anytovariant(GetHue());
}

int PlayerObject::pset_Hue(Variant value)
{
    SetHue(value);
    return 0;
}

Variant PlayerObject::pget_Saturation(void)
{
    return anytovariant(GetSaturation());
}

int PlayerObject::pset_Saturation(Variant value)
{
    SetSaturation(value);
    return 0;
}

Variant PlayerObject::pget_Brightness(void)
{
    return anytovariant(GetBrightness());
}

int PlayerObject::pset_Brightness(Variant value)
{
    SetBrightness(value);
    return 0;
}

Variant PlayerObject::pget_Contrast(void)
{
    return anytovariant(GetContrast());
}

int PlayerObject::pset_Contrast(Variant value)
{
    SetContrast(value);
    return 0;
}

Variant PlayerObject::pget_AudioChannel(void)
{
    return anytovariant(GetAudioChannel());
}

int PlayerObject::pset_AudioChannel(Variant value)
{
    SetAudioChannel(value);
    return 0;
}

Variant PlayerObject::pget_SPU_Channel(void)
{
    return anytovariant(GetSPUChannel());
}

int PlayerObject::pset_SPU_Channel(Variant value)
{
    SetSPUChannel(value);
    return 0;
}

Variant PlayerObject::pget_Speed(void)
{
    return anytovariant(GetSpeed());
}

int PlayerObject::pset_Speed(Variant value)
{
    SetSpeed(value);
    return 0;
}

Variant PlayerObject::pget_File(void)
{
    return anytovariant(GetPlaylistFile());
}

int PlayerObject::pset_File(Variant value)
{
    SetPlaylistFile(value);
    return 0;
}

Variant PlayerObject::pget_Length(void)
{
    return anytovariant(GetLength());
}

Variant PlayerObject::pget_TimePosition(void)
{
    return anytovariant(GetTimePosition(0));
}

int PlayerObject::pset_TimePosition(Variant value)
{
    SetStartPosition(value);
    return 0;
}

int PlayerObject::pset_WindowBorder(Variant value)
{
    SetWindowBorder((int)value);
    return 0;
}

Variant PlayerObject::pget_DeinterlaceFilters(void)
{
    return anytovariant(GetDeinterlaceFilters());
}

Variant PlayerObject::pget_VideoFilter(void)
{
    return anytovariant(GetVideoFilter());
}

int PlayerObject::pset_VideoFilter(Variant value)
{
    SetVideoFilter(value);
    return 0;
}

Variant PlayerObject::pget_AudioFilter(void)
{
    return anytovariant(GetAudioFilter());
}

int PlayerObject::pset_AudioFilter(Variant value)
{
    SetAudioFilter(value);
    return 0;
}

Variant PlayerObject::pget_BroadcastPort(void)
{
    return anytovariant(GetBroadcastPort());
}

int PlayerObject::pset_BroadcastPort(Variant value)
{
    SetBroadcastPort(value);
    return 0;
}

Variant PlayerObject::pget_HasVideo(void)
{
    return anytovariant(HasVideo());
}

Variant PlayerObject::pget_HasAudio(void)
{
    return anytovariant(HasAudio());
}

Variant PlayerObject::pget_VisualizationFilters(void)
{
    return anytovariant(GetVisualizationFilters());
}

Variant PlayerObject::pget_Status(void)
{
    return anytovariant(GetStatus());
}

Variant PlayerObject::pget_VideoWidth(void)
{
    return anytovariant(GetVideoWidth());
}

Variant PlayerObject::pget_VideoHeight(void)
{
    return anytovariant(GetVideoHeight());
}

Variant PlayerObject::pget_Info(void)
{
    return anytovariant(FileInfo(0));
}

Variant PlayerObject::m_Play(int numargs, Variant args[])
{
    Stop();
    if (TryLock()) {
        return VARNULL;
    }
    int rc = PlayerInit();
    if (!rc) {
        rc = Play();
    }
    Unlock();
    if (!rc && IsPlaying()) {
        if (FireEvent("OnStart")) {
            // Custom onetime callback
            if (ongotfocuscb) {
                Script script(name, ongotfocuscb);
                script.Execute();
                SetOnGotFocusCb(0);
            }
        }
    }
    return anytovariant(rc);
}

Variant PlayerObject::m_Stop(int numargs, Variant args[])
{
    int was_playing = IsPlaying();
    Stop();
    if (was_playing && FireEvent("OnStop")) {
        // Custom onetime callback
        if (onlostfocuscb) {
            Script script(name, onlostfocuscb);
            script.Execute();
            SetOnLostFocusCb(0);
        }
    }
    return VARNULL;
}

Variant PlayerObject::m_Pause(int numargs, Variant args[])
{
    Pause();
    return VARNULL;
}

Variant PlayerObject::m_Hide(int numargs, Variant args[])
{
    return anytovariant(WindowHide());
}

Variant PlayerObject::m_Show(int numargs, Variant args[])
{
    return anytovariant(WindowShow());
}

Variant PlayerObject::m_Next(int numargs, Variant args[])
{
    return anytovariant(PlaylistNext());
}

Variant PlayerObject::m_Prev(int numargs, Variant args[])
{
    return anytovariant(PlaylistPrev());
}

Variant PlayerObject::m_Clear(int numargs, Variant args[])
{
    if (TryLock()) {
        return VARNULL;
    }
    PlaylistClear();
    Unlock();
    return VARNULL;
}

Variant PlayerObject::m_Add(int numargs, Variant args[])
{
    SetStartPosition(0);
    if (TryLock()) {
        return VARNULL;
    }
    FileAdd(args[0], 0, 0, 0, NULL);
    Unlock();
    return VARNULL;
}

Variant PlayerObject::m_AddFiles(int numargs, Variant args[])
{
    SetStartPosition(0);
    if (TryLock()) {
        return VARNULL;
    }
    FileAdd(args[0], args[1], 0, 0, NULL);
    Unlock();
    return VARNULL;
}

Variant PlayerObject::m_AddSkip(int numargs, Variant args[])
{
    SetStartPosition(0);
    if (TryLock()) {
        return VARNULL;
    }
    FileAdd(args[0], 0, 0, 0, args[1]);
    Unlock();
    return VARNULL;
}

Variant PlayerObject::m_Sync(int numargs, Variant args[])
{
    WindowSync();
    return VARNULL;
}

Variant PlayerObject::m_Seek(int numargs, Variant args[])
{
    return anytovariant(Seek((int) args[0], false));
}

Variant PlayerObject::m_Position(int numargs, Variant args[])
{
    return anytovariant(Seek((int) args[0], true));
}

Variant PlayerObject::m_Osd(int numargs, Variant args[])
{
    return anytovariant(OsdSet(args[0]));
}

Variant PlayerObject::m_OsdText(int numargs, Variant args[])
{
    OsdText(args[0], args[1], args[2]);
    return VARNULL;
}

Variant PlayerObject::m_OsdSlider(int numargs, Variant args[])
{
    OsdSlider(args[0], args[1], args[2]);
    return VARNULL;
}

Variant PlayerObject::m_OsdInfo(int numargs, Variant args[])
{
    OsdInfo(args[0]);
    return VARNULL;
}

Variant PlayerObject::m_Info(int numargs, Variant args[])
{
    return anytovariant(FileInfo(args[0]));
}

Variant PlayerObject::m_Event(int numargs, Variant args[])
{
    return anytovariant(PlayerEvent(args[0]));
}

Variant PlayerObject::m_Snapshot(int numargs, Variant args[])
{
    Snapshot(args[0]);
    return VARNULL;
}

Variant PlayerObject::m_Find(int numargs, Variant args[])
{
    return anytovariant(FileFind(args[0]));
}

// Try to get autoplay list from the plugin
Variant PlayerObject::m_AutoPlay(int numargs, Variant args[])
{
    GetAutoPlay(args[0]);
    return VARNULL;
}

