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

static Uint32 timer_callback(Uint32 interval, void *param)
{
    ((TimerObject *) param)->Fire();
    return interval;
}

TimerObject::TimerObject(const char *name, int interval, bool autostart):DynamicObject(name)
{
    this->interval = interval;
    this->enabled = false;
    this->autostart = autostart;
    this->type = TYPE_TIMER;
    this->timer_id = 0;
    this->defaultevent = "OnTimer";

    MemberFunctionProperty < TimerObject > *mp;
    mp = new MemberFunctionProperty < TimerObject > ("enabled", this, &TimerObject::pget_Enabled, &TimerObject::pset_Enabled, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TimerObject > ("interval", this, &TimerObject::pget_Interval, &TimerObject::pset_Interval, false);
    AddProperty(mp);

    MemberMethodHandler < TimerObject > *mh;
    mh = new MemberMethodHandler < TimerObject > ("start", this, 0, &TimerObject::m_Start);
    AddMethod(mh);
    mh = new MemberMethodHandler < TimerObject > ("stop", this, 0, &TimerObject::m_Stop);
    AddMethod(mh);
}


TimerObject::~TimerObject(void)
{
    SetEnabled(false);
}

void TimerObject::SetInterval(int interval)
{
    Lock();
    if (interval != this->interval) {
        this->interval = interval;
        if (this->enabled) {
            SetEnabled(true);
        }
    }
    Unlock();
}

void TimerObject::SetAutoStart(bool autostart)
{
    this->autostart = autostart;
}

void TimerObject::SetEnabled(bool enable)
{
    Lock();
    enabled = enable;
    if (timer_id) {
        SDL_RemoveTimer(timer_id);
        timer_id = 0;
    }
    if (enabled && interval > 0) {
        timer_id = SDL_AddTimer(interval, &timer_callback, (void *) this);
    }
    Unlock();
}

void TimerObject::Fire(void)
{
    FireEvent("OnTimer");
}

void TimerObject::OnStart(void)
{
    if (autostart) {
        SetEnabled(true);
    }
}

void TimerObject::OnStop(void)
{
    SetEnabled(false);
}

/* Property get/set functions */

// -- interval --

Variant TimerObject::pget_Interval(void)
{
    return anytovariant(interval);
}

int TimerObject::pset_Interval(Variant value)
{
    SetInterval(value);
    return 0;
}

// -- enabled --

Variant TimerObject::pget_Enabled(void)
{
    return anytovariant(enabled);
}

int TimerObject::pset_Enabled(Variant value)
{
    SetEnabled(value);
    return 0;
}

/* Method functions */

Variant TimerObject::m_Start(int numargs, Variant args[])
{
    SetEnabled(true);
    return VARNULL;
}

Variant TimerObject::m_Stop(int numargs, Variant args[])
{
    SetEnabled(false);
    return VARNULL;
}
