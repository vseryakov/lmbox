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

RadioObject::RadioObject(const char *aname):DynamicObject(aname)
{
    fd = -1;
    version = 2;
    device = "/dev/radio0";
    volume = 65535;
    tuner = 0;
    status = 0;
    freq = 86.5;
    freq_step = 0.10;
    freq_level = 0.35;
    freq_min = 86.5;
    freq_max = 107.5;

    MemberFunctionProperty < RadioObject > *mp;
    mp = new MemberFunctionProperty < RadioObject > ("frequency", this, &RadioObject::pget_Frequency, &RadioObject::pset_Frequency, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < RadioObject > ("device", this, &RadioObject::pget_Device, &RadioObject::pset_Device, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < RadioObject > ("tuner", this, &RadioObject::pget_Tuner, &RadioObject::pset_Tuner, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < RadioObject > ("stereo", this, &RadioObject::pget_Stereo, &RadioObject::pset_Stereo, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < RadioObject > ("volume", this, &RadioObject::pget_Volume, &RadioObject::pset_Volume, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < RadioObject > ("status", this, &RadioObject::pget_Status, NULL, false);
    AddProperty(mp);

    MemberMethodHandler < RadioObject > *mh;
    mh = new MemberMethodHandler < RadioObject > ("play", this, 0, &RadioObject::m_Play);
    AddMethod(mh);
    mh = new MemberMethodHandler < RadioObject > ("stop", this, 0, &RadioObject::m_Stop);
    AddMethod(mh);
    mh = new MemberMethodHandler < RadioObject > ("seek", this, 1, &RadioObject::m_Seek);
    AddMethod(mh);
    mh = new MemberMethodHandler < RadioObject > ("close", this, 1, &RadioObject::m_Close);
    AddMethod(mh);
    mh = new MemberMethodHandler < RadioObject > ("setlimits", this, 3, &RadioObject::m_SetLimits);
    AddMethod(mh);
}

RadioObject::~RadioObject(void)
{
    RadioStop();
    RadioClose();
}

int RadioObject::RadioOpen()
{
    Lock();
    if (fd < 0) {
        if ((fd = open(device.c_str(), O_RDONLY)) < 0) {
            systemObject->Log(0, "RadioObject: Unable to open radio: %s", strerror(errno));
            Unlock();
            return fd;
        }
        memset(&v2caps, 0, sizeof(v2caps));
        if (ioctl(fd, VIDIOC_QUERYCAP, &v2caps) < 0) {
            systemObject->Log(0, "RadioOpen: VIDIOC_QUERYCAP: switch to v4l1: %s", strerror(errno));
            version = 1;
        }
        switch (version) {
        case 1:
            vt.tuner = tuner;
            if (ioctl(fd, VIDIOCGTUNER, &vt) < 0) {
                systemObject->Log(0, "RadioObject: Open: VIDIOCGTUNER: %d: %s", tuner, strerror(errno));
                break;
            }
            flags = vt.flags;
            break;

        case 2:
            if (ioctl(fd, VIDIOC_S_INPUT, &tuner) < 0) {
                systemObject->Log(0, "RadioObject: Tuner: VIDIOC_S_INPUT: %d: %s", tuner, strerror(errno));
            }
            memset(&v2tuner, 0, sizeof(v2tuner));
            v2tuner.index = 0;
            v2tuner.type = V4L2_TUNER_RADIO;
            if (ioctl(fd, VIDIOC_G_TUNER, &v2tuner) < 0) {
                systemObject->Log(0, "RadioObject: Open: VIDIOC_G_TUNER: %d: %s", tuner, strerror(errno));
                break;
            }
            flags = v2tuner.capability;
            break;
        }
    }
    Unlock();
    return fd;
}

void RadioObject::RadioStop(void)
{
    Lock();
    if (fd > -1) {
        switch (version) {
        case 1:
            if (ioctl(fd, VIDIOCGAUDIO, &va) < 0) {
                systemObject->Log(0, "RadioObject: Volume: VIDIOCGAUDIO: %s", strerror(errno));
            } 
            va.flags |= VIDEO_AUDIO_MUTE;
            if (ioctl(fd, VIDIOCSAUDIO, &va) < 0)
                systemObject->Log(0, "RadioObject: Stop: VIDIOCSAUDIO: %s", strerror(errno));
            break;

        case 2:
            v2ctrl.value = 1;
            v2ctrl.id = V4L2_CID_AUDIO_MUTE;
            if (ioctl(fd, VIDIOC_S_CTRL, &v2ctrl) == -1) {
                systemObject->Log(0, "RadioObject: Stop: VIDIOC_S_CTRL: %s", strerror(errno));
            }
            break;
        }
        status = XINE_STATUS_STOP;
    }
    Unlock();
}

void RadioObject::RadioClose(void)
{
    Lock();
    if (fd > -1) {
        close(fd);
    }
    fd = -1;
    Unlock();
}

Variant RadioObject::pget_Frequency(void)
{
    return anytovariant(freq);
}

int RadioObject::pset_Frequency(Variant value)
{
    unsigned long f;
    Lock();
    freq = value;
    if (freq < freq_min) {
        freq = freq_max;
    }
    if (freq > freq_max) {
        freq = freq_min;
    }  
    if (fd > -1) {
        switch (version) {
        case 1:
            f = (unsigned long) ceil(freq * (flags & VIDEO_TUNER_LOW ? 16000 : 16));
            if (ioctl(fd, VIDIOCSFREQ, &f) < 0) {
                systemObject->Log(0, "RadioObject: Frequency: VIDIOCSFREQ: %d: %s", f, strerror(errno));
            }  
            break;

        case 2:
            f = (unsigned long) ceil(freq * (flags & V4L2_TUNER_CAP_LOW ? 16000 : 16));
            memset(&v2freq, 0, sizeof(v2freq));
            v2freq.tuner = 0;
            v2freq.frequency = f;
            v2freq.type = V4L2_TUNER_RADIO;
            if (ioctl(fd, VIDIOC_S_FREQUENCY, &v2freq) < 0) {
                systemObject->Log(0, "RadioObject: Frequency: VIDIOC_S_FREQUENCY: %d: %s", f, strerror(errno));
            }
            break;
        }
    }
    Unlock();
    return 0;
}

Variant RadioObject::pget_Status(void)
{
    switch (status) {
    case XINE_STATUS_PLAY:
        return anytovariant("play");
    case XINE_STATUS_STOP:
        return anytovariant("stop");
    case XINE_STATUS_QUIT:
        return anytovariant("quit");
    }
    return anytovariant("idle");
}

Variant RadioObject::pget_Volume(void)
{
    return anytovariant(volume);
}

int RadioObject::pset_Volume(Variant value)
{
    Lock();
    volume = value;
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 65535) {
        volume = 65535;
    }
    if (fd > -1) {
        switch (version) {
        case 1:
            if (ioctl(fd, VIDIOCGAUDIO, &va) < 0) {
                systemObject->Log(0, "RadioObject: Volume: VIDIOCGAUDIO: %s", strerror(errno));
                Unlock();
                return 0;
            }
            va.volume = volume;
            if (ioctl(fd, VIDIOCSAUDIO, &va) < 0) {
                systemObject->Log(0, "RadioObject: Volume: VIDIOCSAUDIO: %d: %s", volume, strerror(errno));
            }
            break;

        case 2:
            v2ctrl.value = volume;
            v2ctrl.id = V4L2_CID_AUDIO_VOLUME;
            if (ioctl(fd, VIDIOC_S_CTRL, &v2ctrl) < 0) {
                systemObject->Log(0, "RadioObject: Volume: VIDIOC_S_CTRL: %d: %s", volume, strerror(errno));
            }
            break;
        }
    }
    Unlock();
    return 0;
}

Variant RadioObject::pget_Tuner(void)
{
    return anytovariant(tuner);
}

int RadioObject::pset_Tuner(Variant value)
{
    Lock();
    tuner = value;
    if (fd > -1) {
        switch (version) {
        case 1:
            vt.tuner = tuner;
            if (ioctl(fd, VIDIOCSTUNER, &vt) < 0)
                systemObject->Log(0, "RadioObject: Tuner: VIDIOCSTUNER: %d: %s", tuner, strerror(errno));
            break;

        case 2:
            if (ioctl(fd, VIDIOC_S_INPUT, &tuner) < 0) {
                systemObject->Log(0, "RadioObject: Tuner: VIDIOC_S_INPUT: %d: %s", tuner, strerror(errno));
            }
            break;
        }
    }
    Unlock();
    return 0;
}

Variant RadioObject::pget_Stereo(void)
{
    Lock();
    if (fd > -1) {
        switch (version) {
        case 1:
            if (ioctl(fd, VIDIOCGAUDIO, &va) < 0) {
                systemObject->Log(0, "RadioObject: stereo: VIDIOCGAUDIO: %s", strerror(errno));
            }
            stereo = va.mode & VIDEO_SOUND_STEREO;
            break;

        case 2:
            memset(&v2tuner, 0, sizeof(v2tuner));
            v2tuner.index = 0;
            if (ioctl(fd, VIDIOC_G_TUNER, &v2tuner) < -1) {
                systemObject->Log(0, "RadioObject: stereo: VIDIOC_G_TUNER: %s", strerror(errno));
            }
            stereo = v2tuner.audmode & V4L2_TUNER_MODE_STEREO;
            break;
        }
    }
    Unlock();
    return anytovariant(stereo);
}

int RadioObject::pset_Stereo(Variant value)
{
    Lock();
    if (fd > -1) {
        switch (version) {
        case 1:
            if (ioctl(fd, VIDIOCGAUDIO, &va) < 0) {
                systemObject->Log(0, "RadioObject: stereo: VIDIOCGAUDIO: %s", strerror(errno));
                Unlock();
                return 0;
            }
            va.mode = (int) value != 0 ? VIDEO_SOUND_STEREO : VIDEO_SOUND_MONO;
            if (ioctl(fd, VIDIOCSAUDIO, &va) < 0)
                systemObject->Log(0, "RadioObject: Stereo: VIDIOCSAUDIO: %s", strerror(errno));
            break;

        case 2:
            memset(&v2tuner, 0, sizeof(v2tuner));
            v2tuner.index = tuner;
            if (ioctl(fd, VIDIOC_G_TUNER, &v2tuner) < -1) {
                systemObject->Log(0, "RadioObject: Stereo: VIDIOC_G_TUNER: %s", strerror(errno));
            }
            v2tuner.audmode |= V4L2_TUNER_MODE_STEREO;
            if (ioctl(fd, VIDIOC_S_TUNER, &v2tuner) < -1) {
                systemObject->Log(0, "RadioObject: Stereo: VIDIOC_S_TUNER: %s", strerror(errno));
            }
            break;
        }
    }
    Unlock();
    return 0;
}

Variant RadioObject::pget_Device(void)
{
    Lock();
    string rc = device;
    Unlock();
    return anytovariant(rc);
}

int RadioObject::pset_Device(Variant value)
{
    Lock();
    device = (const char *) value;
    Unlock();
    return 0;
}

Variant RadioObject::m_Play(int numargs, Variant args[])
{
    if (RadioOpen() < 0)
        return anytovariant(0);
    Unlock();
    switch (version) {
    case 1:
        if (ioctl(fd, VIDIOCGAUDIO, &va) < 0) {
            systemObject->Log(0, "RadioObject: Volume: VIDIOCGAUDIO: %s", strerror(errno));
        }
        va.volume = volume;
        va.mode = VIDEO_SOUND_STEREO;
        va.flags &= ~VIDEO_AUDIO_MUTE;
        if (ioctl(fd, VIDIOCSAUDIO, &va) < 0) {
            systemObject->Log(0, "RadioObject: Play: VIDIOCSAUDIO: %d: %s", volume, strerror(errno));
        }
        break;

    case 2:
        v2ctrl.value = 0;
        v2ctrl.id = V4L2_CID_AUDIO_MUTE;
        if (ioctl(fd, VIDIOC_S_CTRL, &v2ctrl) == -1) {
            systemObject->Log(0, "RadioObject: Play: VIDIOC_S_CTRL: %s", strerror(errno));
        }
        v2ctrl.value = volume;
        v2ctrl.id = V4L2_CID_AUDIO_VOLUME;
        if (ioctl(fd, VIDIOC_S_CTRL, &v2ctrl) == -1) {
            systemObject->Log(0, "RadioObject: Volume: VIDIOC_S_CTRL: %d: %s", volume, strerror(errno));
        }
        break;
    }
    status = XINE_STATUS_PLAY;
    Unlock();
    FireEvent("OnStart");
    return anytovariant(1);
}

Variant RadioObject::m_Stop(int numargs, Variant args[])
{
    RadioStop();
    FireEvent("OnStop");
    return VARNULL;
}

Variant RadioObject::m_Close(int numargs, Variant args[])
{
    RadioStop();
    RadioClose();
    FireEvent("OnFinish");
    return VARNULL;
}

Variant RadioObject::m_Seek(int numargs, Variant args[])
{
    if (RadioOpen() < 0) {
        return VARNULL;
    }
    int where = args[0];
    int count = (int)((freq_max - freq_min) / freq_step);
    unsigned long fval;
    struct video_tuner vt;
    double freq_cur = freq;
    Lock();
    while (count--) {
        freq_cur += (where >= 0 ? freq_step : -freq_step);
        if (freq_cur < freq_min) {
            freq_cur = freq_max;
        }
        if (freq_cur > freq_max) {
            freq_cur = freq_min;
        }
        switch (version) {
        case 1:
            fval = (unsigned long) ceil(freq_cur * (flags & VIDEO_TUNER_LOW ? 16000 : 16));
            if (ioctl(fd, VIDIOCSFREQ, &fval) < 0) {
                systemObject->Log(0, "RadioObject: Seek: VIDIOCSFREQ: %d: %s", fval, strerror(errno));
                freq_cur = freq;
                goto stop;
            }
            vt.tuner = tuner;
            if (ioctl(fd, VIDIOCGTUNER, &vt) < 0) {
                systemObject->Log(0, "RadioObject: Seek: VIDIOCGTUNER: %d: %s", tuner, strerror(errno));
                freq_cur = freq;
                goto stop;
            }
            usleep(15000);
            if (vt.signal / 65536.0 > freq_level) {
                goto stop;
            }
            break;

        case 2:
            fval = (unsigned long) ceil(freq_cur * (flags & V4L2_TUNER_CAP_LOW ? 16000 : 16));
            memset(&v2freq, 0, sizeof(v2freq));
            v2freq.tuner = 0;
            v2freq.type = V4L2_TUNER_RADIO;
            v2freq.frequency = (unsigned int) freq_cur;
            if (ioctl(fd, VIDIOC_S_FREQUENCY, &v2freq) < 0) {
                systemObject->Log(0, "RadioObject: Seek: VIDIOC_S_FREQUENCY: %d: %s", fval, strerror(errno));
                freq_cur = freq;
                goto stop;
            }
            memset(&v2tuner, 0, sizeof(v2tuner));
            v2tuner.index = 0;
            if (ioctl(fd, VIDIOC_G_TUNER, &v2tuner) < 0) {
                systemObject->Log(0, "RadioObject: Seek: VIDIOC_G_TUNER: %d: %s", tuner, strerror(errno));
            }
            usleep(15000);
            if (v2tuner.signal / 65536.0 > freq_level) {
                goto stop;
            }
            systemObject->Log(0, "radio: %0.2f: %d", freq_cur, v2tuner.signal);
            break;
        }
    }
  stop:
    freq = freq_cur;
    Unlock();
    return VARNULL;
}

Variant RadioObject::m_SetLimits(int numargs, Variant args[])
{
    Lock();
    freq_min = args[0];
    freq_max = args[1];
    freq_step = args[2];
    freq_level = args[3];
    Unlock();
    return VARNULL;
}
