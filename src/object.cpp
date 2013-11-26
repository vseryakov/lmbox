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

enum {
  VARTYPE_UNINITIALISED,
  VARTYPE_INT,
  VARTYPE_ULONG,
  VARTYPE_BOOL,
  VARTYPE_STRING,
  VARTYPE_OBJREF,
  VARTYPE_DOUBLE
};

static struct {
  char *name;
  int flags;
} mapFlags[] = {
                 { "busy", OBJECT_BUSY},
                 { "seekable", OBJECT_SEEKABLE},
                 { "selectable", WIDGET_SELECTABLE},
                 { "disabled", WIDGET_DISABLED},
                 { "pressed", WIDGET_PRESSED},
                 { "inside", WIDGET_INSIDE},
                 { "hidden", WIDGET_HIDDEN},
                 { "changed", WIDGET_CHANGED},
                 { "transparent", WIDGET_TRANSPARENT},
                 { "has_focus", WIDGET_FOCUSED},
                 { "transform", WIDGET_TRANSFORM},
                 { NULL, 0 } };

static struct {
  char *name;
  int type;
} mapTypes[] = {
                 { "object", TYPE_OBJECT},
                 { "widget", TYPE_WIDGET},
                 { "gfx", TYPE_GFX},
                 { "label", TYPE_LABEL},
                 { "listbox", TYPE_LISTBOX},
                 { "container", TYPE_CONTAINER},
                 { "page", TYPE_PAGE},
                 { "screen", TYPE_SCREEN},
                 { "scrollbar", TYPE_SCROLLBAR},
                 { "clocklabel", TYPE_CLOCKLABEL},
                 { "button", TYPE_BUTTON},
                 { "togglebutton", TYPE_TOGGLEBUTTON},
                 { "textfield", TYPE_TEXTFIELD},
                 { "filebrowser", TYPE_FILEBROWSER},
                 { "surface", TYPE_SURFACE},
                 { "layout", TYPE_LAYOUT},
                 { "vboxlayout", TYPE_VBOXLAYOUT},
                 { "hboxlayout", TYPE_HBOXLAYOUT},
                 { "eventhandler", TYPE_EVENTHANDLER},
                 { "callback", TYPE_CALLBACK},
                 { "timer", TYPE_TIMER},
                 { "player", TYPE_PLAYER},
                 { "tv", TYPE_TV},
                 { "sound", TYPE_SOUND},
                 { "radio", TYPE_RADIO},
                 { "webbrowser", TYPE_WEBBROWSER},
                 { "system", TYPE_SYSTEM},
                 { "dynamicobject", TYPE_DYNAMICOBJECT},
                 { "font", TYPE_FONT},
                 { "picture", TYPE_PICTURE},
                 { NULL, 0 } };

Object::Object(const char *name)
{
    this->flags = 0;
    this->refcount = 1;
    this->name = lmbox_strdup(name);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&lock, &attr);
    pthread_mutexattr_destroy(&attr);
}

Object::~Object(void)
{
    lmbox_free(name);
    pthread_mutex_destroy(&lock);
}

int Object::RefCount()
{
    Lock();
    int rc = refcount;
    Unlock();
    return rc;
}

void Object::SetRefCount(int count)
{
    Lock();
    refcount = count;
    Unlock();
}

int Object::IncRef()
{
    Lock();
    int rc = ++refcount;
    Unlock();
    return rc;
}

int Object::DecRef(void)
{
    Lock();
    if (refcount <= 0) {
        systemObject->Log(0, "%s/%d: already zero refcount %d", name, type, refcount);
    }
    refcount--;
    Unlock();
    if (refcount <= 0) {
        delete this;
        return 1;
    }
    return 0;
}

void Object::SetParent(Object * source)
{
}

Object *Object::GetParent(void)
{
    return 0;
}

int Object::Keep(Object ** target, Object * source)
{
    int rc = 0;
    Lock();
    if (source != *target) {
        if (source) {
            source->IncRef();
        }
        if (*target) {
            (*target)->DecRef();
        }
        (*target) = source;
        SetChanged(1);
        rc = 1;
    }
    Unlock();
    return rc;
}

const char *Object::GetName(void)
{
    return name;
}

void Object::WriteFlags(int andmask, int ormask)
{
    Lock();
    int oldflags = flags;
    flags = (flags & andmask) | ormask;
    Unlock();
    if (oldflags != flags) {
        OnFlags(oldflags);
    }
}

void Object::ToggleFlags(int mask)
{
    if (GetFlags() & mask) {
        WriteFlags(~mask, 0);
    } else {
        WriteFlags(-1, mask);
    }
}

void Object::SetFlags(int mask)
{
    WriteFlags(-1, mask);
}

void Object::ClearFlags(int mask)
{
    WriteFlags(~mask, 0);
}

int Object::GetFlags(void)
{
    Lock();
    int rc = flags;
    Unlock();
    return rc;
}

int Object::IsBusy(void)
{
    if (TryLock()) {
        return 1;
    }
    Unlock();
    int count = 0;
    while (GetFlags() & OBJECT_BUSY) {
        if (count++ > 2) {
            return 1;
        }
        SDL_Delay(100);
    }
    return 0;
}

void Object::SetChanged(int flag)
{
    if (flag) {
        SetFlags(WIDGET_CHANGED);
    } else {
        ClearFlags(WIDGET_CHANGED);
    }
}

int Object::IsChanged()
{
    return (GetFlags() & WIDGET_CHANGED);
}

int Object::Lock(void)
{
    pthread_mutex_lock(&lock);
    return 0;
}

int Object::TryLock(void)
{
    return pthread_mutex_trylock(&lock);
}

int Object::Unlock(void)
{
    pthread_mutex_unlock(&lock);
    return 0;
}

void Object::OnEvent(void)
{
}

void Object::OnStart(void)
{
}

void Object::OnExpose(void)
{
    SetChanged(1);
}

void Object::OnStop(void)
{
}

void Object::OnFlags(int oldflags)
{
}

const char *Object::GetType(void)
{
    for (int i = 0; mapTypes[i].name; i++) {
         if (type == mapTypes[i].type) {
             return mapTypes[i].name;
         }
    }
    return "";
}

DynamicObject::DynamicObject(const char *name):Object(name)
{
    this->type = TYPE_DYNAMICOBJECT;
    this->defaultproperty = 0;
    this->defaultevent = 0;
    this->timer_id = 0;
    this->timer_script = 0;
    this->data = 0;
    pthread_cond_init(&msgcond, 0);
    pthread_mutex_init(&msglock, 0);

    MemberFunctionProperty < DynamicObject > *mp;
    mp = new MemberFunctionProperty < DynamicObject > ("name", this, &DynamicObject::pget_Name, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("type", this, &DynamicObject::pget_Type, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("methodcount", this, &DynamicObject::pget_MethodCount, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("flags", this, &DynamicObject::pget_Flags, &DynamicObject::pset_Flags, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("timerinterval", this, &DynamicObject::pget_TimerInterval, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("timerscript", this, &DynamicObject::pget_TimerScript, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("data", this, &DynamicObject::pget_Data, &DynamicObject::pset_Data, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("events", this, &DynamicObject::pget_EventHandlers, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("objects", this, &DynamicObject::pget_Objects, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < DynamicObject > ("properties", this, &DynamicObject::pget_Properties, NULL, false);
    AddProperty(mp);

    MemberMethodHandler < DynamicObject > *mh;
    mh = new MemberMethodHandler < DynamicObject > ("getproperty", this, 1, &DynamicObject::m_GetProperty);
    AddMethod(mh);
    mh = new MemberMethodHandler < DynamicObject > ("getmethod", this, 1, &DynamicObject::m_GetMethod);
    AddMethod(mh);
    mh = new MemberMethodHandler < DynamicObject > ("settimer", this, 2, &DynamicObject::m_SetTimer);
    AddMethod(mh);
    mh = new MemberMethodHandler < DynamicObject > ("firetimer", this, 0, &DynamicObject::m_FireTimer);
    AddMethod(mh);
    mh = new MemberMethodHandler < DynamicObject > ("addeventhandler", this, 2, &DynamicObject::m_AddEventHandler);
    AddMethod(mh);
    mh = new MemberMethodHandler < DynamicObject > ("removeeventhandler", this, 1, &DynamicObject::m_RemoveEventHandler);
    AddMethod(mh);
    mh = new MemberMethodHandler < DynamicObject > ("message", this, 2, &DynamicObject::m_Message);
    AddMethod(mh);
}

DynamicObject::~DynamicObject(void)
{
    lmbox_free(data);
    lmbox_free(timer_script);
    if (timer_id > 0) {
        SDL_RemoveTimer(timer_id);
    }
    DestroyObjects();
    DestroyEventHandlers();
    while (properties.size() > 0) {
        delete properties.back();
        properties.pop_back();
    }
    while (methodhandlers.size() > 0) {
        MethodHandler *handler = methodhandlers.back();
        methodhandlers.pop_back();
        delete handler;
    }
    while (msgqueue.size() > 0) {
        free(msgqueue.front());
        msgqueue.pop();
    }
    pthread_cond_destroy(&msgcond);
    pthread_mutex_destroy(&msglock);
}

int DynamicObject::Initialize(void)
{
    return FireEvent("OnInit") != -1;
}

Display *DynamicObject::GetDisplay()
{
    return systemObject->GetDisplay();
}

Window DynamicObject::GetWindow()
{
    return systemObject->GetWindow();
}

int DynamicObject::SendMessage(const char *msg)
{
    if (msg) {
        pthread_mutex_lock(&msglock);
        msgqueue.push(strdup(msg));
        pthread_cond_signal(&msgcond);
        pthread_mutex_unlock(&msglock);
    }
    return 0;
}

char *DynamicObject::ReadMessage(int timeout)
{
    int rc = 0;
    char *msg = 0;
    struct timeval now;
    struct timespec ts;

    pthread_mutex_lock(&msglock);
    do {
      if (msgqueue.size() > 0) {
          msg = msgqueue.front();
          msgqueue.pop();
          break;
      }
      gettimeofday(&now, 0);
      ts.tv_sec = now.tv_sec + timeout;
      ts.tv_nsec = now.tv_usec * 1000;
      rc = pthread_cond_timedwait(&msgcond, &msglock, &ts);
    } while (rc == 0 || rc == EINTR);
    pthread_mutex_unlock(&msglock);
    return msg;
}

void DynamicObject::RemoveObject(DynamicObject * object)
{
    Lock();
    for (int i = 0; i < containedobjects.size(); i++) {
        if (containedobjects[i] == object) {
            containedobjects.erase(containedobjects.begin() + i);
            object->DecRef();
            break;
        }
    }
    Unlock();
}

void DynamicObject::AddObject(DynamicObject * object)
{
    Lock();
    containedobjects.push_back(object);
    Unlock();
}

DynamicObject *DynamicObject::FindObject(const char *name)
{
    DynamicObject *obj = 0;
    Lock();
    for (int i = 0; i < containedobjects.size(); i++) {
        if (!strcmp(containedobjects[i]->GetName(), name)) {
            obj = containedobjects[i];
            obj->IncRef();
            break;
        }
    }
    Unlock();
    return obj;
}

void DynamicObject::StartObjects(void)
{
    vector < DynamicObject * > objects;
    Lock();
    for (int i = 0; i < containedobjects.size(); i++) {
        containedobjects[i]->IncRef();
        objects.push_back(containedobjects[i]);
    }
    Unlock();
    while (objects.size() > 0) {
        objects.back()->OnStart();
        objects.back()->DecRef();
        objects.pop_back();
    }
}

void DynamicObject::StopObjects(void)
{
    vector < DynamicObject * > objects;
    Lock();
    for (int i = 0; i < containedobjects.size(); i++) {
        containedobjects[i]->IncRef();
        objects.push_back(containedobjects[i]);
    }
    Unlock();
    while (objects.size() > 0) {
        objects.back()->OnStop();
        objects.back()->DecRef();
        objects.pop_back();
    }
}

void DynamicObject::DestroyObjects(void)
{
    while (containedobjects.size() > 0) {
        DynamicObject *object = containedobjects.back();
        containedobjects.pop_back();
        object->DecRef();
    }
}

void DynamicObject::DestroyEventHandlers(void)
{
    while (eventhandlers.size() > 0) {
        EventHandler *handler = eventhandlers.back();
        eventhandlers.pop_back();
        handler->DecRef();
    }
}

void DynamicObject::AddEventHandler(const char *event, const char *code)
{
    char *ename = strdup(event);

    while (ename) {
        char *end = strchr(ename, ',');
        if (end) {
            *end++ = 0;
        }
        Script *script = new Script(ename, code);
        EventHandler *handler = new EventHandler(ename, script);
        RegisterEventHandler(handler);
        handler->DecRef();
        ename = end;
    }
    free(ename);
}

void DynamicObject::RegisterEventHandler(EventHandler *handler)
{
    Lock();
    eventhandlers.push_back(handler);
    handler->IncRef();
    Unlock();
}

void DynamicObject::UnregisterEventHandler(EventHandler * handler)
{
    Lock();
    for (int i = 0; i < eventhandlers.size(); i++) {
        if (eventhandlers[i] == handler) {
            eventhandlers.erase(eventhandlers.begin() + i);
            handler->DecRef();
            break;
        }
    }
    Unlock();
}

EventHandler *DynamicObject::FindEventHandler(const char *name)
{
    EventHandler *handler = 0;
    Lock();
    for (int i = 0; i < eventhandlers.size(); i++) {
        if (!systemObject->IsRunning() && !strcmp(eventhandlers[i]->GetName(),"*")) {
            continue;
        }
        if (eventhandlers[i]->IsEqual(name)) {
            handler = eventhandlers[i];
            handler->IncRef();
            break;
        }
    }
    Unlock();
    return handler;
}

bool DynamicObject::HandleEvent(const char *action)
{
    if (FireEvent(action)) {
        systemObject->Log(3, "HandleEvent: %s processed event %s", name, action);
        return true;
    }
    return false;
}

bool DynamicObject::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    return 0;
}

int DynamicObject::FireEvent(const char *event)
{
    int rc = 0;
    EventHandler *handler = FindEventHandler(event);
    if (handler) {
        Tls *tls = tlsGet();
        tls->object = this;
        lmbox_free(tls->event);
        tls->event = strdup(event);
        systemObject->Log(3, "Executing event handler %s for event %s in %s (run=%d)", handler->GetName(), event, name, systemObject->IsRunning());
        rc = handler->Fire();
        systemObject->Log(5, "Result is %d in %s from event handler %s for event %s", rc, name, handler->GetName(), event);
        handler->DecRef();
        lmbox_destroy((void**)&tls->event);
    }
    return rc;
}

int DynamicObject::AddMethod(MethodHandler * mh)
{
    methodhandlers.push_back(mh);
    return 0;
}

void DynamicObject::RegisterMethod(MethodHandler *handler)
{
    methodhandlers.push_back(handler);
}

void DynamicObject::UnregisterMethod(MethodHandler * handler)
{
    for (int i = 0; i < methodhandlers.size(); i++) {
        if (methodhandlers[i] == handler) {
            methodhandlers.erase(methodhandlers.begin() + i);
            delete handler;
            break;
        }
    }
}

Variant DynamicObject::MethodCall(const char *method, int numargs, Variant args[])
{
    for (int i = 0; i < methodhandlers.size(); i++) {
        if (!strcmp(methodhandlers[i]->GetName(), method)) {
            return methodhandlers[i]->Call(numargs, args);
        }
    }
    systemObject->Log(0, "Object %s does not have a method %s", name, method);
    return VARNULL;
}

bool DynamicObject::HasMethod(const char *method)
{
    for (int i = 0; i < methodhandlers.size(); i++) {
        if (!strcmp(methodhandlers[i]->GetName(), method)) {
            return true;
        }
    }
    return false;
}

MethodHandler *DynamicObject::GetMethod(int index)
{
    if (index < methodhandlers.size() && index >= 0) {
        return methodhandlers[index];
    }
    return NULL;
}

int DynamicObject::GetMethodCount(void)
{
    return methodhandlers.size();
}

const char *DynamicObject::GetDefaultEvent(void)
{
    return defaultevent;
}

int DynamicObject::AddProperty(ObjectProperty * prop)
{
    Lock();
    properties.push_back(prop);
    Unlock();
    return 0;
}

Variant DynamicObject::GetPropertyValue(const char *property)
{
    ObjectProperty *prop = FindProperty(property);
    if (prop) {
        return prop->GetValue();
    }
    systemObject->Log(0, "Object %s does not have a property %s", name, property);
    return VARNULL;
}

int DynamicObject::SetPropertyValue(const char *property, Variant value)
{
    ObjectProperty *prop = FindProperty(property);
    if (prop) {
        return prop->SetValue(value);
    }
    systemObject->Log(0, "Object %s does not have a property %s", GetName(), property);
    return -1;
}

ObjectProperty *DynamicObject::FindProperty(const char *name)
{
    ObjectProperty *prop = 0;
    Lock();
    for (int i = 0; i < properties.size(); i++) {
        if (!strcmp(properties[i]->GetName(), name)) {
            prop = properties[i];
            break;
        }
    }
    Unlock();
    return prop;
}

ObjectProperty *DynamicObject::GetProperty(int index)
{
    if (index < properties.size() && index >= 0) {
        return properties[index];
    }
    return NULL;
}

const char *DynamicObject::GetDefaultProperty(void)
{
    if (defaultproperty) {
        return defaultproperty->GetName();
    }
    return NULL;
}

int DynamicObject::GetPropertyCount(void)
{
    return properties.size();
}

bool DynamicObject::HasProperty(const char *property)
{
    return ((this->FindProperty(property)) != NULL);
}

void DynamicObject::FireTimer(void)
{
    Script *script = 0;
    Lock();
    if (timer_script) {
        script = new Script(GetName(), timer_script);
    }
    Unlock();
    if (script) {
      script->Execute();
      delete script;
    }
}

static Uint32 timer_callback(Uint32 interval, void *param)
{
    ((DynamicObject *) param)->FireTimer();
    return interval;
}

void DynamicObject::SetTimer(int interval, const char *script)
{
    Lock();
    lmbox_free(timer_script);
    timer_script = lmbox_strdup(script);
    timer_interval = interval;
    if (timer_id > 0) {
        SDL_RemoveTimer(timer_id);
        timer_id = 0;
    }
    if (timer_interval > 0) {
        timer_id = SDL_AddTimer(timer_interval, &timer_callback, (void *) this);
    }
    Unlock();
}

Variant DynamicObject::pget_Name(void)
{
    return anytovariant(GetName());
}

Variant DynamicObject::pget_Type(void)
{
    return anytovariant(GetType());
}

Variant DynamicObject::pget_MethodCount(void)
{
    return anytovariant(GetMethodCount());
}

Variant DynamicObject::pget_Flags(void)
{
    string str;
    for (int i = 0; mapFlags[i].name; i ++) {
        if (GetFlags() & mapFlags[i].flags) {
            str += mapFlags[i].name;
            str += " ";
         }
    }
    return anytovariant(str);
}

int DynamicObject::pset_Flags(Variant value)
{
    char *ptr, *str = value;
    for (int i = 0; mapFlags[i].name; i ++) {
        if ((ptr = strstr(str, mapFlags[i].name))) {
            if (ptr > str && *(ptr-1) == '!') {
                ClearFlags(mapFlags[i].flags);
            } else {
                SetFlags(mapFlags[i].flags);
            }
        }
    }
    return 0;
}

Variant DynamicObject::pget_TimerInterval(void)
{
    return anytovariant(timer_interval);
}

Variant DynamicObject::pget_TimerScript(void)
{
    Lock();
    Variant rc = anytovariant(timer_script ? timer_script : "");
    Unlock();
    return rc;
}

Variant DynamicObject::pget_Data(void)
{
    Lock();
    Variant rc = anytovariant(data ? data : "");
    Unlock();
    return rc;
}

int DynamicObject::pset_Data(Variant value)
{
    Lock();
    lmbox_free(data);
    data = lmbox_strdup((const char *)value);
    Unlock();
    return 0;
}

Variant DynamicObject::pget_Objects(void)
{
    string rc;
    Lock();
    for (int i = 0; i < containedobjects.size(); i++) {
         Object *handler = containedobjects[i];
         rc += handler->GetName();
         rc += " ";
    }
    Unlock();
    return anytovariant(rc);
}

Variant DynamicObject::pget_EventHandlers(void)
{
    string rc;
    for (int i = 0; i < eventhandlers.size(); i++) {
         rc += eventhandlers[i]->GetName();
         rc += " ";
    }
    return anytovariant(rc);
}

Variant DynamicObject::pget_Properties(void)
{
    string rc;
    Lock();
    for (int i = 0; i < properties.size(); i++) {
         rc += properties[i]->GetName();
         rc += " ";
    }
    Unlock();
    return anytovariant(rc);
}

Variant DynamicObject::m_GetProperty(int numargs, Variant args[])
{
    ObjectProperty *prop = GetProperty(args[0]);
    if (prop) {
        return anytovariant(prop->GetName());
    }
    return VARNULL;
}

Variant DynamicObject::m_GetMethod(int numargs, Variant args[])
{
    MethodHandler *mthd = GetMethod(args[0]);
    if (mthd) {
        return anytovariant(mthd->GetName());
    }
    return VARNULL;
}

Variant DynamicObject::m_SetTimer(int numargs, Variant args[])
{
    SetTimer(args[0], args[1]);
    return VARNULL;
}

Variant DynamicObject::m_FireTimer(int numargs, Variant args[])
{
    FireTimer();
    return VARNULL;
}

Variant DynamicObject::m_AddEventHandler(int numargs, Variant args[])
{
    AddEventHandler(args[0], args[1]);
    return VARNULL;
}

Variant DynamicObject::m_RemoveEventHandler(int numargs, Variant args[])
{
    EventHandler *handler = FindEventHandler(args[0]);
    if (handler) {
        UnregisterEventHandler(handler);
        handler->DecRef();
    }
    return VARNULL;
}

Variant DynamicObject::m_Message(int numargs, Variant args[])
{
    Variant rc = VARNULL;

    if (!strcmp(args[0], "send")) {
        SendMessage(args[1]);
    } else

    if (!strcmp(args[0], "read")) {
        char *msg = ReadMessage(args[1]);
        if (msg) {
            rc = anytovariant(msg);
            free(msg);
        }
    }
    return rc;
}

Variant::Variant(void)
{
    internaltype = VARTYPE_UNINITIALISED;
    ClearVars();

}

Variant::~Variant(void)
{
}

bool Variant::IsEmpty(void)
{
    return empty;
}

void Variant::Clear(void)
{
    ClearVars();
}

void Variant::ClearVars(void)
{
    ulongvalue = 0;
    intvalue = 0;
    stringvalue = "";
    empty = true;
}

int Variant::GetType(void)
{
    return internaltype;
}

void Variant::SetType(int type)
{
    internaltype = type;
}

Variant Variant::operator=(string value)
{
    if (internaltype == VARTYPE_UNINITIALISED) {
        SetType(VARTYPE_STRING);
    }
    switch (internaltype) {
    case VARTYPE_STRING:
        stringvalue = value;
        break;
    case VARTYPE_ULONG:
        ulongvalue = atol(value.c_str());
        break;
    case VARTYPE_INT:
        intvalue = atoi(value.c_str());
        break;
    case VARTYPE_DOUBLE:
        dblvalue = atof(value.c_str());
        break;
    case VARTYPE_BOOL:
        if (strToBool(value.c_str()))
            intvalue = true;
        else
            intvalue = false;
    }
    return *this;
}

Variant Variant::operator=(const char *value)
{
    string str;
    str = value;
    return operator=(str);
}

Variant Variant::operator+=(const char *value)
{
    string str = *this;
    str += value;
    return operator=(str);
}

Variant Variant::operator=(void *value)
{
    SetType(VARTYPE_OBJREF);
    intvalue = (int) value;
    return *this;
}

Variant Variant::operator=(int value)
{
    if (internaltype == VARTYPE_UNINITIALISED) {
        SetType(VARTYPE_INT);
    }
    switch (internaltype) {
    case VARTYPE_STRING:{
            ostringstream strm;
            strm << value;
            stringvalue = strm.str();
            break;
        }
    case VARTYPE_ULONG:
        ulongvalue = value;
        break;
    case VARTYPE_INT:
        intvalue = value;
        break;
    case VARTYPE_DOUBLE:
        dblvalue = value;
        break;
    case VARTYPE_BOOL:
        if (value)
            intvalue = true;
        else
            intvalue = false;
        break;
    }
    return *this;
}

Variant Variant::operator=(unsigned long value)
{
    if (internaltype == VARTYPE_UNINITIALISED) {
        SetType(VARTYPE_ULONG);
    }
    switch (internaltype) {
    case VARTYPE_STRING:{
            ostringstream strm;
            strm << value;
            stringvalue = strm.str();
            break;
        }
    case VARTYPE_ULONG:
        ulongvalue = value;
        break;
    case VARTYPE_INT:
        intvalue = value;
        break;
    case VARTYPE_DOUBLE:
        dblvalue = value;
        break;
    case VARTYPE_BOOL:
        if (value)
            intvalue = true;
        else
            intvalue = false;
        break;
    }
    return *this;
}

Variant Variant::operator=(double value)
{
    if (internaltype == VARTYPE_UNINITIALISED) {
        SetType(VARTYPE_DOUBLE);
    }
    switch (internaltype) {
    case VARTYPE_STRING:{
            ostringstream strm;
            strm << value;
            stringvalue = strm.str();
            break;
        }
    case VARTYPE_ULONG:
        ulongvalue = (unsigned long) value;
        break;
    case VARTYPE_INT:
        intvalue = (int) value;
        break;
    case VARTYPE_DOUBLE:
        dblvalue = value;
        break;
    case VARTYPE_BOOL:
        if (value)
            dblvalue = true;
        else
            dblvalue = false;
        break;
    }
    return *this;
}

Variant Variant::operator=(bool value)
{
    if (internaltype == VARTYPE_UNINITIALISED) {
        SetType(VARTYPE_BOOL);
    }
    switch (internaltype) {
    case VARTYPE_STRING:
        if (value)
            stringvalue = "true";
        else
            stringvalue = "false";
        break;
    case VARTYPE_DOUBLE:
        dblvalue = value;
        break;
    case VARTYPE_ULONG:
        ulongvalue = value;
        break;
    case VARTYPE_BOOL:
    case VARTYPE_INT:
        intvalue = value;
        break;
    }
    return *this;
}

Variant::operator std::string()
{
    switch (internaltype) {
    case VARTYPE_STRING:
        break;
    case VARTYPE_INT:{
            ostringstream strm;
            strm << intvalue;
            stringvalue = strm.str();
            break;
        }
    case VARTYPE_ULONG:{
            ostringstream strm;
            strm << ulongvalue;
            stringvalue = strm.str();
            break;
        }
    case VARTYPE_DOUBLE:{
            ostringstream strm;
            strm << dblvalue;
            stringvalue = strm.str();
            break;
        }
    case VARTYPE_BOOL:
        if (intvalue)
            stringvalue = "true";
        else
            stringvalue = "false";
        break;
    case VARTYPE_UNINITIALISED:
        stringvalue = "";
        break;
    }
    return stringvalue;
}

Variant::operator int ()
{
    switch (internaltype) {
    case VARTYPE_STRING:
        intvalue = atoi(stringvalue.c_str());
        break;
    case VARTYPE_DOUBLE:
        intvalue = (int) dblvalue;
        break;
    case VARTYPE_ULONG:
        intvalue = (int) ulongvalue;
        break;
    case VARTYPE_BOOL:
    case VARTYPE_INT:
        break;
    case VARTYPE_UNINITIALISED:
        intvalue = 0;
        break;
    }
    return intvalue;
}

Variant::operator unsigned long ()
{
    switch (internaltype) {
    case VARTYPE_STRING:
        ulongvalue = atol(stringvalue.c_str());
        break;
    case VARTYPE_DOUBLE:
        ulongvalue = (unsigned long) dblvalue;
        break;
    case VARTYPE_BOOL:
    case VARTYPE_INT:
        ulongvalue = intvalue;
        break;
    case VARTYPE_ULONG:
        break;
    case VARTYPE_UNINITIALISED:
        ulongvalue = 0;
        break;
    }
    return ulongvalue;
}

Variant::operator short ()
{
    switch (internaltype) {
    case VARTYPE_STRING:
        intvalue = atoi(stringvalue.c_str());
        break;
    case VARTYPE_DOUBLE:
        intvalue = (int) dblvalue;
        break;
    case VARTYPE_ULONG:
        intvalue = (int) ulongvalue;
        break;
    case VARTYPE_BOOL:
    case VARTYPE_INT:
        break;
    case VARTYPE_UNINITIALISED:
        intvalue = 0;
        break;
    }
    return intvalue;
}

Variant::operator double ()
{
    switch (internaltype) {
    case VARTYPE_STRING:
        dblvalue = atof(stringvalue.c_str());
        break;
    case VARTYPE_DOUBLE:
        dblvalue = (int) ulongvalue;
        break;
    case VARTYPE_BOOL:
    case VARTYPE_INT:
        dblvalue = intvalue;
        break;
    case VARTYPE_UNINITIALISED:
        dblvalue = 0;
        break;
    }
    return dblvalue;
}

Variant::operator bool()
{
    switch (internaltype) {
    case VARTYPE_STRING:
        if (strToBool(stringvalue.c_str()))
            intvalue = true;
        else
            intvalue = false;
        break;
    case VARTYPE_DOUBLE:
        intvalue = (int) dblvalue;
        break;
    case VARTYPE_ULONG:
        intvalue = (int) ulongvalue;
        break;
    case VARTYPE_BOOL:
    case VARTYPE_INT:
        break;
    case VARTYPE_UNINITIALISED:
        intvalue = false;
        break;
    }
    return intvalue;
}

Variant::operator char *()
{
    string str = *this;
    return (char *) stringvalue.c_str();
}

Variant::operator const char *()
{
    string str = *this;
    return stringvalue.c_str();
}

Variant::operator void *()
{
    if (internaltype == VARTYPE_OBJREF) {
        return (void *) intvalue;
    }
    return NULL;
}

bool Variant::operator==(Variant value)
{
    if (value.GetType() == VARTYPE_UNINITIALISED) {
        return (internaltype == VARTYPE_UNINITIALISED);
    }
    switch (internaltype) {
    case VARTYPE_OBJREF:{
            int intval = value;
            if (value.GetType() == VARTYPE_OBJREF)
                return intvalue == intval;
            else
                return false;
        }
    case VARTYPE_STRING:{
            string strval = value;
            return (stringvalue == strval);
        }
    case VARTYPE_BOOL:{
            bool boolval = value;
            return (intvalue == boolval);
        }
    case VARTYPE_INT:{
            int intval = value;
            return (intvalue == intval);
        }
    case VARTYPE_ULONG:{
            unsigned long uval = value;
            return (ulongvalue == uval);
        }
    case VARTYPE_DOUBLE:{
            double dblval = value;
            return (dblvalue == dblval);
        }
    case VARTYPE_UNINITIALISED:
        return (value.GetType() == VARTYPE_UNINITIALISED);
    default:
        return false;
    }
}

bool Variant::operator==(std::string value)
{
    switch (internaltype) {
    case VARTYPE_STRING:
        return (stringvalue == value);
    default:
        return false;
    }
}

bool Variant::operator==(const char *value)
{
    string str;
    str = value;
    return operator==(str);
}

bool Variant::operator==(int value)
{
    switch (internaltype) {
    case VARTYPE_INT:
        return (intvalue == value);
    default:
        return false;
    }
}

bool Variant::operator==(unsigned long value)
{
    switch (internaltype) {
    case VARTYPE_ULONG:
        return (ulongvalue == value);
    default:
        return false;
    }
}

bool Variant::operator==(double value)
{
    switch (internaltype) {
    case VARTYPE_DOUBLE:
        return (dblvalue == value);
    default:
        return false;
    }
}

bool Variant::operator==(bool value)
{
    switch (internaltype) {
    case VARTYPE_BOOL:
        return (intvalue == value);
    default:
        return false;
    }
}

ostream & operator<<(ostream & stream, Variant v)
{
    stream << v;
    return stream;
}

const char *Variant::c_str(void)
{
    string str = *this;
    return stringvalue.c_str();
}

Variant Variant::operator+(Variant value)
{
    Variant ret;
    if (value.GetType() == VARTYPE_STRING || this->GetType() == VARTYPE_STRING) {
        string s;
        s += (const char *) *this;
        s += (const char *) value;
        ret = s;
    } else
        ret = ((int) *this + (int) value);
    return ret;
}

unsigned int colorToInt(SDL_Color c)
{
    unsigned int cval = 0xFF000000;

    cval = cval | (c.r * 0x10000);
    cval = cval | (c.g * 0x100);
    cval = cval | (c.b);

    return cval;
}

MethodHandler::MethodHandler(const char *name, int numargs)
{
    this->name = strdup(name);
    this->numargs = numargs;
}

MethodHandler::~MethodHandler(void)
{
    free(name);
}

const char *MethodHandler::GetName(void)
{
    return name;
}

EventHandler::EventHandler(const char *name, Script * script): Object(name)
{
    this->script = script;
}

EventHandler::~EventHandler(void)
{
    delete script;
}

bool EventHandler::IsEqual(const char *str)
{
    return matchString(str, this->name) == 0;
}

int EventHandler::Fire(void)
{
    script->Execute();
    systemObject->Log(9, "Script %s executed with result: %s: %s", name, script->GetResult(), script->GetScript());
    if (!strcmp(script->GetResult(), "lmbox_error")) {
        return -1;
    } else
    if (!strcmp(script->GetResult(), "lmbox_ignore")) {
        return 0;
    }
    return 1;
}

const char *EventHandler::GetScript(void)
{
    return script->GetScript();
}

ObjectProperty::ObjectProperty(const char *name)
{
    this->name = strdup(name);
    this->required = false;
    this->value = 0;
}

ObjectProperty::~ObjectProperty(void)
{
    free(name);
    lmbox_free(value);
}

const char *ObjectProperty::GetName(void)
{
    return name;
}

const char *ObjectProperty::GetString(void)
{
    return value;
}

int ObjectProperty::SetValue(const char *value)
{
    lmbox_free(this->value);
    this->value = lmbox_strdup(value);
    return 0;
}

bool ObjectProperty::IsRequired(void)
{
    return required;
}

void ObjectProperty::SetRequired(bool required)
{
    this->required = required;
}

bool ObjectProperty::IsReadOnly(void)
{
    return false;
}

int ObjectProperty::SetValue(Variant value)
{
    lmbox_free(this->value);
    this->value = lmbox_strdup(value.c_str());
    return 0;
}

Variant ObjectProperty::GetValue(void)
{
    return anytovariant(value ? value : "");
}

KeyBinding::KeyBinding(void)
{
    mod = key = 0;
    action = 0;
}

KeyBinding::~KeyBinding(void)
{
    lmbox_free(action);
}

FileEntry::FileEntry(char *path, time_t s, time_t d, const char *skip)
{
    file = path;
    start_time = s;
    duration = d;
    ParseSkip(skip);
}

FileEntry::FileEntry(const char *path, time_t s, time_t d, const char *skip)
{
    file = path;
    start_time = s;
    duration = d;
    ParseSkip(skip);
}

char *FileEntry::name(void)
{
    return (char*)file.c_str();
}

void FileEntry::ParseSkip(const char *skip)
{
    vector<char*> items;
    PositionEntry entry;

    strSplit(&items, skip, ", ");
    for (int i = 0;i < items.size(); i += 2) {
        entry.start_time = atoi(items[i]);
        entry.duration = atoi(items[i+1]);
        if (entry.start_time > 0 && entry.duration > 0) {
	    skip_list.push_back(entry);
        }
    }
    while (items.size() > 0) {
        lmbox_free(items.back());
        items.pop_back();
    }
}

