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

#ifdef HAVE_MIXER

static void sound_event()
{
    //soundObject.FireEvent("OnFinish");
}

SoundObject::SoundObject(const char *aname):DynamicObject(aname)
{
    init = 0;
    music = 0;
    repeat = 1;
    volume = 10;
    // Global sound object disabled means sound card does not support
    // multiple concurrent access
    if (!systemObject->configGetInt("sound_object", 1)) {
        flags |= WIDGET_DISABLED;
    }

    MemberFunctionProperty < SoundObject > *mp;
    mp = new MemberFunctionProperty < SoundObject > ("file", this, &SoundObject::pget_File, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SoundObject > ("repeat", this, &SoundObject::pget_Repeat, &SoundObject::pset_Repeat, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SoundObject > ("volume", this, &SoundObject::pget_Volume, &SoundObject::pset_Volume, false);
    AddProperty(mp);

    MemberMethodHandler < SoundObject > *mh;
    mh = new MemberMethodHandler < SoundObject > ("play", this, 1, &SoundObject::m_Play);
    AddMethod(mh);
    mh = new MemberMethodHandler < SoundObject > ("stop", this, 0, &SoundObject::m_Stop);
    AddMethod(mh);
    mh = new MemberMethodHandler < SoundObject > ("pause", this, 0, &SoundObject::m_Pause);
    AddMethod(mh);
    mh = new MemberMethodHandler < SoundObject > ("close", this, 0, &SoundObject::m_Close);
    AddMethod(mh);
}

SoundObject::~SoundObject(void)
{
    if (music) {
        Mix_FreeMusic(music);
        music = 0;
    }
    if (init) {
        Mix_CloseAudio();
        init = 0;
    }
}

Variant SoundObject::pget_File(void)
{
    string rc;
    Lock();
    rc = file;
    Unlock();
    return anytovariant(rc);
}

Variant SoundObject::pget_Volume(void)
{
    return anytovariant(Mix_VolumeMusic(-1));
}

int SoundObject::pset_Volume(Variant value)
{
    volume = value;
    Mix_VolumeMusic(volume);
    return 0;
}

Variant SoundObject::pget_Repeat(void)
{
    return anytovariant(repeat);
}

int SoundObject::pset_Repeat(Variant value)
{
    repeat = value;
    return 0;
}

Variant SoundObject::m_Play(int numargs, Variant args[])
{
    if (flags & WIDGET_DISABLED) {
        return anytovariant(0);
    }
    Lock();
    if (!init) {
        if (Mix_OpenAudio(44100, AUDIO_S16, 2, 4096)) {
            systemObject->Log(0,"SoundObject: Unable to open audio: %d", Mix_GetError());
            Unlock();
            return anytovariant(0);
        }
        Mix_VolumeMusic(volume);
        init = 1;
    }
    const char *arg = (const char *) args[0];
    if (strcmp(file.c_str(), arg)) {
        if (music) {
            Mix_FreeMusic(music);
        }
        if (!(music = Mix_LoadMUS(arg))) {
            systemObject->Log(0, "SoundObject: load: %s: %d", arg, Mix_GetError());
            Unlock();
            return anytovariant(0);
        }
        file = arg;
    }
    if (Mix_PlayMusic(music, repeat) != 0) {
        systemObject->Log(0, "SoundObject: play: %s: %d", file.c_str(), Mix_GetError());
        Mix_FreeMusic(music);
        music = 0;
        file = "";
        Unlock();
        return anytovariant(0);
    }
    Mix_HookMusicFinished(sound_event);
    Unlock();
    FireEvent("OnStart");
    return anytovariant(1);
}

Variant SoundObject::m_Stop(int numargs, Variant args[])
{
    Mix_HaltMusic();
    return VARNULL;
}

Variant SoundObject::m_Pause(int numargs, Variant args[])
{
    if (Mix_Paused(-1) > 0) {
        Mix_ResumeMusic();
    } else {
        Mix_PauseMusic();
    }
    return VARNULL;
}

Variant SoundObject::m_Close(int numargs, Variant args[])
{
    if (init) {
        Mix_CloseAudio();
    }
    if (music) {
        Mix_FreeMusic(music);
        music = 0;
    }
    file = "";
    init = 0;
    return VARNULL;
}

#endif
