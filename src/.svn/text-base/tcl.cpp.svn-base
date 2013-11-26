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

typedef struct {
  int flags;
  int op_start;
  int op_next;
  Tcl_Interp *interp;
  char *dbname;
  char *script;
  char *array;
  char *match;
  char *end;
  char *start;
  char *beginwith;
  int beginlen;
  regex_t *regex;
} dbEvalParam;

class Listen {
  public:
    Listen();
    ~Listen();
    int Setup(void);
    int Execute(char *code);
    const char *GetType();

    int fd;
    int type;
    int size;
    int flags;
    int interval;
    char *port;
    char *script;
    char *oninit;
    char *onexit;
    time_t timestamp;
    char *channel;
    struct sockaddr_in sa;
    char data[BUFFER_SIZE];
};

class Schedule {
  public:
    Schedule();
    ~Schedule();
    int Setup(void);

    unsigned long id;
    char *script;
    bool once_flag;
    bool enable_flag;
    bool thread_flag;
    time_t timestamp;
    int interval;
    char min[61];
    char hour[25];
    char mday[32];
    char mon[13];
    char wday[8];
    char year[11];
};

typedef struct {
    Tcl_Interp *interp;
    char *file;
    char *data;
    char *startElement;
    char *endElement;
    char *characters;
} XmlParser;

static bool schedthread = 0;
static vector < Schedule * >schedqueue;
static pthread_mutex_t schedmutex = PTHREAD_MUTEX_INITIALIZER;

static string interpscript;
static vector < Tcl_Interp * >interpqueue;
static pthread_mutex_t interpmutex = PTHREAD_MUTEX_INITIALIZER;

static bool listenthread = 0;
static vector < Listen * >listenqueue;
static pthread_mutex_t listenmutex = PTHREAD_MUTEX_INITIALIZER;

static vector < Object * >lockqueue;
static pthread_mutex_t lockmutex = PTHREAD_MUTEX_INITIALIZER;

static vector < char * >threadqueue;
static pthread_mutex_t threadmutex = PTHREAD_MUTEX_INITIALIZER;

static char *latin_xlt[] = {
  "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand",
  "apostrophe", "parenleft", "parenright", "asterisk", "plus",
  "comma", "minus", "period", "slash", "0", "1", "2", "3", "4", "5", "6", "7",
  "8", "9", "colon", "semicolon", "less", "equal", "greater", "question", "at",
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
  "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
  "backslash", "bracketright", "asciicircum", "underscore", "grave",
  "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
  "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft", "bar",
  "braceright", "asciitilde" };

static char *shifted = "~!@#$%^&*()_+|}{\":?><ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static Tcl_Interp *Script_Create(void);
static Tcl_Interp *Script_Alloc(void);
static void Script_Dealloc(Tcl_Interp *interp);
static void Script_Error(Tcl_Interp * interp, char *fmt, ...);
static int Script_Run(const char *code, bool thread);
static int Script_Cmd(ClientData arg, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);
static void XmlError(void *user_data, const char *msg, ...);
static xmlcallbackfn XmlStartElement;
static xmlcallbackfn XmlEndElement;
static xmlcallbackfn XmlCharacters;

static Object *FindMutex(const char *name)
{
    Object *obj = 0;
    pthread_mutex_lock(&lockmutex);
    for (int i = 0; i < lockqueue.size(); i++) {
         if (!strcmp(lockqueue[i]->GetName(), name)) {
             obj = lockqueue[i];
             obj->IncRef();
             break;
         }
    }
    pthread_mutex_unlock(&lockmutex);
    return obj;
}

Script::Script(const char *name, const char *script)
{
    this->name = lmbox_strdup(name);
    this->code = trimString(script);
    this->lineoffset = 0;
    this->callback1 = 0;
    this->callback2 = 0;
    this->result = 0;
    this->flags = 0;
    this->arg = 0;
}

Script::~Script(void)
{
    lmbox_free(name);
    lmbox_free(code);
    lmbox_free(result);
}

void Script::SetCallback(void *arg, scriptcallbackfn *callback1, scriptcallbackfn *callback2)
{
    this->arg = arg;
    this->callback1 = callback1;
    this->callback2 = callback2;
}

void Script::SetLineOffset(int line)
{
    this->lineoffset = line;
}

void Script::SetScript(const char *script)
{
    lmbox_free(this->code);
    this->code = trimString(script);
}

void Script::Execute(const char *script)
{
    SetScript(script);
    Execute();
}

void Script::Execute(void)
{
    interp = scriptAlloc();
    if (!interp) {
        return;
    }
    // Run script callback first
    if (callback1) {
        (*callback1)(this, arg);
    }
    lmbox_destroy((void**)&this->result);
    // Run the script if provided, otherwise it's just allocating Tcl interp
    if (code && *code) {
        switch (Tcl_EvalEx(interp, code, -1, 0)) {
         case TCL_OK:
         case TCL_BREAK:
         case TCL_RETURN:
         case TCL_CONTINUE:
            break;

         default:
            Script_Error(interp, "Error: name %s, line %d, result %s, '%s', ", GetName(), lineoffset, Tcl_GetStringResult(interp), code);
        }
        this->result = lmbox_strdup(Tcl_GetStringResult(interp));
        Tcl_ResetResult(interp);
    }
    // Run script callback on end
    if (callback2) {
        (*callback2)(this, arg);
    }
}

const char *Script::GetResult(void)
{
    return this->result ? this->result : "";
}

const char *Script::GetScript(void)
{
    return this->code ? this->code : "";
}

const char *Script::GetName(void)
{
    return this->name ? this->name : "tcl";
}

int Script::GetFlags(void)
{
    return flags;
}

void Script::SetFlags(int mask)
{
    flags |= mask;
}

void Script::ClearFlags(int mask)
{
    flags &= ~mask;
}

Tcl_Interp *Script::GetInterp()
{
    return interp;
}

Schedule::Schedule(void)
{
    id = 0;
    enable_flag = true;
    thread_flag = false;
    once_flag = false;
    script = 0;
    timestamp = 0;
    interval = 0;
    timestamp = 0;
    memset(min, -1, sizeof(min));
    memset(hour, -1, sizeof(hour));
    memset(mday, -1, sizeof(mday));
    memset(mon, -1, sizeof(mon));
    memset(wday, -1, sizeof(wday));
    memset(year, -1, sizeof(year));
}

Schedule::~Schedule(void)
{
    lmbox_free(script);
}

int Schedule::Setup(void)
{
    // Use global id generator
    if (!id) {
        id = systemObject->dbNextId(0);
    }
    // Run in interval seconds in the future
    if (interval > 0) {
        return 0;
    }
    if (min[sizeof(min) - 1] == -1) {
        memset(min, 2, sizeof(min));
    }
    if (mon[sizeof(mon) - 1] == -1) {
        memset(mon, 2, sizeof(mon));
    }
    if (hour[sizeof(hour) - 1] == -1) {
        memset(hour, 2, sizeof(hour));
    }
    if (mday[sizeof(mday) - 1] == -1) {
        memset(mday, 2, sizeof(mday));
    }
    if (wday[sizeof(wday) - 1] == -1) {
        memset(wday, 2, sizeof(wday));
    }
    if (year[sizeof(year) - 1] == -1) {
        memset(year, 2, sizeof(year));
    }
    return 0;
}

int threadCreate(void *(*func)(void*), void *arg)
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&tid, &attr, func, arg)) {
        systemObject->Log(0, "pthread_create: %s", strerror(errno));
        return -1;
    }
    pthread_attr_destroy(&attr);
    return 0;
}

void threadNameSet(const char *name, ...)
{
    Tls *tls = tlsGet();
    va_list ap;
    va_start(ap, name);
    vsnprintf(tls->threadname, sizeof(tls->threadname), name, ap);
    va_end(ap);
    pthread_mutex_lock(&threadmutex);
    threadqueue.push_back(tls->threadname);
    pthread_mutex_unlock(&threadmutex);
}

void threadNameClear(const char *name)
{
    pthread_mutex_lock(&threadmutex);
    for (int i = 0; i < threadqueue.size(); i++) {
        if (name == threadqueue[i]) {
            threadqueue.erase(threadqueue.begin() + i);
            break;
        }
    }
    pthread_mutex_unlock(&threadmutex);
}

static void *scheduler_thread(void *arg)
{
    struct tm lt;
    Schedule *sched;

    schedthread = 1;
    threadNameSet("scheduler");

    while (!systemObject->IsQuit()) {
        pthread_mutex_lock(&schedmutex);
        for (int i = 0; i < schedqueue.size(); i++) {
            sched = schedqueue[i];
            if (!sched->enable_flag) {
                continue;
            }
            time_t now = time(0);
            localtime_r(&now, &lt);
            // Interval based schedule
            if (sched->interval > 0) {
                if (!sched->timestamp) {
                    sched->timestamp = now + sched->interval;
                }
                if (now - sched->timestamp < -15 || sched->timestamp - now > 15) {
                    continue;
                }
                // Assign next run timestamp
                sched->timestamp = now + sched->interval;
            } else
            // Exact time based schedule
            if (sched->min[lt.tm_min] == -1 ||
                sched->hour[lt.tm_hour] == -1 ||
                sched->mon[lt.tm_mon] == -1 ||
                sched->year[(lt.tm_year + 1900) - 2005] == -1 ||
                (sched->mday[lt.tm_mday] == -1 || sched->wday[lt.tm_wday] == -1)) {
                continue;
            }
            if (sched->once_flag) {
                sched->enable_flag = false;
            }
            systemObject->Log(5, "%d-%02d-%02d %02d:%02d:%02d: sched%d: %s, %d", lt.tm_year + 1900, lt.tm_mon, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, i, sched->script, sched->interval);
            Script_Run(sched->script, sched->thread_flag);
        }
        pthread_mutex_unlock(&schedmutex);
        sleep(10);
    }
    pthread_mutex_lock(&schedmutex);
    schedthread = 0;
    while (schedqueue.size() > 0 ) {
        sched = schedqueue.back();
        schedqueue.pop_back();
        delete sched;
    }
    pthread_mutex_unlock(&schedmutex);
    systemObject->Log(0, "scheduler_thread: exited");
    Tcl_ExitThread(0);
    return 0;
}

// Callback script to be called before executing socket Tcl code
static void listen_setup_callback(Script *script, void *arg)
{
    Listen *lst = (Listen*)arg;
    Tcl_Interp *interp = script->GetInterp();
    Tcl_Channel chan = 0;

    Tcl_SetVar(interp, "lmbox_chan", "", 0);
    Tcl_SetVar(interp, "lmbox_fd", intString(lst->fd), 0);
    Tcl_SetVar(interp, "lmbox_object", lst->port, 0);
    Tcl_SetVar(interp, "lmbox_data", lst->data, 0);
    Tcl_SetVar(interp, "lmbox_type", lst->GetType(), 0);
    Tcl_SetVar(interp, "lmbox_port", intString(ntohs(lst->sa.sin_port)), 0);
    Tcl_SetVar(interp, "lmbox_ipaddr", sock_inet_str(lst->sa.sin_addr), 0);

    lmbox_free(lst->channel);
    lst->channel = 0;
    switch (lst->type) {
     case SOCK_FILE:
     case SOCK_STREAM:
        chan = Tcl_MakeFileChannel((ClientData)dup(lst->fd), TCL_READABLE|TCL_WRITABLE);
        break;
    }
    if (chan) {
        lst->channel = strdup(Tcl_GetChannelName(chan));
        Tcl_RegisterChannel(interp, chan);
        Tcl_SetVar(interp, "lmbox_chan", lst->channel, 0);
    }
}

// Callback script to be called after executing socket Tcl code
static void listen_cleanup_callback(Script *script, void *arg)
{
    Listen *lst = (Listen*)arg;
    Tcl_Interp *interp = script->GetInterp();

    if (lst->channel) {
        Tcl_Channel chan = Tcl_GetChannel(interp, lst->channel, 0);
        if (chan) {
            Tcl_UnregisterChannel(interp, chan);
        }
        lmbox_free(lst->channel);
        lst->channel = 0;
    }
}

Listen::Listen(void)
{
    type = SOCK_STREAM;
    fd = -1;
    size = 0;
    flags = 0;
    data[0] = 0;
    port = 0;
    script = 0;
    oninit = 0;
    onexit = 0;
    interval = 0;
    timestamp = 0;
    channel = 0;
}

Listen::~Listen(void)
{
    if (fd > 0) {
        close(fd);
    }
    lmbox_free(channel);
    lmbox_free(script);
    lmbox_free(oninit);
    lmbox_free(onexit);
    lmbox_free(port);
}

// Send given script if configured
int Listen::Execute(char *code)
{
    if (code) {
        Script script(GetType(), code);
        script.SetCallback(this, listen_setup_callback, listen_cleanup_callback);
        script.Execute();
    }
    return 0;
}

const char *Listen::GetType(void)
{
   switch (type) {
    case SOCK_FILE:
       return "file";
    case SOCK_DGRAM:
       return "udp";
    case SOCK_STREAM:
       return "tcp";
   }
}

int Listen::Setup(void)
{
   int n = 1;
   switch (type) {
    case SOCK_FILE:
       fd = open(port, O_RDWR);
       if (fd == -1) {
           systemObject->Log(0, "ListenSetup: %s: %s: open: %s", GetType(), port, strerror(errno));
           return -1;
       }
       if (flags & LISTEN_SEEK_END && lseek(fd, 0, SEEK_END) == -1) {
           systemObject->Log(0, "ListenSetup: %s: %s: lseek: %s", GetType(), port, strerror(errno));
       }
       break;

    case SOCK_DGRAM:
       fd = sock_listen_udp(atoi(port));
       if (fd == -1) {
           return -1;
       }
       break;

    case SOCK_STREAM:
       fd = sock_listen_tcp(atoi(port));
       if (fd == -1) {
           return -1;
       }
       break;
   }
   if (flags & LISTEN_ASYNC && ioctl(fd, FIONBIO, &n) == -1) {
       systemObject->Log(0, "ListenSetup: %s: %s: FIONBIO: %s", GetType(), port, strerror(errno));
   }
   Execute(oninit);
   return fd;
}

// Executes system command
static void *run_process(void *arg)
{
    threadNameSet((const char *)arg);
    system((char *) arg);
    lmbox_free(arg);
    Tcl_ExitThread(0);
}

// Executes script in separate thread
static void *run_thread(void *arg)
{
    threadNameSet((const char*)arg);
    // Script code
    Script_Run((const char *) arg, false);
    lmbox_free(arg);
    Tcl_ExitThread(0);
}

// Executes socket callback
static void *run_listen_script(void *arg)
{
    Listen *lst = (Listen*)arg;
    lst->Execute(lst->script);
    delete lst;
}

static void *listener_thread(void *arg)
{
    fd_set fds;
    time_t now;
    struct timeval tv;
    Listen *lst, *lst2;
    int size, fd, fdmax, errcount = 0;
    struct sockaddr_in sa;
    int salen = sizeof(struct sockaddr_in);

    listenthread = 1;
    threadNameSet("listener");

    while (!systemObject->IsQuit()) {
        FD_ZERO(&fds);
        fdmax = 0;
        now = time(0);
        pthread_mutex_lock(&listenmutex);
        for (int i = 0; i < listenqueue.size(); i++) {
            lst = listenqueue[i];
            switch (lst->type) {
             case SOCK_FILE:
                // Check interval if we need to skip polling this fd
                if (lst->interval > 0 && lst->timestamp > 0 &&
                    now - lst->timestamp < lst->interval) {
                    continue;
                }
                if (lst->flags & LISTEN_SEEK_START && lseek(lst->fd, 0, SEEK_SET) == -1) {
                    systemObject->Log(0, "ListenSetup: lseek: %s: %s", lst->port, strerror(errno));
                }
                break;
            }
            FD_SET(lst->fd,&fds);
            if (lst->fd > fdmax) {
                fdmax = lst->fd;
            }
        }
        pthread_mutex_unlock(&listenmutex);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        switch(select(fdmax + 1, &fds, 0, 0, &tv)) {
         case 0:
            continue;
         case -1:
            if (errno != EINTR && errcount < 2) {
                systemObject->Log(0, "ListenThread: select: %d: %s", fdmax, strerror(errno));
            }
            continue;
        }
        if (systemObject->IsQuit()) {
            break;
        }
        pthread_mutex_lock(&listenmutex);
        for (int i = 0; i < listenqueue.size(); i++) {
            lst = listenqueue[i];
            if (!FD_ISSET(lst->fd,&fds)) {
                continue;
            }
            lst->size = 0;
            lst->timestamp = time(0);
            switch (lst->type) {
             case SOCK_DGRAM:
                lst->size = recvfrom(lst->fd, lst->data, sizeof(lst->data)-1, 0, (struct sockaddr*)&lst->sa, (socklen_t*)&salen);
                if (lst->size < 0) {
                    systemObject->Log(0, "ListenThread: recvfrom: %s: %s", lst->port, strerror(errno));
                    continue;
                }
                if (lst->size == 0) {
                    continue;
                }
                fd = -1;
                break;

             case SOCK_STREAM:
                fd = accept(lst->fd, (struct sockaddr*)&lst->sa, (socklen_t*)&salen);
                if (fd == -1) {
                    systemObject->Log(0, "ListenThread: accept: %s: %s", lst->port, strerror(errno));
                    continue;
                }
                break;

             case SOCK_FILE:
                while (lst->size < sizeof(lst->data)-1) {
                    if (read(lst->fd, &lst->data[lst->size], 1) <= 0 ||
                        lst->data[lst->size] == '\n') {
                        break;
                    }
                    lst->size++;
                }
                if (lst->size == 0) {
                    continue;
                }
                fd = dup(lst->fd);
                break;
            }
            lst->data[lst->size] = 0;
            lst2 = new Listen();
            lst2->fd = fd;
            lst2->type = lst->type;
            lst2->size = lst->size;
            lst2->flags = lst->flags;
            memcpy(&lst2->sa, &lst->sa, sizeof(lst->sa));
            if (lst->size) {
                memcpy(lst2->data, lst->data, lst->size+1);
            }
            lst2->port = lmbox_strdup(lst->port);
            lst2->script = lmbox_strdup(lst->script);
            lst2->oninit = lmbox_strdup(lst->oninit);
            lst2->onexit = lmbox_strdup(lst->onexit);
            if (lst2->flags & LISTEN_THREAD) {
                if (threadCreate(run_listen_script, (void *)lst2)) {
                    systemObject->Log(0, "ListenThread: pthread_create: %s", strerror(errno));
                    delete lst2;
                }
            } else {
                run_listen_script(lst2);
            }
        }
        pthread_mutex_unlock(&listenmutex);
    }
    pthread_mutex_lock(&listenmutex);
    while (listenqueue.size() > 0 ) {
        lst = listenqueue.back();
        listenqueue.pop_back();
        lst->Execute(lst->onexit);
        delete lst;
    }
    listenthread = 0;
    pthread_mutex_unlock(&listenmutex);
    systemObject->Log(0, "listener_thread: exited");
    Tcl_ExitThread(0);
    return 0;
}

int scriptInit(void)
{
    return 0;
}

void scriptShutdown(void)
{
    pthread_mutex_lock(&interpmutex);
    while (interpqueue.size() > 0 ) {
        Tcl_Interp *interp = interpqueue.back();
        interpqueue.pop_back();
        Tcl_DeleteInterp(interp);
    }
    pthread_mutex_unlock(&interpmutex);
}

// Get existing interp from the queue
Tcl_Interp *scriptAlloc(void)
{
    if (systemObject->IsShutdown()) {
        return 0;
    }
    Tls *tls = tlsGet();
    if (tls->interp) {
        return tls->interp;
    }
    Tcl_Interp *interp = 0;
    pthread_mutex_lock(&interpmutex);
    if (interpqueue.size() > 0) {
        interp = interpqueue.back();
        interpqueue.pop_back();
    }
    pthread_mutex_unlock(&interpmutex);
    // Create new interp
    if (!interp && !(interp = Script_Create())) {
        return 0;
    }
    tls->interp = interp;
    return interp;
}

// Put interp back to the queue
void scriptDealloc(Tcl_Interp *interp)
{
    if (systemObject->IsShutdown() || systemObject->configGetInt("tcl_nocache", 0)) {
        Tcl_DeleteInterp(interp);
        return;
    }
    // Run Tcl cleanup proc
    if (Tcl_EvalEx(interp, "lmbox_tclcleanup", -1, 0) != TCL_OK) {
        Script_Error(interp, "tclcleanup: ");
    }
    pthread_mutex_lock(&interpmutex);
    interpqueue.push_back(interp);
    pthread_mutex_unlock(&interpmutex);
}

static Tcl_Interp *Script_Create()
{
    char *script, buf[256];

    // Check if we have init script loaded into memory already
    pthread_mutex_lock(&interpmutex);
    if (interpscript == "") {
        script = strFileRead(systemObject->configGetValue("initfile", "lmbox.tcl"), 0);
        if (script) {
            interpscript += script;
            interpscript += "\n";
            free(script);
        }
        // Load int.tcl from the Tcl library directory
        const char *tcl_path = systemObject->configGetValue("tcldir", "modules");
        snprintf(buf, sizeof(buf), "%s/init.tcl", tcl_path);
        script = strFileRead(buf, 0);
        if (script) {
            interpscript += script;
            interpscript += "\n";
            free(script);
        }
        // Load all Tcl files
        struct dirent **files = 0;
        int count = scandir(tcl_path, &files, 0, alphasort);
        for (int i = 0;i < count; i++) {
            if (files[i]->d_name[0] != '.' &&
                strcmp(files[i]->d_name, "init.tcl") &&
                !matchString(files[i]->d_name, "*.tcl")) {
                snprintf(buf, sizeof(buf), "%s/%s", tcl_path, files[i]->d_name);
                script = strFileRead(buf, 0);
                if (script) {
                    interpscript += script;
                    interpscript += "\n";
                    free(script);
                }
            }
            lmbox_free(files[i]);
        }
        lmbox_free(files);
    }
    pthread_mutex_unlock(&interpmutex);

    Tcl_Interp *interp = Tcl_CreateInterp();
    Tcl_InitMemory(interp);
    if (Tcl_Init(interp) != TCL_OK) {
        Script_Error(interp, 0);
        Tcl_DeleteInterp(interp);
        return 0;
    }
    Tcl_CreateObjCommand(interp, "lmbox", Script_Cmd, 0, NULL);
    // Load Tcl init script into new interp
    if (Tcl_EvalEx(interp, interpscript.c_str(), -1, TCL_EVAL_GLOBAL) != TCL_OK) {
        Script_Error(interp, "init: ");
    }
    return interp;
}

static int Script_Run(const char *code, bool thread)
{
    if (thread) {
        char *arg = strdup(code);
        if (threadCreate(run_thread, (void *) arg)) {
            free(arg);
            return -1;
        }
        return 0;
    }
    Script script("run", (char *) code);
    script.Execute();
    return 0;
}

static void Script_Error(Tcl_Interp * interp, char *fmt, ...)
{
    char *msg = 0;
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        msg = lmbox_vprintf(fmt, ap);
        va_end(ap);
    }
    const char *errorInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    systemObject->Log(0, "%s%s", msg ? msg : "", errorInfo ? errorInfo : "");
    lmbox_free(msg);
}

static int Script_DbEval(void *arg, char *key, char *value)
{
    dbEvalParam *param = (dbEvalParam*)arg;
    if (!key || !value) {
        systemObject->Log(0, "dbEval: %s: empty key/value", param->dbname);
        return -1;
    }
    // Key matching
    if (param->match) {
        // Call the script for matching records only
        if (matchString(key, param->match)) return 0;
    } else
    if (param->regex) {
        if(matchRegex(key, param->regex)) return 0;
    } else
    if (param->beginwith) {
        // Stop if key does not start with the pattern
        if (strncmp(key, param->beginwith, param->beginlen)) {
            return 1;
        }
    }
    // Max value
    if (param->end) {
        if (strcmp(key, param->end) > 0) {
            return 1;
        }
    }
    // If we got here, we can run Tcl script
    Tcl_SetVar(param->interp, "lmbox_dbkey", key, 0);
    Tcl_SetVar(param->interp, "lmbox_dbdata", value, 0);
    // If split flag is set that means the record is a list
    // of name-value pairs as in Tcl "array set"
    // If array name is given, then populate array elements instead
    if (param->flags & DBEVAL_SPLIT) {
        int argc;
        const char **argv;
        if (param->array) {
            Tcl_UnsetVar(param->interp, param->array, 0);
        }
        if (Tcl_SplitList(param->interp, value, &argc, &argv) == TCL_OK) {
            for (int i = 0; i < argc - 1; i += 2) {
                if (param->array) {
                    Tcl_SetVar2(param->interp, param->array, argv[i], argv[i+1], 0);
                } else {
                    Tcl_SetVar(param->interp, argv[i], argv[i+1], 0);
                }
            }
            Tcl_Free((char*)argv);
        } else {
            Script_Error(param->interp, "dbeval: split: %s: %s: ", param->dbname, key);
            return (param->flags & DBEVAL_STOP ? 1 : 0);
        }
    }
    int rc = Tcl_EvalEx(param->interp, param->script, -1, 0);
    const char *result = Tcl_GetStringResult(param->interp);
    switch(rc) {
     case TCL_OK:
     case TCL_CONTINUE:
        break;
     case TCL_RETURN:
        if (!strcmp("lmbox_stop", result)) {
            return 1;
        }
        if (!strcmp("lmbox_delete", result)) {
            return -1;
        }
        break;
     case TCL_BREAK:
        return 1;
     default:
        Script_Error(param->interp, "dbeval: script: %s: %s: ", param->dbname, key);
        if (param->flags & DBEVAL_STOP) {
            return 1;
        }
    }
    return 0;
}

static int Script_Cmd(ClientData arg, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
    GUI_Page *page = 0;
    DynamicObject *obj = 0;

    static CONST char *opts[] = {
        "version", "getcwd", "gethome", "puts", "match",
        "run", "exec", "quit", "isquit", "isshutdown", "delay", "call",
        "get", "set", "exists", "create",
        "setpage", "getpage", "prevpage", "newpage",
        "postevent", "eventfire", "eventcreate", "eventremove", "getevent", "geteventdata",
        "config", "setconfig", "listconfig",
        "key", "setkey", "listkey",
        "var", "varset", "varunset", "varlist", "varappend", "varincr",
        "schedule", "schedulelist", "scheduleclose", "scheduleinit", "scheduleremove",
        "refresh", "eventobject", "waitforobject",
        "listen", "listenlist", "listenremove", "listeninit", "listenclose",
        "udpsend", "urldecode", "urlencode", "striphtml",
        "fileread", "filewrite", "fileopen", "fileclose",
        "objectcreate", "objectremove", "objectrefcount",
        "lockcreate", "lock", "unlock", "lockremove", "locklist",
        "dbget", "dbput", "dbdel", "dbeval", "dbexists",
        "dbsync", "dbcompact", "dbopen", "dblist", "dbnextid", "dbdrop", "dbstat",
        "xevent", "tclscript", "showoverlay", "xmlparse",
        "geturl", "random", "refreshscreen", "statfs",
        "cdromstatus", "cdromeject", "cdromclose",
        "threadid", "threadcount", "threadlist", "threadclose",
        0
    };
    enum {
        cmdVersion, cmdGetCwd, cmdGetHome, cmdPuts, cmdMatch,
        cmdRun, cmdExec, cmdQuit, cmdIsQuit, cmdIsShutdown, cmdDelay, cmdCall,
        cmdPropertyGet, cmdPropertySet, cmdPropertyExists, cmdPropertyCreate,
        cmdPageSet, cmdPageGet, cmdPagePrev, cmdPageNew,
        cmdEventPost, cmdEventFire, cmdEventCreate, cmdEventRemove, cmdEventGet, cmdEventDataGet,
        cmdConfig, cmdConfigSet, cmdConfigList,
        cmdKey, cmdKeySet, cmdKeyList,
        cmdVarGet, cmdVarSet, cmdVarUnset, cmdVarList, cmdVarAppend, cmdVarIncr,
        cmdSchedule, cmdScheduleList, cmdScheduleClose, cmdScheduleInit, cmdScheduleRemove,
        cmdRefresh, cmdEventObject, cmdWaitForObject,
        cmdListen, cmdListenList, cmdListenRemove, cmdListenInit, cmdListenClose,
        cmdUdpSend, cmdUrlDecode, cmdUrlEncode, cmdStripHtml,
        cmdFileRead, cmdFileWrite, cmdFileOpen, cmdFileClose,
        cmdObjectCreate, cmdObjectRemove, cmdObjectRefCount,
        cmdLockCreate, cmdLock, cmdUnlock, cmdLockRemove, cmdLockList,
        cmdDbGet, cmdDbPut, cmdDbDel, cmdDbEval, cmdDbExists,
        cmdDbSync, cmdDbCompact, cmdDbOpen, cmdDbList, cmdDbNextId, cmdDbDrop, cmdDbStat,
        cmdXEvent, cmdTclScript, cmdShowOverlay, cmdXmlParse,
        cmdGetUrl, cmdRandom, cmdRefreshScreen, cmdStatFs,
        cmdCdromStatus, cmdCdromEject, cmdCdromClose,
        cmdThreadID, cmdThreadCount, cmdThreadList, cmdThreadClose
    } cmd;

    if (systemObject->IsShutdown()) {
        return TCL_ERROR;
    }

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], opts, "option", 0, (int *) &cmd) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmd) {
    case cmdVersion:
        Tcl_AppendResult(interp, LMBOX_VERSION, 0);
        break;

    case cmdThreadID:
        Tcl_SetObjResult(interp, Tcl_NewLongObj((long)pthread_self()));
        break;

    case cmdGetCwd:
        Tcl_AppendResult(interp, systemObject->GetCwd(), 0);
        break;

    case cmdPuts: {
        struct tm lt;
        char timebuf[64];
        time_t now = time(0);

        localtime_r(&now, &lt);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %T: ", &lt);
        fprintf(stdout, timebuf);
        for (int i = 2; i < objc; i++) {
            fprintf(stdout, "%s", Tcl_GetString(objv[i]));
        }
        fprintf(stdout, "\n");
        break;
    }

    case cmdGetHome:
        Tcl_AppendResult(interp, systemObject->GetHome(), 0);
        break;

    case cmdCdromStatus:
        Tcl_AppendResult(interp, systemObject->cdromStatus(), 0);
        break;

    case cmdCdromEject:
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->cdromEject()));
        break;

    case cmdCdromClose:
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->cdromClose()));
        break;

    case cmdRefresh:
        systemObject->PostEvent("refresh", 0, 0);
        break;

    case cmdRefreshScreen:
        systemObject->DoUpdate(0);
        break;

    case cmdTclScript:
        pthread_mutex_lock(&interpmutex);
        Tcl_AppendResult(interp, interpscript.c_str(), 0);
        pthread_mutex_unlock(&interpmutex);
        break;

    case cmdDelay:
        SDL_Delay(objc > 2 ? atoi(Tcl_GetString(objv[2])) : 10);
        break;

    case cmdRandom:
        if (objc > 2) {
            Tcl_SetObjResult(interp, Tcl_NewLongObj(1 + (int) (atof(Tcl_GetString(objv[2])) * (rand() / (RAND_MAX + 1.0)))));
        } else {
            Tcl_SetObjResult(interp, Tcl_NewLongObj(random()));
        }
        break;

    case cmdMatch:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "string pattern");
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(matchString(Tcl_GetString(objv[2]), Tcl_GetString(objv[3]))));
        break;

    case cmdUrlDecode:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "data");
            return TCL_ERROR;
        }
        Tcl_AppendResult(interp, urlDecodeString(Tcl_GetString(objv[2])), 0);
        break;

    case cmdUrlEncode:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "data");
            return TCL_ERROR;
        }
        Tcl_AppendResult(interp, urlEncodeString(Tcl_GetString(objv[2])), (Tcl_FreeProc*)free);
        break;

    case cmdThreadCount: {
        pthread_mutex_lock(&threadmutex);
        int count = threadqueue.size();
        pthread_mutex_unlock(&threadmutex);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(count));
        break;
    }

    case cmdThreadList:
        pthread_mutex_lock(&threadmutex);
        for (int i = 0; i < threadqueue.size(); i++) {
            Tcl_AppendResult(interp, intString(i), " {", threadqueue[i], "} ", 0);
        }
        pthread_mutex_unlock(&threadmutex);
        break;

    case cmdThreadClose: {
        int running = 1;
        time_t now = time(0);
        int timeout = systemObject->configGetInt("shutdown_timeout", 30);
        while (running && time(0) - now < timeout) {
            pthread_mutex_lock(&threadmutex);
            running = threadqueue.size();
            pthread_mutex_unlock(&threadmutex);
            sleep(1);
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(running));
        break;
    }

    case cmdStatFs: {
        char str[128];
        struct statvfs vfs;

        if (objc < 3) {
          Tcl_WrongNumArgs(interp, 2, objv, "data");
          return TCL_ERROR;
        }
        if (statvfs(Tcl_GetString(objv[2]), &vfs)) {
            Tcl_AppendResult(interp,"statfs: ",strerror(errno), NULL);
            return TCL_ERROR;
        }
        Tcl_AppendResult(interp,"path ",Tcl_GetString(objv[2]), " ", 0);
        sprintf(str, "%lu", vfs.f_bsize);
        Tcl_AppendResult(interp, "bsize ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_frsize);
        Tcl_AppendResult(interp,"frsize ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_blocks);
        Tcl_AppendResult(interp,"blocks ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_bfree);
        Tcl_AppendResult(interp,"bfree ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_bavail);
        Tcl_AppendResult(interp,"bavail ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_files);
        Tcl_AppendResult(interp,"files ", str , " ", 0);
        sprintf(str, "%lu", vfs.f_ffree);
        Tcl_AppendResult(interp,"ffree ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_favail);
        Tcl_AppendResult(interp,"favail ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_fsid);
        Tcl_AppendResult(interp,"fsid ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_flag);
        Tcl_AppendResult(interp,"flag ", str, " ", 0);
        sprintf(str, "%lu", vfs.f_namemax);
        Tcl_AppendResult(interp, "namemax ", str, " ", 0);
        break;
    }

    case cmdGetUrl: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "url ?-timeout secs? ?-headers hdrs? ?-outheaders varname? ?-data varname?");
            return TCL_ERROR;
        }
        string outheaders;
        int status, size = 0, timeout = 10;
        char *headers = 0, *data = 0, *outname = "", *sname = "";

        for (int argc = 3; argc < objc - 1; argc += 2) {
            if (!strcmp(Tcl_GetString(objv[argc]), "-timeout")) {
                timeout = atoi(Tcl_GetString(objv[argc+1]));
            } else
            if (!strcmp(Tcl_GetString(objv[argc]), "-headers")) {
                headers = Tcl_GetString(objv[argc+1]);
            } else
            if (!strcmp(Tcl_GetString(objv[argc]), "-outheaders")) {
                outname = Tcl_GetString(objv[argc+1]);
            } else
            if (!strcmp(Tcl_GetString(objv[argc]), "-status")) {
                sname = Tcl_GetString(objv[argc+1]);
            }
        }
        status = sock_geturl(Tcl_GetString(objv[2]), timeout, headers, &data, &size, (*outname ? &outheaders : 0));
        if (*outname) {
            Tcl_SetVar(interp, outname, outheaders.c_str(), 0);
        }
        if (*sname) {
            Tcl_ObjSetVar2(interp, Tcl_NewStringObj(sname, -1), 0, Tcl_NewIntObj(status), 0);
        }
        if (size > 0) {
            Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((const unsigned char*)data, size));
            lmbox_free(data);
        }
        break;
    }

    case cmdLockCreate: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name");
            return TCL_ERROR;
        }
        Object *obj = FindMutex(Tcl_GetString(objv[2]));
        if (!obj) {
            obj = new Object(Tcl_GetString(objv[2]));
            obj->IncRef();
            pthread_mutex_lock(&lockmutex);
            lockqueue.push_back(obj);
            pthread_mutex_unlock(&lockmutex);
        }
        obj->DecRef();
        break;
    }

    case cmdLock: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name");
            return TCL_ERROR;
        }
        Object *obj = FindMutex(Tcl_GetString(objv[2]));
        if (obj) {
            obj->Lock();
            obj->DecRef();
        }
        break;
    }

    case cmdUnlock: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name");
            return TCL_ERROR;
        }
        Object *obj = FindMutex(Tcl_GetString(objv[2]));
        if (obj) {
            obj->Unlock();
            obj->DecRef();
        }
        break;
    }

    case cmdLockRemove:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name");
            return TCL_ERROR;
        }
        pthread_mutex_lock(&lockmutex);
        for (int i = 0; i < lockqueue.size(); i++) {
            if (!strcmp(lockqueue[i]->GetName(), Tcl_GetString(objv[2]))) {
                lockqueue[i]->DecRef();
                lockqueue.erase(lockqueue.begin() + i);
                break;
            }
        }
        pthread_mutex_lock(&lockmutex);
        break;

    case cmdLockList:
        pthread_mutex_lock(&lockmutex);
        for (int i = 0; i < lockqueue.size(); i++) {
             Tcl_AppendResult(interp, lockqueue[i]->GetName(), " ", 0);
        }
        pthread_mutex_unlock(&lockmutex);
        break;

    case cmdObjectCreate:
        for (int i = 2; i < objc; i++) {
            if ((obj = systemObject->FindObject(Tcl_GetString(objv[i])))) {
                obj->DecRef();
            } else {
               obj = new DynamicObject(Tcl_GetString(objv[i]));
               systemObject->AddObject(obj);
            }
        }
        break;

    case cmdObjectRemove:
        for (int i = 2; i < objc; i++) {
            if ((obj = systemObject->FindObject(Tcl_GetString(objv[i])))) {
                obj->DecRef();
                systemObject->RemoveObject(obj);
            }
        }
        break;

    case cmdObjectRefCount:
        for (int i = 2; i < objc; i++) {
            if ((obj = systemObject->FindObject(Tcl_GetString(objv[i])))) {
                Tcl_AppendResult(interp, intString(obj->RefCount()), " ", 0);
                obj->DecRef();
            }
        }
        break;

    case cmdWaitForObject:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "name timeout");
            return TCL_ERROR;
        }
        systemObject->WaitForObject(Tcl_GetString(objv[2]), atoi(Tcl_GetString(objv[3])));
        break;

    case cmdFileOpen: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "file ?mode?");
            return TCL_ERROR;
        }
        int fd = open(Tcl_GetString(objv[2]), O_RDWR|O_CREAT, 0644);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(fd));
        break;
    }

    case cmdFileClose:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "fd");
            return TCL_ERROR;
        }
        close(atoi(Tcl_GetString(objv[2])));
        break;

    case cmdFileRead: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "fd ?size?");
            return TCL_ERROR;
        }
        int fd = atoi(Tcl_GetString(objv[2]));
        char buf[BUFFER_SIZE+1];
        int nread, size = MAXINT;
        if (objc > 3) {
            size = atoi(Tcl_GetString(objv[3]));
        }
        Tcl_DString ds;
        Tcl_DStringInit(&ds);
        while (size > 0) {
          nread = read(fd, buf, MIN(size, sizeof(buf)));
          if (nread <= 0) {
              break;
          }
          Tcl_DStringAppend(&ds, buf, nread);
          size -= nread;
        }
        Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char*)ds.string, ds.length));
        Tcl_DStringFree(&ds);
        break;
    }

    case cmdFileWrite: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "fd data");
            return TCL_ERROR;
        }
        int size, nwrite, sent = 0;
        int fd = atoi(Tcl_GetString(objv[2]));
        unsigned char *data = Tcl_GetByteArrayFromObj(objv[3], &size);
        while (size > 0) {
            nwrite = write(fd, data + sent, size);
            if (nwrite <= 0) {
                break;
            }
            sent += nwrite;
            size -= nwrite;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(sent));
        break;
    }

    case cmdConfig:{
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name ?default?");
            return TCL_ERROR;
        }
        if (objc > 3) {
            Tcl_SetResult(interp, Tcl_GetString(objv[3]), TCL_VOLATILE);
        }
        const char *value = systemObject->configGetValue(Tcl_GetString(objv[2]), 0);
        if (*value) {
            Tcl_SetResult(interp, (char *) value, TCL_VOLATILE);
        }
        break;
    }

    case cmdConfigSet:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "name value");
            return TCL_ERROR;
        }
        systemObject->configSetValue(Tcl_GetString(objv[2]), Tcl_GetString(objv[3]));
        break;

    case cmdConfigList:
        systemObject->configList(interp, objc > 2 ? Tcl_GetString(objv[2]) : 0);
        break;

    case cmdKey:{
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name");
            return TCL_ERROR;
        }
        Tcl_AppendResult(interp, systemObject->KeyToAction(nameToKey(Tcl_GetString(objv[2]))), 0);
        break;
    }

    case cmdKeyList:
        Tcl_AppendResult(interp, systemObject->KeyBindings().c_str(),0);
        break;

    case cmdKeySet:{
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "name value");
            return TCL_ERROR;
        }
        systemObject->KeyBind(Tcl_GetString(objv[2]), Tcl_GetString(objv[3]));
        break;
    }

    case cmdRun:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "command");
            return TCL_ERROR;
        }
        if (Script_Run(Tcl_GetString(objv[2]), true)) {
            Tcl_AppendResult(interp, "Failed to create run thread", 0);
            return TCL_ERROR;
        }
        break;

    case cmdExec: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "command");
            return TCL_ERROR;
        }
        char *arg = strdup(Tcl_GetString(objv[2]));
        if (threadCreate(run_process, (void *) arg)) {
            Tcl_AppendResult(interp, "Failed to create exec thread", 0);
            free(arg);
            return TCL_ERROR;
        }
        break;
    }

    case cmdPagePrev:
        Tcl_AppendResult(interp, systemObject->GetPrevPage(), 0);
        break;

    case cmdPageNew:
        Tcl_AppendResult(interp, systemObject->GetNewPage(), 0);
        break;

    case cmdPageGet:
        if ((page = systemObject->GetCurrentPage())) {
            Tcl_AppendResult(interp, page->GetName(), 0);
            page->DecRef();
        }
        break;

    case cmdPageSet:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "page");
            return TCL_ERROR;
        }
        systemObject->PostEvent("switchpage", Tcl_GetString(objv[2]), NULL);
        break;

    case cmdShowOverlay:
        systemObject->PostEvent("showoverlay", objc > 2 ? Tcl_GetString(objv[2]) : NULL, NULL);
        break;

    case cmdQuit:
        systemObject->Quit();
        break;

    case cmdIsQuit:
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->IsQuit()));
        break;

    case cmdIsShutdown:
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->IsShutdown()));
        break;

    case cmdEventPost:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "actionname ?data1? ?data2?");
            return TCL_ERROR;
        }
        systemObject->PostEvent(Tcl_GetString(objv[2]),
                                objc > 3 ? Tcl_GetString(objv[3]) : 0,
                                objc > 4 ? Tcl_GetString(objv[4]) : 0);
        break;

    case cmdCall:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "objectname methodname ?param1 param2 ...?");
            return TCL_ERROR;
        }
        if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
            Variant vargs[objc - 4];
            for (int argc = 0; argc < objc - 4 && argc < 32; argc++) {
                vargs[argc] = Tcl_GetString(objv[argc + 4]);
            }
            Variant result = obj->MethodCall(Tcl_GetString(objv[3]), objc - 4, vargs);
            if (result != VARNULL) {
                Tcl_AppendResult(interp, result.c_str(), 0);
            }
            obj->DecRef();
        }
        break;

    case cmdPropertyExists:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "objname propertyname");
            return TCL_ERROR;
        }
        obj = systemObject->FindObject(Tcl_GetString(objv[2]));
        if (obj) {
            Tcl_SetObjResult(interp, Tcl_NewIntObj(obj->HasProperty(Tcl_GetString(objv[3]))));
            obj->DecRef();
        } else {
            Tcl_AppendResult(interp, "0", 0);
        }
        break;

    case cmdPropertyGet:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "objname propertyname ?default?");
            return TCL_ERROR;
        }
        if (objc > 4) {
            Tcl_SetResult(interp, Tcl_GetString(objv[4]), TCL_VOLATILE);
        }
        if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
            Variant value = obj->GetPropertyValue(Tcl_GetString(objv[3]));
            if (value != VARNULL) {
                Tcl_SetResult(interp, (char*)value.c_str(), TCL_VOLATILE);
            }
            obj->DecRef();
        }
        break;

    case cmdPropertySet:
        if (objc < 5) {
            Tcl_WrongNumArgs(interp, 2, objv, "objname name value ...");
            return TCL_ERROR;
        }
        if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
            for (int argc = 3; argc < objc - 1; argc += 2) {
                obj->SetPropertyValue(Tcl_GetString(objv[argc]), anytovariant(Tcl_GetString(objv[argc+1])));
            }
            obj->DecRef();
        }
        break;

    case cmdPropertyCreate:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "objname propertyname ...");
            return TCL_ERROR;
        }
        if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
            for (int argc = 3; argc < objc; argc++) {
                if (!obj->HasProperty(Tcl_GetString(objv[argc]))) {
                    ObjectProperty *prop = new ObjectProperty(Tcl_GetString(objv[argc]));
                    obj->AddProperty(prop);
                }
            }
            obj->DecRef();
        }
        break;

    case cmdDbOpen: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname ?-dups?");
            return TCL_ERROR;
        }
        int flags = 0;
        for (int i = 3; i < objc; i++) {
             if (!strcmp(Tcl_GetString(objv[i]), "-dups")) {
                 flags |= DB_DUPSORT;
             }
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->dbOpen(Tcl_GetString(objv[2]), flags)));
        break;
    }

    case cmdDbDrop: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname");
            return TCL_ERROR;
        }
        Tcl_NewIntObj(systemObject->dbDrop(Tcl_GetString(objv[2])));
        break;
    }

    case cmdDbGet: {
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname keyname ?default?");
            return TCL_ERROR;
        }
        if (objc > 4) {
            Tcl_SetResult(interp, Tcl_GetString(objv[4]), TCL_VOLATILE);
        }
        char *value = systemObject->dbGet(Tcl_GetString(objv[2]), Tcl_GetString(objv[3]));
        if (value) {
            Tcl_SetResult(interp, value, (Tcl_FreeProc*)lmbox_free);
        }
        break;
    }

    case cmdDbSync:
        systemObject->dbSync();
        break;

    case cmdDbCompact:
        systemObject->dbCompact();
        break;

    case cmdDbNextId:
        Tcl_SetObjResult(interp, Tcl_NewLongObj(systemObject->dbNextId(objc > 2 ? Tcl_GetString(objv[2]) : 0)));
        break;

    case cmdDbList:
        Tcl_SetResult(interp, systemObject->dbList(), (Tcl_FreeProc*)lmbox_free);
        break;

    case cmdDbExists:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname keyname");
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->dbExists(Tcl_GetString(objv[2]), Tcl_GetString(objv[3]))));
        break;

    case cmdDbPut:
        if (objc < 5) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname keyname value");
            return TCL_ERROR;
        }
        systemObject->dbPut(Tcl_GetString(objv[2]), Tcl_GetString(objv[3]), Tcl_GetString(objv[4]));
        break;

    case cmdDbDel:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname ?keyname?");
            return TCL_ERROR;
        }
        systemObject->dbDel(Tcl_GetString(objv[2]), objc > 3 ? Tcl_GetString(objv[3]) : 0);
        break;

    case cmdDbStat:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname name");
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->dbStat(Tcl_GetString(objv[2]), Tcl_GetString(objv[3]))));
        break;

    case cmdDbEval: {
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "dbname script ?-start keyname? ?-split? ?-stoponerror? ?-array name? ?-regex str? ?-match str? ?-beginwith str? ?-end str? ?-desc?");
            return TCL_ERROR;
        }
        dbEvalParam param;
        memset(&param, 0, sizeof(param));
        param.interp = interp;
        param.dbname = Tcl_GetString(objv[2]);
        param.script = Tcl_GetString(objv[3]);
        for (int i = 4; i < objc; i++) {
             if (!strcmp(Tcl_GetString(objv[i]), "-desc")) {
                 param.op_next = DB_PREV;
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-last")) {
                 param.op_start = DB_LAST;
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-split")) {
                 param.flags |= DBEVAL_SPLIT;
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-stoponerror")) {
                 param.flags |= DBEVAL_STOP;
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-start")) {
                 if (i < objc - 1) {
                     param.start = Tcl_GetString(objv[++i]);
                 }
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-regex")) {
                 if (i < objc - 1) {
                     param.regex = parseRegex(Tcl_GetString(objv[++i]));
                 }
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-match")) {
                 if (i < objc - 1) {
                     param.match = Tcl_GetString(objv[++i]);
                 }
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-array")) {
                 if (i < objc - 1) {
                     param.array = Tcl_GetString(objv[++i]);
                     param.flags |= DBEVAL_SPLIT;
                 }
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-beginwith")) {
                 if (i < objc - 1) {
                     param.beginwith = Tcl_GetString(objv[++i]);
                     param.beginlen = strlen(param.beginwith);
                 }
             } else
             if (!strcmp(Tcl_GetString(objv[i]), "-end")) {
                 if (i < objc - 1) {
                     param.end = Tcl_GetString(objv[++i]);
                 }
             }
        }
        systemObject->dbEval(Tcl_GetString(objv[2]), &param, Script_DbEval, param.start, param.op_start, param.op_next);
        freeRegex(param.regex);
        break;
    }

    case cmdEventCreate:
        if (objc < 5) {
            Tcl_WrongNumArgs(interp, 2, objv, "objname eventname script");
            return TCL_ERROR;
        }
        if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
            const char *name = Tcl_GetString(objv[3]);
            EventHandler *handler = obj->FindEventHandler(name);
            if (!handler) {
                Script *script = new Script(name, Tcl_GetString(objv[4]));
                handler = new EventHandler(name, script);
                obj->RegisterEventHandler(handler);
            } else {
                systemObject->Log(0, "EventCreate: %s already has event handler for %s", obj->GetName(), name);
            }
            handler->DecRef();
            obj->DecRef();
        }
        break;

    case cmdEventRemove:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "objname eventname");
            return TCL_ERROR;
        }
        if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
            EventHandler *handler = obj->FindEventHandler(Tcl_GetString(objv[3]));
            if (handler) {
                obj->UnregisterEventHandler(handler);
                handler->DecRef();
            }
            obj->DecRef();
        }
        break;

    case cmdEventObject:
        Tcl_AppendResult(interp, Tcl_GetVar(interp, "lmbox_object", 0), 0);
        break;

    case cmdEventGet:
        Tcl_SetResult(interp, systemObject->GetEvent(), (Tcl_FreeProc*)lmbox_free);
        break;

    case cmdEventDataGet:
        Tcl_SetResult(interp, systemObject->GetEventData(objc > 2 ? atoi(Tcl_GetString(objv[2])) : 0), (Tcl_FreeProc*)lmbox_free);
        break;

    case cmdEventFire:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "objname eventname");
            return TCL_ERROR;
        }
        if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
            obj->FireEvent(Tcl_GetString(objv[3]));
            obj->DecRef();
        }
        break;

    case cmdVarIncr: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name");
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(systemObject->varIncr(Tcl_GetString(objv[2]))));
        break;
    }

    case cmdVarGet: {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name ?default?");
            return TCL_ERROR;
        }
        if (objc > 3) {
            Tcl_SetResult(interp, Tcl_GetString(objv[3]), TCL_VOLATILE);
        }
        char *val = (char *) systemObject->varGet(Tcl_GetString(objv[2]));
        if (val) {
            Tcl_SetResult(interp, val, TCL_VOLATILE);
        }
        break;
    }

    case cmdVarSet:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "name value ...");
            return TCL_ERROR;
        }
        for (int argc = 2; argc < objc - 1; argc += 2) {
            systemObject->varSet(Tcl_GetString(objv[argc]), Tcl_GetString(objv[argc+1]));
        }
        break;

    case cmdVarAppend:
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "name value value ...");
            return TCL_ERROR;
        }
        for (int argc = 3; argc < objc; argc++) {
            systemObject->varAppend(Tcl_GetString(objv[2]), Tcl_GetString(objv[argc]));
        }
        break;

    case cmdVarUnset:
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "pattern ...");
            return TCL_ERROR;
        }
        for (int argc = 2; argc < objc; argc++) {
            systemObject->varUnset(Tcl_GetString(objv[argc]));
        }
        break;

    case cmdVarList:
        systemObject->varList(interp, objc >= 3 ? Tcl_GetString(objv[2]) : 0);
        break;

    case cmdStripHtml: {
        char *data;
        static char **tags = 0;

        if(objc < 3) {
          Tcl_WrongNumArgs(interp, 2, objv,"text ?tags?");
          return TCL_ERROR;
        }
        data = stripHtmlString(Tcl_GetString(objv[2]), tags);
        Tcl_SetResult(interp, data, (Tcl_FreeProc*)lmbox_free);
        break;
    }

    case cmdXmlParse:{
        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "?-data data? ?-file file? ?-startelement func? ?-endelement func? ?-characters func?");
            return TCL_ERROR;
        }
        XmlParser parser;
        memset(&parser, 0, sizeof(parser));
        parser.interp = interp;
        for (int i = 2; i < objc - 1; i += 2) {
            if (!strcmp(Tcl_GetString(objv[i]), "-file")) {
                parser.file = Tcl_GetString(objv[i + 1]);
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-data")) {
                parser.data = Tcl_GetString(objv[i + 1]);
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-startelement")) {
                parser.startElement = Tcl_GetString(objv[i + 1]);
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-endelement")) {
                parser.endElement = Tcl_GetString(objv[i + 1]);
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-characters")) {
                parser.characters = Tcl_GetString(objv[i + 1]);
            }
        }
        if (parser.data) {
            XML_Parser xml(parser.data, strlen(parser.data));
            xml.SetCallbacks(&parser, XmlStartElement, XmlEndElement, XmlCharacters);
            xml.WalkNode(xml.GetRoot());
        } else
        if (parser.file) {
            XML_Parser xml(parser.file);
            xml.SetCallbacks(&parser, XmlStartElement, XmlEndElement, XmlCharacters);
            xml.WalkNode(xml.GetRoot());
        } else {
            Tcl_AppendResult(interp, "-file or -data should be specified", 0);
            return TCL_ERROR;
        }
        break;
    }

    case cmdListen:{
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "port script ?-thread 0|1? ?-udp 0|1? ?-oninit data? ?-onexit data? ?-seekend 1? -async 0|1? ?-interval secs?");
            return TCL_ERROR;
        }
        Listen *lst = new Listen();
        lst->port = strdup(Tcl_GetString(objv[2]));
        lst->script = strdup(Tcl_GetString(objv[3]));
        if (lst->port[0] == '/') {
            lst->type = SOCK_FILE;
        }
        for (int i = 4; i < objc - 1; i += 2) {
            if (!strcmp(Tcl_GetString(objv[i]), "-seekend")) {
                if (atoi(Tcl_GetString(objv[i + 1]))) {
                    lst->flags |= LISTEN_SEEK_END;
                }
            }
            if (!strcmp(Tcl_GetString(objv[i]), "-thread")) {
                if (atoi(Tcl_GetString(objv[i + 1]))) {
                    lst->flags |= LISTEN_THREAD;
                }
            }
            if (!strcmp(Tcl_GetString(objv[i]), "-async")) {
                if (atoi(Tcl_GetString(objv[i + 1]))) {
                    lst->flags |= LISTEN_ASYNC;
                }
            }
            if (!strcmp(Tcl_GetString(objv[i]), "-udp")) {
                lst->type = atoi(Tcl_GetString(objv[i + 1])) ? SOCK_DGRAM : lst->type;
            }
            if (!strcmp(Tcl_GetString(objv[i]), "-interval")) {
                lst->interval = atoi(Tcl_GetString(objv[i + 1]));
            }
            if (!strcmp(Tcl_GetString(objv[i]), "-oninit")) {
                lmbox_free(lst->oninit);
                lst->oninit = strdup(Tcl_GetString(objv[i + 1]));
            }
            if (!strcmp(Tcl_GetString(objv[i]), "-onexit")) {
                lmbox_free(lst->onexit);
                lst->onexit = strdup(Tcl_GetString(objv[i + 1]));
            }
        }
        // Open listen socket
        if (lst->Setup() == -1) {
            delete lst;
            break;
        }
        pthread_mutex_lock(&listenmutex);
        listenqueue.push_back(lst);
        pthread_mutex_unlock(&listenmutex);
        break;
    }

    case cmdListenInit:
        if (threadCreate(listener_thread, 0)) {
            Tcl_AppendResult(interp, "Failed to start listener thread", 0);
            return TCL_ERROR;
        }
        break;

    case cmdListenClose: {
        int running = 1;
        time_t now = time(0);
        int timeout = systemObject->configGetInt("shutdown_timeout", 10);
        while (running && time(0) - now < timeout) {
            pthread_mutex_lock(&listenmutex);
            running = listenthread;
            pthread_mutex_unlock(&listenmutex);
            sleep(1);
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(running));
        break;
    }

    case cmdListenList:{
        string rc;
        char buf[1024];
        Listen *lst;
        pthread_mutex_lock(&listenmutex);
        for (int j = 0; j < listenqueue.size(); j++) {
            lst = listenqueue[j];
            snprintf(buf, sizeof(buf), "listen %s {%s} -thread %d -udp %d -seekend %d -async %d -interval %d ",
                     lst->port, lst->script, lst->flags & LISTEN_THREAD,
                     lst->type == SOCK_DGRAM, lst->flags & LISTEN_SEEK_END,
                     lst->flags & LISTEN_ASYNC, lst->interval);
            rc += buf;
            if (lst->oninit) {
               rc += "-oninit {";
               rc += lst->oninit;
               rc += "} ";
            }
            if (lst->onexit) {
               rc += "-onexit {";
               rc += lst->onexit;
               rc += "} ";
            }
            rc += "\n";
        }
        pthread_mutex_unlock(&listenmutex);
        Tcl_AppendResult(interp, rc.c_str(), 0);
        break;
    }

    case cmdListenRemove:{
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "port ?-udp?");
            return TCL_ERROR;
        }
        int type = SOCK_STREAM;
        char *port = Tcl_GetString(objv[2]);
        if (port[0] == '/') {
            type = SOCK_FILE;
        }
        if (objc > 3 && !strcmp(Tcl_GetString(objv[3]),"-udp")) {
            type = SOCK_DGRAM;
        }
        vector < Listen * >::iterator iter;
        pthread_mutex_lock(&listenmutex);
        for (iter = listenqueue.begin(); iter != listenqueue.end(); iter++) {
            Listen *lst = *iter;
            if (!strcmp(lst->port,port) && lst->type == type) {
                listenqueue.erase(iter);
                delete lst;
                break;
            }
        }
        pthread_mutex_unlock(&listenmutex);
        break;
    }


    case cmdScheduleInit:
        if (threadCreate(scheduler_thread, 0)) {
            Tcl_AppendResult(interp, "Failed to start scheduler thread", 0);
            return TCL_ERROR;
        }
        break;

    case cmdScheduleClose: {
        int running = 1;
        time_t now = time(0);
        int timeout = systemObject->configGetInt("shutdown_timeout", 30);
        while (running && time(0) - now < timeout) {
            pthread_mutex_lock(&schedmutex);
            running = schedthread;
            pthread_mutex_unlock(&schedmutex);
            sleep(1);
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(running));
        break;
    }

    case cmdSchedule:{
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "script ?-id id? ?-min mins? ?-hour hrs? ?-day day? ?-mon mon? ?-year year? ?-dow dow? ?-interval secs? ?-thread 0|1? ?-once 0|1? ?-enable 0|1? ?-timestamp secs?");
            return TCL_ERROR;
        }
        Schedule *sched = 0;
        for (int i = 3; i < objc - 1; i += 2) {
            if (!strcmp(Tcl_GetString(objv[i]), "-min")) {
                int v = atoi(Tcl_GetString(objv[i + 1]));
                if (v < 0 || v >= 60) {
                    continue;
                }
                if (!sched) {
                    sched = new Schedule();
                }
                sched->min[v] = 1;
                sched->min[sizeof(sched->min) - 1] = 1;
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-hour")) {
                int v = atoi(Tcl_GetString(objv[i + 1]));
                if (v < 0 || v >= 24) {
                    continue;
                }
                if (!sched) {
                    sched = new Schedule();
                }
                sched->hour[v] = 1;
                sched->hour[sizeof(sched->hour) - 1] = 1;
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-day")) {
                int v = atoi(Tcl_GetString(objv[i + 1]));
                if (v < 0 || v >= 32) {
                    continue;
                }
                if (!sched) {
                    sched = new Schedule();
                }
                sched->mday[v] = 1;
                sched->mday[sizeof(sched->mday) - 1] = 1;
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-mon")) {
                int v = atoi(Tcl_GetString(objv[i + 1]));
                if (v < 1 || v > 12) {
                    continue;
                }
                if (!sched) {
                    sched = new Schedule();
                }
                sched->mon[v - 1] = 1;
                sched->mon[sizeof(sched->mon) - 1] = 1;
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-year")) {
                int v = atoi(Tcl_GetString(objv[i + 1]));
                if (v < 2005 || v >= 2015) {
                    continue;
                }
                if (!sched) {
                    sched = new Schedule();
                }
                sched->year[v - 2005] = 1;
                sched->year[sizeof(sched->year) - 1] = 1;
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-dow")) {
                int v = atoi(Tcl_GetString(objv[i + 1]));
                if (v < 0 || v >= 7) {
                    continue;
                }
                if (!sched) {
                    sched = new Schedule();
                }
                sched->wday[v] = 1;
                sched->wday[sizeof(sched->wday) - 1] = 1;
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-interval")) {
                if (!sched) {
                    sched = new Schedule();
                }
                sched->interval = atoi(Tcl_GetString(objv[i + 1]));
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-timestamp")) {
                if (!sched) {
                    sched = new Schedule();
                }
                sched->timestamp = atol(Tcl_GetString(objv[i + 1]));
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-thread")) {
                if (!sched) {
                    sched = new Schedule();
                }
                sched->thread_flag = atoi(Tcl_GetString(objv[i + 1]));
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-once")) {
                if (!sched) {
                    sched = new Schedule();
                }
                sched->once_flag = atoi(Tcl_GetString(objv[i + 1]));
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-enable")) {
                if (!sched) {
                    sched = new Schedule();
                }
                sched->enable_flag = atoi(Tcl_GetString(objv[i + 1]));
            } else
            if (!strcmp(Tcl_GetString(objv[i]), "-id")) {
                if (!sched) {
                    sched = new Schedule();
                }
                sched->id = atol(Tcl_GetString(objv[i + 1]));
            }
        }
        // Check if we have schedule with given id already
        vector < Schedule * >::iterator iter;
        pthread_mutex_lock(&schedmutex);
        for (iter = schedqueue.begin(); sched && iter != schedqueue.end(); iter++) {
            Schedule *s = *iter;
            if (s->id == sched->id) {
                Tcl_AppendResult(interp, "schedule ",sched->script," with id ", intString(sched->id)," exists", 0);
                delete sched;
                sched = 0;
            }
        }
        pthread_mutex_unlock(&schedmutex);
        if (!sched || sched->Setup()) {
            Tcl_AppendResult(interp, "unable to setup schedule: invalid id or date/time", 0);
            if (sched) {
                delete sched;
            }
            return TCL_ERROR;
        }
        sched->script = strdup(Tcl_GetString(objv[2]));
        pthread_mutex_lock(&schedmutex);
        schedqueue.push_back(sched);
        pthread_mutex_unlock(&schedmutex);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(sched->id));
        break;
    }

    case cmdScheduleRemove:{
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "id");
            return TCL_ERROR;
        }
        unsigned long id = atol(Tcl_GetString(objv[2]));
        vector < Schedule * >::iterator iter;
        pthread_mutex_lock(&schedmutex);
        for (iter = schedqueue.begin(); iter != schedqueue.end(); iter++) {
            Schedule *sched = *iter;
            if (sched->id == id) {
                schedqueue.erase(iter);
                delete sched;
                break;
            }
        }
        pthread_mutex_unlock(&schedmutex);
        break;
    }

    case cmdScheduleList:{
        string rc;
        char buf[1024];
        Schedule *sched;
        pthread_mutex_lock(&schedmutex);
        for (int j = 0; j < schedqueue.size(); j++) {
            sched = schedqueue[j];
            // General parameters
            snprintf(buf, sizeof(buf),
                     "schedule {%s} -id %lu -enable %d -once %d -thread %d -interval %d ",
                     sched->script, sched->id, sched->enable_flag,
                     sched->once_flag, sched->thread_flag, sched->interval);
            rc += buf;
            // Show next run timestamp
            if (sched->interval) {
                sprintf(buf, "-timestamp %lu ", sched->timestamp);
                rc += buf;
            }
            for (int i = 0; i < 60; i++) {
                if (sched->min[i] != 1) {
                    continue;
                }
                sprintf(buf, "-min %d ", i);
                rc += buf;
            }
            for (int i = 0; i < 24; i++) {
                if (sched->hour[i] != 1) {
                    continue;
                }
                sprintf(buf, "-hour %d ", i);
                rc += buf;
            }
            for (int i = 0; i < 12; i++) {
                if (sched->mon[i] != 1) {
                    continue;
                }
                sprintf(buf, "-mon %d ", i + 1);
                rc += buf;
            }
            for (int i = 0; i < 10; i++) {
                if (sched->year[i] != 1) {
                    continue;
                }
                sprintf(buf, "-year %d ", i + 2005);
                rc += buf;
            }
            for (int i = 0; i < 31; i++) {
                if (sched->mday[i] != 1) {
                    continue;
                }
                sprintf(buf, "-mday %d ", i);
                rc += buf;
            }
            for (int i = 0; i < 7; i++) {
                if (sched->wday[i] != 1) {
                    continue;
                }
                sprintf(buf, "-dow %d ", i);
                rc += buf;
            }
            rc += "\n";
        }
        pthread_mutex_unlock(&schedmutex);
        Tcl_AppendResult(interp, rc.c_str(), 0);
        break;
    }

    case cmdUdpSend:{
         fd_set fds;
         char buf[BUFFER_SIZE];
         struct timeval tv;
         struct sockaddr_in sa;
         socklen_t salen = sizeof(sa);
         char *address = 0, *port = 0, *data = 0;
         int i, fd, len, timeout = 5, retries = 1, noreply = 0;

         if (objc < 5) {
             Tcl_WrongNumArgs(interp, 1, objv, "ipaddr port data ?-timeout N? ?-retries N? ?-noreply 1|0?");
             return TCL_ERROR;
         }
         address = Tcl_GetString(objv[2]);
         port = Tcl_GetString(objv[3]);
         data = Tcl_GetStringFromObj(objv[4],&len);
         for (i = 5; i < objc-1; i += 2) {
             if (!strcmp(Tcl_GetString(objv[i]),"-timeout")) {
                 timeout = atoi(Tcl_GetString(objv[i+1]));
                 continue;
             }
             if (!strcmp(Tcl_GetString(objv[i]),"-retries")) {
                 retries = atoi(Tcl_GetString(objv[i+1]));
                 continue;
             }
             if (!strcmp(Tcl_GetString(objv[i]),"-noreply")) {
                 noreply = atoi(Tcl_GetString(objv[i+1]));
                 continue;
             }
             break;
         }
         sa.sin_family = AF_INET;
         sa.sin_port = htons(atoi(port));
         sa.sin_addr.s_addr = inet_addr(address);
         fd = socket(AF_INET, SOCK_DGRAM, 0);
         if (fd < 0) {
             Tcl_AppendResult(interp, "socket error: ", address, ":", port, " ", strerror(errno), 0);
             return TCL_ERROR;
         }
         // To support brodcasting
         i = 1;
         setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &i, sizeof(int));
udp_send:
         if (sendto(fd, data, len, 0,(struct sockaddr*)&sa, sizeof(sa)) < 0) {
             Tcl_AppendResult(interp, "sendto error: ", address, ":", port, " ", strerror(errno), 0);
             close(fd);
             return TCL_ERROR;
         }
         if (noreply) {
             close(fd);
             return TCL_OK;
         }
         memset(buf,0,sizeof(buf));
udp_wait:
         FD_ZERO(&fds);
         FD_SET(fd,&fds);
         tv.tv_sec = timeout;
         tv.tv_usec = 0;
         len = select(fd+1, &fds, 0, 0, &tv);
         switch (len) {
          case -1:
              if (errno == EINTR || errno == EINPROGRESS || errno == EAGAIN) {
                  goto udp_wait;
              }
              Tcl_AppendResult(interp, "select error: ", address, ":", port, " ", strerror(errno), 0);
              close(fd);
              return TCL_ERROR;

          case 0:
              if (--retries < 0) {
                  goto udp_send;
              }
              Tcl_AppendResult(interp, "timeout: ", address, ":", port, 0);
              close(fd);
              return TCL_ERROR;
         }
         if (FD_ISSET(fd, &fds)) {
             len = recvfrom(fd, buf, sizeof(buf)-1, 0, (struct sockaddr*)&sa, &salen);
             if (len > 0) {
                 Tcl_AppendResult(interp, buf, 0);
             }
         }
         close(fd);
         break;
    }

#ifdef HAVE_XTEST
    case cmdXEvent: {
         // borrowed from Android Extensions to Tk
         // Transmits events to the X server, which will translate them and send them on to other
         // applications as if the user had triggered with the keyboard or mouse.
         //
         //  xevent ?obj objname? ?mouse x,y? ?key keysym? ?keydn keysym? ?keyup keysym? ?btndn btn? ?btnup btn? ?click btn? ?string {string}?
         //
         // x,y     - x and y coordinates for mouse
         // keysym  - X11 key symbol from /usr/X11R6/lib/X11/XKeysymDB
         // btn     - mouse button number
         // objname - object name whos X11 display to use
         Display *display = 0;
         int value, shift, x, y;

         for (int i = 2; i < objc;i += 2) {
             char *op = Tcl_GetString(objv[i]);
             char *arg = Tcl_GetString(objv[i+1]);

             if (!strcmp("obj", op)) {
                 if ((obj = systemObject->FindObject(Tcl_GetString(objv[2])))) {
                     display = obj->GetDisplay();
                     obj->DecRef();
                 }
             }
             if (!display) {
                 display = systemObject->GetDisplay();
             }
             if (!strncmp("key", op, 3)) {
                 if (!(value = atoi(arg))) {
                     if ((value = XStringToKeysym(arg)) != NoSymbol) {
                         value = XKeysymToKeycode(display, value);
                     }
                 }
                 if (!value) {
                     Tcl_AppendResult(interp,"unknown key ",arg,0);
                     return TCL_ERROR;
                 }
                 switch(op[3]) {
                  case 'd':
                      XTestFakeKeyEvent(display, value, True, 0);
                      break;
                  case 'u':
                      XTestFakeKeyEvent(display, value, False, 0);
                      break;
                  default:
                      XTestFakeKeyEvent(display, value, True, 0);
                      XTestFakeKeyEvent(display, value, False, 0);
                      break;
                 }
             } else

             if (!strcmp("mouse", op)) {
                 x = atoi(arg);
                 if ((arg = strchr(arg, ','))) {
                      y = 0;
                 } else {
                      y = atoi(++arg);
                 }
                 XTestFakeMotionEvent(display, 0, x, y, 0);
             } else

             if (!strcmp("rmouse", op)) {
                 x = atoi(arg);
                 if ((arg = strchr(arg, ','))) {
                      y = 0;
                 } else {
                      y = atoi(++arg);
                 }
                 XTestFakeRelativeMotionEvent(display, x, y, 0);
             } else

             if (!strcmp(op,"btndn")) {
                 x = atoi(arg);
                 XTestFakeButtonEvent(display, x, True, 0);
             } else

             if (!strcmp(op, "btnup")) {
                 x = atoi(arg);
                 XTestFakeButtonEvent(display, x, False, 0);
             } else

             if (!strcmp(op, "click")) {
                 x = atoi(arg);
                 XTestFakeButtonEvent(display, x, True, 0);
                 XTestFakeButtonEvent(display, x, False, 0);
             } else

             if (!strcmp(op,"string")) {
                 for (i = 0; arg[i]; i++) {
                     shift = 0;
                     if (strchr(shifted, arg[i])) {
                         shift = XStringToKeysym("Shift_L");
                         shift = XKeysymToKeycode(display, shift);
                         XTestFakeKeyEvent(display, shift, True, 0);
                     }
                     value = XStringToKeysym(latin_xlt[(int)arg[i] - (int)' ']);
                     value = XKeysymToKeycode(display, value);
                     XTestFakeKeyEvent(display, value, True, 0);
                     XTestFakeKeyEvent(display, value, False, 0);
                     if (shift) {
                         XTestFakeKeyEvent(display, shift, False, 0);
                     }
                 }
             }
             XFlush(display);
         }
    }
#endif
    }
    return TCL_OK;
}

static void XmlStartElement(void *userData, XML_Node *node)
{
    XmlParser *parser = (XmlParser *) userData;
    string cmd = parser->startElement;
    cmd += " {";
    cmd += node->name;
    cmd += "} ";
    for (XML_Property *p = node->properties; p; p = p->next) {
        cmd += " {";
        cmd += p->name;
        cmd += "} {";
        cmd += p->value ? p->value : "";
        cmd += "}";
    }
    if (Tcl_EvalEx(parser->interp, cmd.c_str(), -1, 0) != TCL_OK) {
        Script_Error(parser->interp, 0);
    }
}

static void XmlEndElement(void *userData, XML_Node *node)
{
    XmlParser *parser = (XmlParser *) userData;
    string cmd = parser->endElement;
    cmd += " {";
    cmd += node->name;
    cmd += "}";
    if (Tcl_EvalEx(parser->interp, cmd.c_str(), -1, 0) != TCL_OK) {
        Script_Error(parser->interp, 0);
    }
}

static void XmlCharacters(void *userData, XML_Node *node)
{
    XmlParser *parser = (XmlParser *) userData;
    string cmd = parser->characters;
    cmd += " {";
    cmd += node->data;
    cmd += "}";
    if (Tcl_EvalEx(parser->interp, cmd.c_str(), -1, 0) != TCL_OK) {
        Script_Error(parser->interp, 0);
    }
}
