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

#define MWM_HINTS_FUNCTIONS                       (1L << 0)
#define MWM_HINTS_DECORATIONS                     (1L << 1)
#define MWM_HINTS_INPUT_MODE                      (1L << 2)
#define MWM_HINTS_STATUS                          (1L << 3)

#define MWM_FUNC_ALL                              (1L << 0)
#define MWM_FUNC_RESIZE                           (1L << 1)
#define MWM_FUNC_MOVE                             (1L << 2)
#define MWM_FUNC_MINIMIZE                         (1L << 3)
#define MWM_FUNC_MAXIMIZE                         (1L << 4)
#define MWM_FUNC_CLOSE                            (1L << 5)

#define MWM_DECOR_ALL                             (1L << 0)
#define MWM_DECOR_BORDER                          (1L << 1)
#define MWM_DECOR_RESIZEH                         (1L << 2)
#define MWM_DECOR_TITLE                           (1L << 3)
#define MWM_DECOR_MENU                            (1L << 4)
#define MWM_DECOR_MINIMIZE                        (1L << 5)
#define MWM_DECOR_MAXIMIZE                        (1L << 6)

#define MWM_INPUT_MODELESS                        0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL       1
#define MWM_INPUT_SYSTEM_MODAL                    2
#define MWM_INPUT_FULL_APPLICATION_MODAL          3
#define MWM_INPUT_APPLICATION_MODAL               MWM_INPUT_PRIMARY_APPLICATION_MODAL

#define MWM_TEAROFF_WINDOW	                  (1L<<0)

typedef struct MotifWmHints {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
};

static void db_error(const DB_ENV *env, const char *errpfx, const char *msg)
{
     systemObject->Log(0, "lmbox_db: %s: %s", errpfx, msg);
}

#ifdef HAVE_LIRC
// LIRC events
static void *lirc_thread(void *arg)
{
    int rc = 0;
    int shutdown = 0;
    char *code, *action;
    struct lirc_config *config;

    if (lirc_readconfig(0, &config, 0)) {
        return 0;
    }
    while (!lirc_nextcode(&code)) {
        if (!code) {
            continue;
        }
        while (!(rc = lirc_code2char(config, code, &action)) && action) {
            systemObject->PostEvent(action, 0, 0);
            systemObject->Log(5, "lirc_thread: %s", action);
            // Special case of repeated shutdown event, after 3 times we shout exit
            // the application, it could hang
            if (!strcmp(action, "shutdown")) {
                if (++shutdown >= 3) {
                    systemObject->Quit();
                    systemObject->Shutdown();
                    exit(0);
                }
            } else {
                shutdown = 0;
            }
        }
        free(code);
        if (rc == -1) {
            break;
        }
    }
    lirc_freeconfig(config);
    systemObject->Log(0, "LIRC thread exited: %d", rc);
    return 0;
}
#endif

SystemObject::SystemObject(const char *aname): DynamicObject(aname)
{
    this->loglevel = 0;
    this->bootTime = time(0);
    this->type = TYPE_SYSTEM;
    this->joystick = 0;
    this->xscale = this->yscale = 1;
    this->sdlevents = true;
    this->screen = 0;
    this->db_env = 0;
    this->cwd = 0;
    this->quit_flag = 0;
    this->shutdown_flag = 0;
    this->running_flag = 0;
    this->appname = strdup("lmbox");
    this->newpage = 0;
    this->sdlopts = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWACCEL;
    this->sdlflags = SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE|SDL_INIT_VIDEO;
    this->homedir = lmbox_printf("%s/.lmbox", xine_get_homedir());
    memset(&wm, 0, sizeof(wm));
    pthread_mutex_init(&configmutex, 0);
    pthread_mutex_init(&cachemutex, 0);
    pthread_mutex_init(&eventmutex, 0);
    pthread_mutex_init(&switchmutex, 0);

    Tcl_InitHashTable(&cachetable, TCL_STRING_KEYS);
    Tcl_InitHashTable(&configtable, TCL_STRING_KEYS);

    MemberFunctionProperty < SystemObject > *mp;
    mp = new MemberFunctionProperty < SystemObject > ("screenwidth", this, &SystemObject::pget_ScreenWidth, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("screenheight", this, &SystemObject::pget_ScreenHeight, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("currentfile", this, &SystemObject::pget_CurrentFile, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("version", this, &SystemObject::pget_Version, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("pagecount", this, &SystemObject::pget_PageCount, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("boottime", this, &SystemObject::pget_BootTime, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("idletime", this, &SystemObject::pget_IdleTime, &SystemObject::pset_IdleTime, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("focus", this, &SystemObject::pget_Focus, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("xscale", this, &SystemObject::pget_XScale, &SystemObject::pset_XScale, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("yscale", this, &SystemObject::pget_YScale, &SystemObject::pset_YScale, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("fontcache", this, &SystemObject::pget_FontCache, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("imagecache", this, &SystemObject::pget_ImageCache, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("loglevel", this, &SystemObject::pget_LogLevel, &SystemObject::pset_LogLevel, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < SystemObject > ("threadid", this, &SystemObject::pget_ThreadID, NULL, false);
    AddProperty(mp);

    MemberMethodHandler < SystemObject > *mh;
    mh = new MemberMethodHandler < SystemObject > ("quit", this, 0, &SystemObject::m_Quit);
    AddMethod(mh);
    mh = new MemberMethodHandler < SystemObject > ("focusnext", this, 0, &SystemObject::m_FocusNext);
    AddMethod(mh);
    mh = new MemberMethodHandler < SystemObject > ("focusprevious", this, 0, &SystemObject::m_FocusPrevious);
    AddMethod(mh);
    mh = new MemberMethodHandler < SystemObject > ("selectfocused", this, 0, &SystemObject::m_SelectFocused);
    AddMethod(mh);
    mh = new MemberMethodHandler < SystemObject > ("performaction", this, 1, &SystemObject::m_HandleEvent);
    AddMethod(mh);
    mh = new MemberMethodHandler < SystemObject > ("gotopage", this, 1, &SystemObject::m_GotoPage);
    AddMethod(mh);
    mh = new MemberMethodHandler < SystemObject > ("refresh", this, 0, &SystemObject::m_Refresh);
    AddMethod(mh);
}

SystemObject::~SystemObject(void)
{
    lmbox_free(cwd);
    lmbox_free(newpage);
    lmbox_free(appname);
    lmbox_free(homedir);
    Tcl_DeleteHashTable(&cachetable);
    Tcl_DeleteHashTable(&configtable);
}

int SystemObject::Init(void)
{
    threadID = (unsigned long)pthread_self();
    setbuf(stderr, NULL);
    setbuf(stdout, NULL);
    // Parse font and image path
    char *s, *e, *path = s = strdup(configGetValue("fontpath", "fonts"));
    while (s) {
        if ((e = strchr(s, ':'))) {
            *e++ = 0;
        }
        if (*s) {
            fontpath.push_back(strdup(s));
        }
        s = e;
    }
    free(path);
    path = s = strdup(configGetValue("imagepath", "images:modules"));
    while (s) {
        if ((e = strchr(s, ':'))) {
            *e++ = 0;
        }
        if (*s) {
            imagepath.push_back(strdup(s));
        }
        s = e;
    }
    free(path);

    // Add default key bindgins
    if (!keybindings.size()) {
        KeyBind("escape", "quit");
        KeyBind("enter", "accept");
        KeyBind("backspace", "cancel");
        KeyBind("shift+tab", "prev");
        KeyBind("tab", "next");
        KeyBind("leftarrow", "prev");
        KeyBind("rightarrow", "next");
        KeyBind("uparrow", "up");
        KeyBind("downarrow", "down");
        KeyBind("pageup", "prevpage");
        KeyBind("pagedown", "nextpage");
        KeyBind("home", "first");
        KeyBind("end", "last");
        KeyBind("joystickup", "up");
        KeyBind("joystickdown", "down");
        KeyBind("joystickleft", "prev");
        KeyBind("joystickright", "next");
        KeyBind("f1", "help");
        KeyBind("f9", "config");
    }
    SetLogLevel(configGetInt("loglevel", 0));
    // Add default objects
    char objname[32];
    AddObject(new TVObject("tv"));
    for (int i = 1; i < configGetInt("tv_objects", 1); i++) {
        sprintf(objname, "tv%d", i);
        AddObject(new TVObject(objname));
    }
    AddObject(new RadioObject("radio"));
    for (int i = 1; i < configGetInt("radio_objects", 1); i++) {
        sprintf(objname, "radio%d", i);
        AddObject(new RadioObject(objname));
    }
#ifdef HAVE_XINE
    AddObject(new XinePlayerObject("xineplayer"));
    for (int i = 1; i < configGetInt("player_objects", 5); i++) {
        sprintf(objname, "xineplayer%d", i);
        AddObject(new XinePlayerObject(objname));
    }
#endif
#ifdef HAVE_VLC
    AddObject(new VLCPlayerObject("vlcplayer"));
    for (int i = 1; i < configGetInt("player_objects", 5); i++) {
        sprintf(objname, "vlcplayer%d", i);
        AddObject(new VLCPlayerObject(objname));
    }
#endif
#ifdef HAVE_MIXER
    AddObject(new SoundObject("sound"));
#endif
#ifdef HAVE_MOZILLA
    AddObject(new WebBrowserObject("webbrowser"));
#endif

    // Do we need video/GUI part
    if (configGetInt("nogui", 0)) {
        return 0;
    }

    if (!XInitThreads()) {
        Log(0, "XInitThreads failed");
        return 0;
    }
    // Setup scaling before initialization
    SetXScale(configGetDouble("xscale", 1));
    SetYScale(configGetDouble("yscale", 1));

    // Initialize SDL
    if (SDL_Init(sdlflags) < 0) {
        Log(0, "Failed to initialise SDL library");
        return 0;
    }
    // Initialize the TTF library
    if (TTF_Init() < 0) {
        Log(0, "Failed to initialise TTF font library");
        SDL_Quit();
        return 0;
    }
    // Initialize joystick support
    if (!configGetInt("nojoystick", 1)) {
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0 || !(joystick = SDL_JoystickOpen(0))) {
            Log(0, "Failed to initialise SDL joystick subsystem: %d", SDL_GetError());
            configSetValue("nojoystick", "1");
        } else {
            SDL_JoystickEventState(SDL_ENABLE);
            sdlflags |= SDL_INIT_JOYSTICK;
        }
    }
    // Initialize the GUI library
    SDL_EnableUNICODE(1);
    // Create and install the screen (this will attempt to change to the
    // specified video mode)
    int w = configGetInt("width", 640);
    int h = configGetInt("height", 480);
    int d = configGetInt("depth", 24);
    // Save dimensions for further access
    configSetInt("width", w);
    configSetInt("height", h);
    configSetInt("depth", d);

    if (configGetInt("fullscreen", 0)) {
        sdlopts |= SDL_FULLSCREEN;
    }
    if (configGetInt("noframe", 1)) {
        sdlopts |= SDL_NOFRAME;
    }
    d = SDL_VideoModeOK(w, h, d, sdlopts);
    if (d) {
       SDL_Surface *temp = SDL_SetVideoMode(w, h, d, sdlopts);
       if (temp) {
           screen = new GUI_Screen("screen", temp);
       }
    }
    if (screen == NULL) {
        Log(0, "Couldn't initialise SDL video mode %dx%dx%d", w, h, d);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    // Show initial background image
    GUI_Surface *image = LoadImage(configGetValue("bgimage", 0), true);
    if (image) {
        screen->SetBackgroundStyle(BACKGROUND_SCALED);
        screen->SetBackground(image);
        screen->DoUpdate(0);
    }
    if (!configGetInt("nokeyrepeat", 0)) {
        // Set keyboard repeat
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    }
    if (configGetInt("hidecursor", 0)) {
        SDL_ShowCursor(0);
        SDL_WarpMouse(0, 0);
    }
#ifdef HAVE_LIRC
    // LIRC initialization
    if (!configGetInt("nolirc", 0)) {
        if (lirc_init("lmbox", 1) == -1 || threadCreate(lirc_thread, 0)) {
            lirc_deinit();
            configSetValue("nolirc", "1");
            Log(0, "Failed to initialize LIRC");
        }
    }
#endif
    // X11 display info
    SDL_GetWMInfo(&wm);
    // Set window caption
    SDL_WM_SetCaption("lmbox", "lmbox");
    // Ask for no borders
    LockDisplay();
    SetWindowBorder(GetDisplay(), GetWindow(), 0);
    UnlockDisplay();
    SetPointer(0, 0);
#ifdef HAVE_MOZILLA
    gtk_init(0, 0);
#endif
    return 1;
}

void SystemObject::Shutdown(void)
{
    shutdown_flag = 1;
    dbClose();
#ifdef HAVE_LIRC
    lirc_deinit();
#endif
    SDL_ShowCursor(1);
    if (joystick) {
        SDL_JoystickClose(joystick);
        joystick = 0;
    }
    while (externaleventhandlers.size() > 0) {
        externaleventhandlers.pop_back();
    }
    if (screen) {
        screen->DecRef();
        screen = 0;
    }
    while (keybindings.size() > 0) {
        delete keybindings.back();
        keybindings.pop_back();
    }
    while (fontpath.size() > 0) {
        free(fontpath.back());
        fontpath.pop_back();
    }
    while (imagepath.size() > 0) {
        free(imagepath.back());
        imagepath.pop_back();
    }
    while (images.size() > 0) {
        if (images.back()->RefCount() != 1) {
            Log(0, "%s: freee with refcount %d", images.back()->GetName(), images.back()->RefCount());
        }
        images.back()->DecRef();
        images.pop_back();
    }
    while (fonts.size() > 0) {
        fonts.back()->DecRef();
        fonts.pop_back();
    }
    DestroyObjects();
    varUnset("*");
    configUnset("*");
}

void SystemObject::SetAppName(const char *app)
{
    lmbox_free(appname);
    appname = lmbox_strdup(app);
}

const char *SystemObject::GetAppName(void)
{
    return appname;
}

int SystemObject::configGetInt(const char *name, int def)
{
    const char *val = configGetValue(name, "NULL");
    if (!strcmp(val, "NULL")) {
        return def;
    }
    return atoi(val);
}

double SystemObject::configGetDouble(const char *name, double def)
{
    const char *val = configGetValue(name, "NULL");
    if (!strcmp(val, "NULL")) {
        return def;
    }
    return atof(val);
}

const char *SystemObject::configGetValue(const char *name, const char *def)
{
    Tcl_HashEntry *entry;
    const char *value = def;
    pthread_mutex_lock(&configmutex);
    if ((entry = Tcl_FindHashEntry(&configtable, name))) {
        value = (const char *) Tcl_GetHashValue(entry);
    }
    pthread_mutex_unlock(&configmutex);
    return value ? value : "";
}

void SystemObject::configSetInt(const char *name, int value)
{
    char buf[32];
    sprintf(buf, "%i", value);
    configSetValue(name, buf);
}

void SystemObject::configSetValue(const char *name, const char *value)
{
    int rc;
    Tcl_HashEntry *entry;
    pthread_mutex_lock(&configmutex);
    entry = Tcl_CreateHashEntry(&configtable, name, &rc);
    if (!rc) {
        free(Tcl_GetHashValue(entry));
    }
    Tcl_SetHashValue(entry, strdup(value));
    pthread_mutex_unlock(&configmutex);
}

void SystemObject::configUnset(const char *mask)
{
    char *key;
    Tcl_HashEntry *entry;
    Tcl_HashSearch search;
    pthread_mutex_lock(&configmutex);
    entry = Tcl_FirstHashEntry(&configtable, &search);
    while (entry) {
        key = Tcl_GetHashKey(&configtable, entry);
        if (Tcl_StringMatch(key, mask)) {
            free(Tcl_GetHashValue(entry));
            Tcl_DeleteHashEntry(entry);
        }
        entry = Tcl_NextHashEntry(&search);
    }
    pthread_mutex_unlock(&configmutex);
}

void SystemObject::configList(Tcl_Interp * interp, const char *mask)
{
    char *key;
    Tcl_HashEntry *entry;
    Tcl_HashSearch search;
    pthread_mutex_lock(&configmutex);
    entry = Tcl_FirstHashEntry(&configtable, &search);
    while (entry) {
        key = Tcl_GetHashKey(&configtable, entry);
        if (!mask || Tcl_StringMatch(key, mask)) {
            Tcl_AppendElement(interp, Tcl_GetHashKey(&configtable, entry));
            Tcl_AppendElement(interp, (const char *) Tcl_GetHashValue(entry));
        }
        entry = Tcl_NextHashEntry(&search);
    }
    pthread_mutex_unlock(&configmutex);
}

const char *SystemObject::varGet(const char *name)
{
    char *rc = 0;
    Tcl_HashEntry *entry;
    pthread_mutex_lock(&cachemutex);
    if ((entry = Tcl_FindHashEntry(&cachetable, name))) {
        rc = (char *) Tcl_GetHashValue(entry);
    }
    pthread_mutex_unlock(&cachemutex);
    return rc;
}

void SystemObject::varSet(const char *name, const char *value)
{
    int rc;
    Tcl_HashEntry *entry;
    pthread_mutex_lock(&cachemutex);
    entry = Tcl_CreateHashEntry(&cachetable, name, &rc);
    if (!rc) {
        free(Tcl_GetHashValue(entry));
    }
    Tcl_SetHashValue(entry, strdup(value));
    pthread_mutex_unlock(&cachemutex);
}

int SystemObject::varIncr(const char *name)
{
    int rc, val = 0;
    Tcl_HashEntry *entry;
    pthread_mutex_lock(&cachemutex);
    entry = Tcl_CreateHashEntry(&cachetable, name, &rc);
    if (!rc) {
        val = atoi((const char*)Tcl_GetHashValue(entry));
        free(Tcl_GetHashValue(entry));
    }
    Tcl_SetHashValue(entry, lmbox_printf("%d", ++val));
    pthread_mutex_unlock(&cachemutex);
    return val;
}

void SystemObject::varAppend(const char *name, const char *value)
{
    int rc;
    Tcl_HashEntry *entry;
    pthread_mutex_lock(&cachemutex);
    entry = Tcl_CreateHashEntry(&cachetable, name, &rc);
    if (!rc) {
        value = lmbox_printf("%s%s", Tcl_GetHashValue(entry), value);
        free(Tcl_GetHashValue(entry));
    }
    Tcl_SetHashValue(entry, !rc ? value : strdup(value));
    pthread_mutex_unlock(&cachemutex);
}

void SystemObject::varUnset(const char *mask)
{
    char *key;
    Tcl_HashEntry *entry;
    Tcl_HashSearch search;
    pthread_mutex_lock(&cachemutex);
    entry = Tcl_FirstHashEntry(&cachetable, &search);
    while (entry) {
        key = Tcl_GetHashKey(&cachetable, entry);
        if (Tcl_StringMatch(key, mask)) {
            free(Tcl_GetHashValue(entry));
            Tcl_DeleteHashEntry(entry);
        }
        entry = Tcl_NextHashEntry(&search);
    }
    pthread_mutex_unlock(&cachemutex);
}

void SystemObject::varList(Tcl_Interp * interp, const char *mask)
{
    char *key;
    Tcl_HashEntry *entry;
    Tcl_HashSearch search;
    pthread_mutex_lock(&cachemutex);
    entry = Tcl_FirstHashEntry(&cachetable, &search);
    while (entry) {
        key = Tcl_GetHashKey(&cachetable, entry);
        if (!mask || Tcl_StringMatch(key, mask)) {
            Tcl_AppendElement(interp, key);
            Tcl_AppendElement(interp, (const char *) Tcl_GetHashValue(entry));
        }
        entry = Tcl_NextHashEntry(&search);
    }
    pthread_mutex_unlock(&cachemutex);
}

void SystemObject::Run(void)
{
    int key;
    SDL_Event sevent;
    bool processed = false;
    bool disablelirc = configGetInt("disablelirc", 0);
    bool hidecursor = configGetInt("hidecursor", 1);

    quit_flag = false;
    running_flag = true;
    while (!quit_flag) {
        processed = false;
        while (IsEvents() && SDL_PollEvent(&sevent)) {
            processed = false;
            SetIdleTime(0);
            switch (sevent.type) {
            case SDL_ACTIVEEVENT:
            case SDL_VIDEORESIZE:
            case SDL_VIDEOEXPOSE:
                OnExpose();
                break;

            case SDL_KEYDOWN:
                processed = HandleKeyEvent(&sevent);
                break;

            case SDL_JOYBUTTONDOWN:
                if (sevent.jbutton.button > 8) {
                    break;
                }
                switch (sevent.jbutton.button) {
                case 0:
                    key = SDLK_F1;
                    break;
                case 1:
                    key = SDLK_F2;
                    break;
                case 2:
                    key = SDLK_F3;
                    break;
                case 3:
                    key = SDLK_F4;
                    break;
                case 4:
                    key = SDLK_F5;
                    break;
                case 5:
                    key = SDLK_F6;
                    break;
                case 6:
                    key = SDLK_F7;
                    break;
                case 7:
                    key = SDLK_F8;
                    break;
                case 8:
                    key = SDLK_F9;
                    break;
                default:
                    key = SDLK_BACKSPACE;
                }
                if (key == SDLK_BACKSPACE) {
                    break;
                }
                for (unsigned int i = 0; i < keybindings.size(); i++) {
                    if (key != keybindings[i]->key) {
                        continue;
                    }
                    PostEvent(keybindings[i]->action, NULL, NULL);
                    processed = true;
                    break;
                }
                break;

            case SDL_JOYAXISMOTION:
                key = SDLK_BACKSPACE;
                if (sevent.jaxis.axis == 0) {
                    if (sevent.jaxis.value > 10)
                        key = SDL_HAT_RIGHT;
                    else
                    if (sevent.jaxis.value < -10)
                        key = SDL_HAT_LEFT;
                } else
                if (sevent.jaxis.axis == 1) {
                    if (sevent.jaxis.value > 10)
                        key = SDL_HAT_DOWN;
                    else
                    if (sevent.jaxis.value < -10)
                        key = SDL_HAT_UP;
                }
                if (key == SDLK_BACKSPACE) {
                    break;
                }
                for (unsigned int i = 0; i < keybindings.size(); i++) {
                    if (key != keybindings[i]->key) {
                        continue;
                    }
                    PostEvent(keybindings[i]->action, NULL, NULL);
                    processed = true;
                    break;
                }
                break;

            case SDL_JOYHATMOTION:
                key = sevent.jhat.value;
                if (key == SDLK_BACKSPACE) {
                    break;
                }
                for (unsigned int i = 0; i < keybindings.size(); i++) {
                    if (key != keybindings[i]->key) {
                        continue;
                    }
                    PostEvent(keybindings[i]->action, NULL, NULL);
                    processed = true;
                    break;
                }
                break;
            }
            if (quit_flag) {
                break;
            }
            if (!processed) {
                processed = HandleMouseEvent(&sevent, 0, 0);
            }
        }
        // lmbox events from the queue
        while (true) {
            pthread_mutex_lock(&eventmutex);
            event.type = "";
            if (eventqueue.size() > 0) {
                event = eventqueue.front();
                eventqueue.pop();
            }
            pthread_mutex_unlock(&eventmutex);
            if (event.type == "") {
                break;
            }
            Log(3, "Run: system: event: %s %s %s", event.type.c_str(), event.data1.c_str(), event.data2.c_str());
            // Ignore events if busy
            if (GetFlags() & OBJECT_BUSY) {
                Log(3, "SystemObjectRun: ignore event: %s", event.type.c_str());
                continue;
            }
            SetIdleTime(0);
            if (event.type == "quit") {
                Quit();
            } else
            if (event.type == "expose") {
                OnExpose();
            } else
            if (event.type == "refresh") {
                DoUpdate(1);
            } else
            if (event.type == "delay") {
                SDL_Delay(atoi(event.data1.c_str()));
            } else
            if (event.type == "fire") {
                FireObject(event.data1.c_str(), event.data2.c_str());
            } else
            if (event.type == "switchpage") {
                SwitchPage(event.data1.c_str());
            } else
            if (event.type == "showoverlay") {
                ShowOverlay(event.data1.c_str());
            } else
            if (!screen->HandleEvent(event.type.c_str())) {
                HandleEvent(event.type.c_str());
            }
        }
        if (quit_flag) {
            break;
        }
        // Run event handlers from external systems (gtk, gdk)
        for (int i = 0; i < externaleventhandlers.size(); i++) {
            externaleventhandlers[i]->OnEvent();
        }
        DoUpdate(0);
        SDL_Delay(10);
    }
}

bool SystemObject::HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset)
{
    Lock();
    if (screen->HandleMouseEvent(event, xoffset, yoffset)) {
        Unlock();
        return 1;
    }
    for (int i = 0; i < containedobjects.size(); i++) {
        if (containedobjects[i]->HandleMouseEvent(event, xoffset, yoffset)) {
            Unlock();
            return 1;
        }
    }
    Unlock();
    // Not processed, post it as action event
    char x[12], y[12];
    switch (event->type) {
     case SDL_MOUSEBUTTONDOWN:
        sprintf(x, "%d", event->button.x - xoffset);
        sprintf(y, "%d", event->button.y - yoffset);
        PostEvent(tlsSprintf(0, "button%d", event->button.button), x, y);
        break;
    }
    return 0;
}

bool SystemObject::HandleKeyEvent(const SDL_Event * event)
{
    Lock();
    if (screen->HandleKeyEvent(event)) {
        Unlock();
        return 1;
    }
    Unlock();
    for (int i = 0; i < keybindings.size(); i++) {
        if (event->key.keysym.sym != keybindings[i]->key) {
            continue;
        }
        if (!checkModKeys(event->key.keysym.mod, keybindings[i]->mod)) {
            continue;
        }
        PostEvent(keybindings[i]->action, NULL, NULL);
        return true;
    }
    // Send key symbol as an action
    PostEvent(keyToName(event->key.keysym.sym).c_str(), NULL, NULL);
    return 0;
}

void SystemObject::PostEvent(const char *type, const char *data1, const char *data2)
{
    SetIdleTime(0);
    pthread_mutex_lock(&eventmutex);
    EventEntry e;
    e.type = type;
    e.data1 = data1 ? data1 : "";
    e.data2 = data2 ? data2 : "";
    eventqueue.push(e);
    pthread_mutex_unlock(&eventmutex);
}

void SystemObject::DoUpdate(int force)
{
    if (freeze || !screen) {
        return;
    }
    screen->DoUpdate(force);
}

void SystemObject::OnExpose(void)
{
    Lock();
    DynamicObject::OnExpose();
    for (int i = 0; i < containedobjects.size(); i++) {
        containedobjects[i]->OnExpose();
    }
    for (int i = 0; i < externaleventhandlers.size(); i++) {
        externaleventhandlers[i]->OnExpose();
    }
    screen->OnExpose();
    Unlock();
}

void SystemObject::Flip(void)
{
    if (freeze || !screen) {
        return;
    }
    screen->Flip();
}

int SystemObject::FireObject(const char *name, const char *event)
{
    int rc = 0;
    DynamicObject *obj = FindObject(name);
    if (obj) {
        rc = obj->FireEvent(event);
        obj->DecRef();
    }
    return rc;
}

void SystemObject::WaitForObject(const char *name, int timeout)
{
    unsigned int now = SDL_GetTicks();
    DynamicObject *obj = FindObject(name);
    if (timeout <= 0) {
        timeout = 10000;
    }
    while (obj && obj->IsBusy()) {
        SDL_Delay(100);
        if (timeout && SDL_GetTicks() - now > timeout) {
            Log(1, "WaitForObject: timeout %d, waiting for %s", timeout, name);
            break;
        }
    }
    if (obj) {
        obj->DecRef();
    }
}

int SystemObject::GetLogLevel(void)
{
    return loglevel;
}

void SystemObject::SetLogLevel(int level)
{
    loglevel = level;
}

void SystemObject::Log(int level, const char *fmt, ...)
{
    if (loglevel < level) {
        return;
    }
    struct tm lt;
    char timebuf[64];
    time_t now = time(0);

    localtime_r(&now, &lt);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %T: ", &lt);
    fprintf(stderr, timebuf);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void SystemObject::Quit(void)
{
    quit_flag = true;
}

bool SystemObject::IsQuit(void)
{
    return quit_flag;
}

bool SystemObject::IsShutdown(void)
{
    return shutdown_flag;
}

bool SystemObject::IsRunning(void)
{
    return running_flag;
}

void SystemObject::AddPage(GUI_Container * page)
{
    screen->AddWidget(page);
}

int SystemObject::SwitchPage(const char *pagename)
{
    GUI_Page *page = (GUI_Page*)screen->FindWidget(pagename);
    int rc = SwitchPage(page);
    return rc;
}

int SystemObject::SwitchPage(GUI_Page * page)
{
    if (IsQuit()) {
        return -1;
    }
    if (!page) {
        page = (GUI_Page*)screen->GetWidget(0);
    }
    if (!page) {
        Log(0, "Unable to find page to switch to");
        return -1;
    }
    GUI_Page *curpage = GetCurrentPage();
    // Same page, do nothing
    if (page == curpage) {
        if (curpage) {
            curpage->DecRef();
        }
        page->DecRef();
        return 0;
    }
    if (!page->GetWidth()) {
        page->SetWidth(screen->GetWidth());
    }
    if (!page->GetHeight()) {
        page->SetHeight(screen->GetHeight());
    }
    Log(3, "SwitchPage: %s: newpage: %s", (curpage ? curpage->GetName() : ""), page->GetName());
    if (curpage) {
        // Keep prev page if old page's prev page is different than the new one
        if (strcmp(curpage->GetPrevPage(), curpage->GetName())) {
            page->SetPrevPage(curpage->GetName());
        }
        curpage->DecRef();
    }
    lmbox_free(newpage);
    newpage = lmbox_strdup(page->GetName());
    FireEvent("OnSwitchPage");
    screen->SetFocusWidget(screen->ContainsWidget(page));
    page->DecRef();
    return 0;
}

int SystemObject::ShowOverlay(const char *pagename)
{
    GUI_Page *page = 0;

    if (pagename) {
        page = (GUI_Page*)screen->FindWidget(pagename);
    }
    int rc = screen->ShowOverlay(page);
    if (page) {
        page->DecRef();
    }
    screen->SetChanged(1);
    return rc;
}

int SystemObject::Chdir(const char *path)
{
    chdir(path);
    if (!(cwd = getcwd(NULL, 0))) {
        Log(0, "Init: Chdir: %s", strerror(errno));
        return -1;
    }
    return 0;
}

char *SystemObject::GetCwd(void)
{
    return cwd;
}

char *SystemObject::GetHome(void)
{
    return homedir;
}

const char *SystemObject::GetPrevPage(void)
{
    GUI_Page *page = GetCurrentPage();
    if (page) {
        const char *rc = page->GetPrevPage();
        page->DecRef();
        return rc;
    }
    return 0;
}

const char *SystemObject::GetNewPage(void)
{
    return newpage;
}

GUI_Screen *SystemObject::GetScreen(void)
{
    return screen;
}

GUI_Page *SystemObject::GetCurrentPage(void)
{
    return screen ? (GUI_Page*)screen->GetFocusWidget() : 0;
}

DynamicObject *SystemObject::FindObject(const char *name)
{
    if (!name || !*name) {
        return 0;
    }
    if (!strcmp(name, "system")) {
        IncRef();
        return this;
    }
    if (!strcmp(name, "this")) {
        Tls *tls = tlsGet();
        if (tls->object) {
            tls->object->IncRef();
            return tls->object;
        }
    }
    string namebuf;
    DynamicObject *obj;
    if (!strncmp(name, "player", 6)) {
        namebuf = configGetValue("systemplayer", "xine");
        namebuf += name;
        name = namebuf.c_str();
    }
    obj = DynamicObject::FindObject(name);
    if (obj) {
        return obj;
    }
    Lock();
    GUI_Page *page = GetCurrentPage();
    if (page) {
        if (!strcmp(name, "currentpage")) {
            Unlock();
            return page;
        }
        if (!strcmp(name, "focusedwidget")) {
            obj = page->GetFocusWidget();
        } else {
            obj = page->FindObject(name);
        }
        page->DecRef();
        if (obj) {
            Unlock();
            return obj;
        }
    }
    if (screen) {
        page = (GUI_Page*)screen->FindWidget(name);
        if (page) {
            Unlock();
            return page;
        }
        GUI_Widget *widget;
        for (int i = 0; i < screen->GetWidgetCount(); i++) {
            if (i == screen->GetFocusIndex()) {
                continue;
            }
            if ((widget = screen->GetWidget(i))) {
                obj = widget->FindObject(name);
                widget->DecRef();
                if (obj) {
                    Unlock();
                    return obj;
                }
            }
        }
    }
    Unlock();
    return 0;
}

int SystemObject::GetPageCount(void)
{
    return screen ? screen->GetWidgetCount() : 0;
}

bool SystemObject::IsEvents(void)
{
    return this->sdlevents;
}

void SystemObject::SetEvents(bool flag)
{
    this->sdlevents = flag;
}

void SystemObject::SetFreeze(int nfreeze)
{
    this->freeze = nfreeze;
}

bool SystemObject::HasFocus(void)
{
    return (SDL_GetAppState() & SDL_APPINPUTFOCUS) != 0;
}

void SystemObject::SetPointer(int x, int y)
{
    LockDisplay();
    XWarpPointer(GetDisplay(), GetWindow(), GetWindow(), 0, 0, 0, 0, x, y);
    UnlockDisplay();
}

int SystemObject::IsWindowViewable(Display * display, Window window)
{
    Status status;
    XWindowAttributes attr;
    if (!display || window == None) {
        return -1;
    }
    status = XGetWindowAttributes(display, window, &attr);
    if (status != BadDrawable && status != BadWindow && attr.map_state == IsViewable) {
        return 1;
    }
    return 0;
}

void SystemObject::SetWindowBorder(Display * display, Window window, int border)
{
    MotifWmHints hints = { MWM_HINTS_DECORATIONS, border ? MWM_DECOR_BORDER : 0, 0, 0, 0 };
    Atom atom = XInternAtom(display, "_MOTIF_WM_HINTS", False);
    if (atom != None) {
        XChangeProperty(display, window, atom, atom, 32, PropModeReplace, (unsigned char *) &hints, 5);
    }
}

Window SystemObject::CreateWindow(Display *display, Visual *visual, int depth, int x, int y, int w, int h, int border)
{
    XSizeHints hint;
    XWindowChanges value;
    XSetWindowAttributes attr;

    XLockDisplay(display);
    attr.backing_store = Always;
    attr.override_redirect = True;
    attr.colormap = DefaultColormap(display, DefaultScreen(display));
    attr.background_pixel = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateWindow(display, GetWindow(), x, y, w, h, border, depth, InputOutput, visual, CWBackingStore|CWBackPixel|CWColormap|CWOverrideRedirect, &attr);
    if (window == None) {
        Log(0, "CreateWindow: unable to create window %dx%dx%dx%d", x, y, w, h);
        XUnlockDisplay(display);
        return None;
    }
    hint.x = x;
    hint.y = y;
    hint.width = w;
    hint.height = h;
    hint.flags = USSize | USPosition | PPosition | PSize;
    hint.win_gravity = StaticGravity;
    hint.flags = PPosition | PSize | PWinGravity;
    XSetNormalHints(display, window, &hint);
    XSetWMNormalHints(display, window, &hint);
    XSelectInput(display, window, ExposureMask);
    SetWindowBorder(display, window, border);
    value.stack_mode = Above;
    XConfigureWindow(display, window, CWStackMode, &value);
    XUnlockDisplay(display);
    return window;
}

// List all available TrueColor visuals, pick the best one for xine.
// We prefer visuals of depth 15/16 (fast).  Depth 24/32 may be OK,
// but could be slow. Borrowed from xine-ui
Visual *SystemObject::FindVisual(Display *display, int *depth)
{
    int vnum;
    Visual *visual = 0;
    XVisualInfo vtmpl;
    XVisualInfo *vinfo;
    XWindowAttributes attrs;

    XLockDisplay(display);
    *depth = 0;
    vtmpl.c_class = TrueColor;
    vtmpl.screen = DefaultScreen(display);
    vinfo = XGetVisualInfo(display, VisualScreenMask | VisualClassMask, &vtmpl, &vnum);
    if (vinfo) {
        int vpref = -1, vindex = -1;
        for (int i = 0; i < vnum; i++) {
            int pref = vinfo[i].depth == 15 || vinfo[i].depth == 16 ? 3 : vinfo[i].depth > 16 ? 2 : 1;
            if (pref <= vpref) {
                continue;
            }
            vpref = pref;
            vindex = i;
        }
        if (vindex != -1) {
            *depth = vinfo[vindex].depth;
            visual = vinfo[vindex].visual;
            systemObject->Log(9, "FindVisual: choosing visual 0x%x, depth %d bpp %d\n", vinfo[vindex].visualid, vinfo[vindex].depth, vinfo[vindex].bits_per_rgb);
        }
        XFree(vinfo);
    }
    if (*depth == 0) {
        XGetWindowAttributes(display, DefaultRootWindow(display), &attrs);
        *depth = attrs.depth;
        if (XMatchVisualInfo(display, DefaultScreen(display), *depth, TrueColor, &vtmpl)) {
            visual = vtmpl.visual;
            systemObject->Log(9, "FindVisual: using visual 0x%x, depth %d bpp %d\n", vtmpl.visualid, vtmpl.depth, vtmpl.bits_per_rgb);
        } else {
            *depth = DefaultDepth(display, DefaultScreen(display));
            visual = DefaultVisual(display, DefaultScreen(display));
            systemObject->Log(9, "FindVisual: couldn't find true color visual, using default with depth %d\n", *depth);
        }
    }
    XUnlockDisplay(display);
    return visual;
}

void SystemObject::LockDisplay()
{
    if (wm.info.x11.lock_func) {
        wm.info.x11.lock_func();
    }
}

void SystemObject::UnlockDisplay()
{
    if (wm.info.x11.unlock_func) {
        wm.info.x11.unlock_func();
    }
}

Display *SystemObject::GetDisplay()
{
    return wm.info.x11.display;
}

Window SystemObject::GetWindow()
{
    return wm.info.x11.window;
}

int SystemObject::GetScreenWidth(void)
{
    return screen ? screen->GetWidth() : configGetInt("width", 640);
}

int SystemObject::GetScreenHeight(void)
{
    return screen ? screen->GetHeight() : configGetInt("height", 480);
}

double SystemObject::GetXScale(void)
{
    return xscale;
}

void SystemObject::SetXScale(double scale)
{
    if ((xscale = scale) <= 0) {
        xscale = 1;
    }
}

double SystemObject::GetYScale(void)
{
    return yscale;
}

void SystemObject::SetYScale(double scale)
{
    if ((yscale = scale) <= 0) {
        yscale = 1;
    }
}

int SystemObject::cdromEject(void)
{
   int fd, rc = -1;

   if ((fd = open(configGetValue("dvd_device", "/dev/dvd"), O_RDONLY|O_NONBLOCK)) > -1) {
       rc = ioctl(fd, CDROMEJECT);
       close(fd);
   }
   return rc;
}

int SystemObject::cdromClose(void)
{
   int fd, rc = -1;

   if ((fd = open(configGetValue("dvd_device", "/dev/dvd"), O_RDONLY|O_NONBLOCK)) > -1) {
       rc = ioctl(fd, CDROMCLOSETRAY);
       close(fd);
   }
   return rc;
}

const char *SystemObject::cdromStatus(void)
{
   int fd;
   const char *status = "empty";

   if ((fd = open(configGetValue("dvd_device", "/dev/dvd"), O_RDONLY|O_NONBLOCK)) > -1) {
       switch (ioctl(fd, CDROM_DISC_STATUS, CDSL_CURRENT)) {
        case -1:
            Log(0, "GetCdromStatus: %s", strerror(errno));
            break;
        case CDS_NO_DISC:
        case CDS_NO_INFO:
            break;
        case CDS_AUDIO:
            status = "audio";
            break;
        case CDS_DATA_1:
            status = "data1";
            break;
        case CDS_DATA_2:
            status = "data2";
            break;
        case CDS_XA_2_1:
            status = "xa21";
            break;
        case CDS_XA_2_2:
            status = "xa22";
            break;
        case CDS_MIXED:
            status = "mixed";
            break;
        default:
            status = "unknown";
       }
       close(fd);
   }
   return status;
}

void SystemObject::SetIdleTime(int idle)
{
    eventTime = time(0) - idle;
}

int SystemObject::GetIdleTime(void)
{
    return time(0) - eventTime;
}

char *SystemObject::GetEvent(void)
{
    Tls *tls = tlsGet();
    pthread_mutex_lock(&eventmutex);
    char *rc = lmbox_strdup(tls->event ? tls->event : event.type.c_str());
    pthread_mutex_unlock(&eventmutex);
    return rc;
}

char *SystemObject::GetEventData(int mode)
{
    char *rc = 0;
    pthread_mutex_lock(&eventmutex);
    switch (mode) {
     case 0:
         rc = lmbox_printf("{%s} {%s}", event.data1.c_str(), event.data2.c_str());
         break;
     case 1:
         rc = lmbox_strdup(event.data1.c_str());
         break;
     case 2:
         rc = lmbox_strdup(event.data2.c_str());
         break;
    }
    pthread_mutex_unlock(&eventmutex);
    return rc;
}

unsigned long SystemObject::GetBootTime(void)
{
    return bootTime;
}

unsigned long SystemObject::GetThreadID(void)
{
    return threadID;
}

const char *SystemObject::KeyToAction(int key)
{
    for (int i = 0; i < keybindings.size(); i++) {
        if (keybindings[i]->key == key) {
            return keybindings[i]->action;
        }
    }
    return 0;
}

string SystemObject::KeyBindings()
{
    string rc;
    for (int i = 0; i < keybindings.size(); i++) {
        rc += keybindings[i]->action;
        rc += " ";
        rc += keyToName(keybindings[i]->key);
        rc += " {";
        rc += modToName(keybindings[i]->mod);
        rc += "} ";
    }
    return rc;
}

void SystemObject::KeyBind(const char *key, const char *action)
{
    KeyBinding *bind = new KeyBinding;
    // Parse modifiers
    char *buf, *s, *p;
    buf = s = strdup(key);
    p = strchr(s, '+');
    while (p) {
        *p++ = 0;
        bind->mod |= nameToMod(s);
        p = strchr((s = p), '+');
    }
    key = s;
    if ((bind->key = nameToKey(key)) != -1) {
        bind->action = strdup(action);
        keybindings.push_back(bind);
    } else {
        Log(0, "KeyBind: unknown key: %s: %s", key, action);
        delete bind;
    }
    free(buf);
}

char *SystemObject::FindFile(const char *filename)
{
    int count = 0;
    struct stat st;
    string fullpath = filename;

    // Try in all configured directories
    while (1) {
        if (!stat(fullpath.c_str(), &st)) {
            return strdup(fullpath.c_str());
        }
        if (count >= imagepath.size() || filename[0] == '/') {
            break;
        }
        fullpath = imagepath[count];
        fullpath += "/";
        fullpath += filename;
        count++;
    }
    return 0;
}

GUI_Font *SystemObject::LoadFont(string filename, int fontsize)
{
    if (filename == "") {
        return NULL;
    }

    int count = 0;
    struct stat st;
    GUI_Font *font = 0;
    string fullpath = filename;

    // Search in the cache first
    while (1) {
        Lock();
        for (int i = 0; i < fonts.size(); i++) {
            if (fullpath == fonts[i]->GetName() && fontsize == fontsizes[i]) {
                font = fonts[i];
                break;
            }
        }
        Unlock();
        if (font) {
            return font;
        }
        if (count < fontpath.size() && filename.c_str()[0] != '/') {
            fullpath = fontpath[count];
            fullpath += "/";
            fullpath += filename;
            count++;
            continue;
        }
        break;
    }
    count = 0;
    fullpath = filename;
    // Try in all configured directories
    while (1) {
        if (!stat(fullpath.c_str(), &st)) {
            font = new GUI_Font(fullpath.c_str(), fontsize);
        }
        if (font) {
            break;
        }
        if (count < fontpath.size() && filename.c_str()[0] != '/') {
            fullpath = fontpath[count];
            fullpath += "/";
            fullpath += filename;
            count++;
            continue;
        }
        Log(0, "Failed to load font %s", filename.c_str());
        return NULL;
    }
    Lock();
    fonts.push_back(font);
    fontsizes.push_back(fontsize);
    Log(2, "font loaded %s, size=%d", filename.c_str(), fontsize);
    Unlock();
    return font;
}

// This function does assume ownership of the image
GUI_Surface *SystemObject::LoadImage(string filename, bool cache)
{
    if (filename == "") {
        return NULL;
    }
    int count = 0;
    struct stat st;
    GUI_Surface *image = 0;
    string fullpath = filename;

    // Search in the cache first
    while (1) {
        Lock();
        for (int i = 0; i < images.size(); i++) {
            if (fullpath == images[i]->GetName()) {
                image = images[i];
                break;
            }
        }
        Unlock();
        if (image) {
            return image;
        }
        if (count < imagepath.size() && filename.c_str()[0] != '/') {
            fullpath = imagepath[count];
            fullpath += "/";
            fullpath += filename;
            count++;
            continue;
        }
        break;
    }
    count = 0;
    fullpath = filename;
    // Try in all configured directories
    while (1) {
        if (!stat(fullpath.c_str(), &st)) {
            image = new GUI_Surface(fullpath.c_str());
        }
        if (image) {
            break;
        }
        if (count < imagepath.size() && filename.c_str()[0] != '/') {
            fullpath = imagepath[count];
            fullpath += "/";
            fullpath += filename;
            count++;
            continue;
        }
        Log(0, "Failed to load image %s", filename.c_str());
        return NULL;
    }
    if (cache) {
        Lock();
        images.push_back(image);
        Unlock();
    } else {
        // No cache means parent will be the sole owner of the image
        image->SetRefCount(0);
    }
    return image;
}

void SystemObject::RegisterExternalEventHandler(Object *obj)
{
    Lock();
    obj->IncRef();
    externaleventhandlers.push_back(obj);
    Unlock();
}

void SystemObject::UnregisterExternalEventHandler(Object *obj)
{
    Lock();
    for (int i = 0; i < externaleventhandlers.size(); i++) {
        if (externaleventhandlers[i] == obj) {
            externaleventhandlers.erase(externaleventhandlers.begin() + i);
            obj->DecRef();
            break;
        }
    }
    Unlock();
}

int SystemObject::dbInit(void)
{
    int rc;

    const char *path = configGetValue("db_path", GetHome());
    int flags = DB_CREATE|DB_THREAD|DB_INIT_MPOOL|DB_PRIVATE|DB_INIT_LOCK;

    if (db_env) {
        return 0;
    }
    if (access(path, F_OK) && mkdir(path, 0755)) {
        Log(0, "dbInit: %s: %s", path, strerror(errno));
        return -1;
    }
    if ((rc = db_env_create(&db_env, 0)) != 0) {
        Log(0, "dbInit: db_env_create: %s", db_strerror(rc));
        return -1;
    }
    db_env->set_cachesize(db_env, 0, configGetInt("db_cachesize", 1024*1024*32), 0);
    db_env->set_errpfx(db_env, GetName());
    db_env->set_errcall(db_env, db_error);
    db_env->set_tmp_dir(db_env, path);
    db_env->set_data_dir(db_env, path);
    if ((rc = db_env->open(db_env, path, flags, 0)) != 0) {
        db_env->err(db_env, rc, "dbInit: %s", path);
        db_env->close(db_env, 0);
        db_env = 0;
        return -1;
    }
    // Required system databases
    dbOpen("system", 0);
    dbOpen("config", 0);
    dbOpen("calendar", 0);
    dbOpen("favorites", 0);
    dbOpen("log", DB_DUPSORT);
    return 0;
}

char *SystemObject::dbList(void)
{
    string rc;
    Lock();
    for (int i = 0; i < db_list.size(); i++) {
        rc += db_list[i]->dname;
        rc += " ";
    }
    Unlock();
    return strdup(rc.c_str());
}

void SystemObject::dbClose(void)
{
    Lock();
    while (db_list.size()) {
        db_list.back()->close(db_list.back(), 0);
        db_list.pop_back();
    }
    if (db_env) {
        db_env->close(db_env, 0);
        db_env = 0;
    }
    Unlock();
}

int SystemObject::dbOpen(const char *dbname, int flags)
{
    DB *db = 0;
    int recover = 0;
    char path[256];

    Lock();
    if ((db = dbFind(dbname))) {
        Unlock();
        return 0;
    }
again:
    int rc = db_create(&db, db_env, 0);
    if (rc) {
        db_env->err(db_env, rc, "db_create: %s", dbname);
        Unlock();
        return -1;
    }
    db->set_flags(db, flags);
    int db_pagesize = configGetInt("db_pagesize", 0);
    int db_btminkey = configGetInt("db_btminkey", 0);
    if (db_btminkey) db->set_bt_minkey(db, db_btminkey);
    if (db_pagesize) db->set_pagesize(db, db_pagesize);

    snprintf(path, sizeof(path), "%s/lmbox.db", configGetValue("db_path", GetHome()));
    if ((rc = db->open(db, 0, path, dbname, DB_BTREE, DB_CREATE|DB_THREAD, 0664)) != 0) {
        db->err(db, rc, "dbOpen: %s: %s", dbname, path);
        db->close(db, 0);
        // Re-create database in case of fatal corruption
        if ((rc == DB_RUNRECOVERY || rc == EINVAL) && recover == 0) {
            recover = 1;
            unlink(path);
            Log(0, "dbOpen: %s: re-creating database file", dbname);
            goto again;
        }
        Unlock();
        return -1;
    }
    db_list.push_back(db);
    Unlock();
    return 0;
}

int SystemObject::dbDrop(const char *dbname)
{
    Lock();
    for (int i = 0; i < db_list.size(); i++) {
        if (!strcmp(db_list[i]->dname, dbname)) {
            char *path = strdup(db_list[i]->fname);
            db_list[i]->close(db_list[i], 0);
            db_list.erase(db_list.begin() + i);
            int rc = db_env->dbremove(db_env, 0, path, dbname, 0);
            free(path);
            break;
        }
    }
    Unlock();
    return 0;
}

void SystemObject::dbSync(void)
{
    for (int i = 0; i < db_list.size(); i++) {
        db_list[i]->sync(db_list[i], 0);
    }
}

void SystemObject::dbCompact(void)
{
#if DB_VERSION_MAJOR >= 4
#if DB_VERSION_MINOR >= 5
    for (int i = 0; i < db_list.size(); i++) {
        db_list[i]->compact(db_list[i], NULL, NULL, NULL, NULL, 0, NULL);
    }
#endif
#endif
}

DB *SystemObject::dbFind(const char *dbname)
{
    Lock();
    DB *db = 0;
    for (int i = 0; i < db_list.size(); i++) {
        if (!strcmp(db_list[i]->dname, dbname)) {
            db = db_list[i];
            break;
        }
    }
    Unlock();
    return db;
}

int SystemObject::dbPut(const char *dbname, const char *name, const char *value)
{
    DBT key, data;
    DB *db = dbFind(dbname);
    if (!db) {
        Log(0, "dbPut: %s: not found", dbname);
        return -1;
    }
    memset(&key, 0, sizeof(DBT));
    key.data = (void*)name;
    key.size = strlen(name) + 1;
    memset(&data, 0, sizeof(DBT));
    data.data = (void*)value;
    data.size = strlen(value) + 1;
    int rc = db->put(db, 0, &key, &data, 0);
    if (rc) {
        db->err(db, rc, "dbPut: %s", dbname);
        return -1;
    }
    return 0;
}

int SystemObject::dbDel(const char *dbname, const char *name)
{
    int rc;
    DB *db = dbFind(dbname);
    if (!db) {
        Log(0, "dbDel: %s: not found", dbname);
        return -1;
    }
    if (name && *name) {
        DBT key;
        memset(&key, 0, sizeof(DBT));
        key.data = (void*)name;
        key.size = strlen(name) + 1;
        rc = db->del(db, 0, &key, 0);
    } else {
        u_int32_t count;
        rc = db->truncate(db, 0, &count, 0);
    }
    if (rc && rc != DB_NOTFOUND) {
        db->err(db, rc ,"dbDel: %s", dbname);
        return -1;
    }
    return 0;
}

unsigned long SystemObject::dbNextId(const char *key)
{
    if (!key) {
        key = "lmbox_sequence";
    }
    unsigned long id = 1;
    Lock();
    char *val = dbGet("system", key);
    if (val) {
        id = atol(val);
        free(val);
    }
    dbPut("system", key, uintString(id + 1));
    Unlock();
    return id;
}

char *SystemObject::dbGet(const char *dbname, const char *name)
{
    DBT key, data;
    DB *db = dbFind(dbname);
    if (!db) {
        Log(0, "dbGet: %s: not found", dbname);
        return 0;
    }

    memset(&key, 0, sizeof(DBT));
    key.data = (void*)name;
    key.size = strlen(name) + 1;
    memset(&data, 0, sizeof(DBT));
    data.flags = DB_DBT_MALLOC;

    int rc = db->get(db, NULL, &key, &data, 0);
    if (rc != 0 && rc != DB_NOTFOUND) {
        db->err(db, rc, "dbGet: %s", dbname);
        return 0;
    }
    return (char*)data.data;
}

int SystemObject::dbExists(const char *dbname, const char *name)
{
    char buf[2];
    DBT key, data;
    DB *db = dbFind(dbname);
    if (!db) {
        Log(0, "dbExists: %s: not found", dbname);
        return 0;
    }

    memset(&key, 0, sizeof(DBT));
    key.data = (void*)name;
    key.size = strlen(name) + 1;
    memset(&data, 0, sizeof(DBT));
    data.flags = DB_DBT_PARTIAL|DB_DBT_USERMEM;
    data.data = &buf;
    data.dlen = 1;
    data.ulen = 1;

    int rc = db->get(db, NULL, &key, &data, 0);
    if (rc != 0 && rc != DB_NOTFOUND) {
        db->err(db, rc, "dbExists: %s", dbname);
        return 0;
    }
    return rc != DB_NOTFOUND;
}

unsigned long SystemObject::dbStat(const char *dbname, const char *param)
{
    DB *db = dbFind(dbname);
    if (!db) {
        Log(0, "dbStat: %s: not found", dbname);
        return 0;
    }
    DB_BTREE_STAT *stat;
    unsigned long result = 0;
    int rc = db->stat(db, 0, &stat, 0);
    if (rc) {
        db->err(db, rc, "dbStat: %s", dbname);
        return 0;
    }
    if (!strcmp(param, "count")) {
        result = stat->bt_ndata;
    }
    if (!strcmp(param, "pagesize")) {
        result = stat->bt_ndata;
    }
    if (!strcmp(param, "minkey")) {
        result = stat->bt_ndata;
    }
    if (!strcmp(param, "levels")) {
        result = stat->bt_levels;
    }
    if (!strcmp(param, "keys")) {
        result = stat->bt_nkeys;
    }
    if (!strcmp(param, "pages")) {
        result = stat->bt_int_pg;
    }
    free(stat);
    return result;
}

int SystemObject::dbEval(const char *dbname, void *arg, int (*script)(void*, char *, char*), const char *keystr, int op_start, int op_next)
{
    DBC *cursor;
    DBT key, data;
    int status;

    DB *db = dbFind(dbname);
    if (!db) {
        Log(0, "dbEval: %s: not found", dbname);
        return -1;
    }
    memset(&key, 0, sizeof(DBT));
    key.flags = DB_DBT_REALLOC;
    memset(&data, 0, sizeof(DBT));
    data.flags = DB_DBT_REALLOC;

    // Range request, position cursor to the closest key
    if (keystr) {
        key.data = strdup(keystr);
        key.size = strlen(keystr) + 1;
        op_start = DB_SET_RANGE;
    }
    int rc = db->cursor(db, NULL, &cursor, 0);
    if (rc) {
        db->err(db, rc, "dbEval: cursor: %s", dbname);
        lmbox_free(key.data);
        return -1;
    }
    rc = cursor->c_get(cursor, &key, &data, op_start ? op_start : DB_FIRST);
    while (!rc) {
        // Execute the script for each record
        status = script(arg, (char*)key.data, (char*)data.data);
        if (status == 1) {
            break;
        }
        if (status == -1) {
            rc = cursor->c_del(cursor, 0);
        }
        // Advance to the next record
        rc = cursor->c_get(cursor, &key, &data, op_next ? op_next : DB_NEXT);
    }
    lmbox_free(key.data);
    lmbox_free(data.data);
    cursor->c_close(cursor);
    if (rc != 0 && rc != DB_NOTFOUND) {
        db->err(db, rc, "dbEval: c_get: %s", dbname);
        return -1;
    }
    return 0;
}

Variant SystemObject::pget_BootTime(void)
{
    return anytovariant((unsigned long)GetBootTime());
}

Variant SystemObject::pget_ThreadID(void)
{
    return anytovariant((unsigned long)GetThreadID());
}

Variant SystemObject::pget_LogLevel(void)
{
    return anytovariant(GetLogLevel());
}

int SystemObject::pset_LogLevel(Variant value)
{
    SetLogLevel(value);
    return 0;
}

Variant SystemObject::pget_IdleTime(void)
{
    return anytovariant(GetIdleTime());
}

int SystemObject::pset_IdleTime(Variant value)
{
    SetIdleTime(value);
    return 0;
}

Variant SystemObject::pget_Focus(void)
{
    return anytovariant(HasFocus());
}

Variant SystemObject::pget_XScale(void)
{
    return anytovariant(GetXScale());
}

int SystemObject::pset_XScale(Variant value)
{
    SetXScale(value);
    return 0;
}

Variant SystemObject::pget_YScale(void)
{
    return anytovariant(GetYScale());
}

int SystemObject::pset_YScale(Variant value)
{
    SetYScale(value);
    return 0;
}

Variant SystemObject::pget_ScreenWidth(void)
{
    return anytovariant(GetScreenWidth());
}

Variant SystemObject::pget_ScreenHeight(void)
{
    return anytovariant(GetScreenHeight());
}

Variant SystemObject::pget_CurrentFile(void)
{
    return anytovariant(configGetValue("xmlfile", "lmbox.xml"));
}

Variant SystemObject::pget_Version(void)
{
    return anytovariant(LMBOX_VERSION);
}

Variant SystemObject::pget_PageCount(void)
{
    return anytovariant(GetPageCount());
}

Variant SystemObject::pget_FontCache(void)
{
    string rc;
    Lock();
    for (int i = 0; i < fonts.size(); i++) {
        rc += fonts[i]->GetName();
        rc += ":";
        rc += intString(fontsizes[i]);
        rc += "\n";
    }
    Unlock();
    return anytovariant(rc);
}

Variant SystemObject::pget_ImageCache(void)
{
    string rc;
    Lock();
    for (int i = 0; i < images.size(); i++) {
        rc += images[i]->GetName();
        rc += "\n";
    }
    Unlock();
    return anytovariant(rc);
}

/*
 * Method functions
 */

Variant SystemObject::m_Quit(int numargs, Variant args[])
{
    Quit();
    return VARNULL;
}

Variant SystemObject::m_FocusNext(int numargs, Variant args[])
{
    PostEvent("next", NULL, NULL);
    return VARNULL;
}

Variant SystemObject::m_FocusPrevious(int numargs, Variant args[])
{
    PostEvent("prev", NULL, NULL);
    return VARNULL;
}

Variant SystemObject::m_SelectFocused(int numargs, Variant args[])
{
    PostEvent("accept", NULL, NULL);
    return VARNULL;
}

Variant SystemObject::m_HandleEvent(int numargs, Variant args[])
{
    string str = args[0];
    HandleEvent(str.c_str());
    return VARNULL;
}

Variant SystemObject::m_GotoPage(int numargs, Variant args[])
{
    string pagename = args[0];
    PostEvent("switchpage", pagename.c_str(), 0);
    return VARNULL;
}

Variant SystemObject::m_Refresh(int numargs, Variant args[])
{
    DoUpdate(1);
    return VARNULL;
}
