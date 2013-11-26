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

#ifdef HAVE_VLC

VLCPlayerObject::VLCPlayerObject(const char *aname):PlayerObject(aname)
{
    vlc = VLC_Create();
    VLC_Init(vlc, 0, 0);
}

VLCPlayerObject::~VLCPlayerObject(void)
{
    VLC_CleanUp(vlc);
    VLC_Destroy(vlc);
}

int VLCPlayerObject::PlayerInit(void)
{
    if (PlayerObject::PlayerInit()) {
        return -1;
    }
    vlc_value_t val;
    val.i_int = (int)window;
    VLC_VariableSet(vlc, "drawable", val);
    val.i_int = area.w;
    VLC_VariableSet(vlc, "drawablew", val);
    val.i_int = area.h;
    VLC_VariableSet(vlc, "drawableh", val);
    return 0;
}

int VLCPlayerObject::Play(void)
{
    if (!playlist_files.size()) {
        return 0;
    }
    // Randomize play list
    if (playlist_shuffle) {
        for (int i = 0; i < playlist_files.size(); i++) {
            int j = (int) ((playlist_files.size() - 1) * (rand() / (RAND_MAX + 1.0)));
            if (i == j) {
                continue;
            }
            char *file = playlist_files[j];
            playlist_files[j] = playlist_files[i];
            playlist_files[i] = file;
        }
    }
    // Try all in the playlist until success
    for (int i = 0; i < playlist_files.size(); i++) {
       VLC_AddTarget(vlc, playlist_files[i]->name(), 0, 0, PLAYLIST_APPEND, PLAYLIST_END);
    }
    VLC_Play(vlc);
    has_audio = true;
    has_video = true;
    return PlayerObject::Play();
}

void VLCPlayerObject::Pause(void)
{
    PlayerObject::Pause();
    VLC_Pause(vlc);
}

void VLCPlayerObject::Stop(void)
{
    PlayerObject::Stop();
    VLC_CleanUp(vlc);
}

int VLCPlayerObject::GetTimePosition(int *len)
{
    if (len) {
        *len = VLC_LengthGet(vlc);
    }
    return VLC_TimeGet(vlc);
}

int VLCPlayerObject::Seek(int offset, bool absolute)
{
    VLC_TimeSet(vlc, offset, !absolute);
    return 0;
}

int VLCPlayerObject::PlaylistNext(void)
{
    VLC_PlaylistNext(vlc);
    return 0;
}

int VLCPlayerObject::PlaylistPrev(void)
{
    VLC_PlaylistPrev(vlc);
    return 0;
}

int VLCPlayerObject::GetPlaylistIndex(void)
{
    if (TryLock()) {
        return -1;
    }
    int rc = playlist_index = VLC_PlaylistIndex(vlc);
    Unlock();
    return rc;
}

int VLCPlayerObject::GetVolume(void)
{
    int rc = VLC_VolumeGet(vlc);
    return rc;
}

void VLCPlayerObject::SetVolume(int val)
{
    VLC_VolumeSet(vlc, val);
}

void VLCPlayerObject::SetMute(int val)
{
    VLC_VolumeMute(vlc);
}

int VLCPlayerObject::GetLength(void)
{
    int len = 0;
    GetTimePosition(&len);
    return len;
}

const char *VLCPlayerObject::GetStatus(void)
{
    if (VLC_IsPlaying(vlc)) {
        return "play";
    }
    return "idle";
}

#endif
