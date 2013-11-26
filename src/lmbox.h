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

#ifndef LMBOX_H
#define LMBOX_H

#if !defined(HAVE_XINE) && !defined(HAVE_VLC)
#error "Xine or VLC should be used for player support, none is defined so far"
#endif

#if defined(HAVE_XINE) && defined(HAVE_VLC)
#error "Either one Xine or VLC should be used for player support only"
#endif

#include <map>
#include <sstream>
#include <algorithm>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <iostream>

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <math.h>
#include <regex.h>
#include <values.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/cdrom.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <sys/statvfs.h>

#include <db.h>

#include <tcl.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_syswm.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_endian.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#ifdef HAVE_XINE
#include <xine.h>
#include <xine/xineutils.h>
#endif

#ifdef HAVE_VLC
#include <vlc/vlc.h>
#endif

#ifdef HAVE_MIXER
#include "SDL_mixer.h"
#endif

#ifdef LOAD_PNG
#include <png.h>
#endif

#ifdef HAVE_XTEST
#include <X11/extensions/XTest.h>
#endif

#ifdef HAVE_LIRC
#include <lirc/lirc_client.h>
#endif

#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif

#ifdef HAVE_MOZILLA
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <prenv.h>
#include <gtkmozembed.h>
#include <nsIDOMMouseEvent.h>
#include <dom/nsIDOMKeyEvent.h>
#endif

#define LMBOX_VERSION                   "0.9.6"

#define TLS_STRSIZE                     2
#define BUFFER_SIZE                     2048
#define DEFAULT_FONTSIZE                12
#define DOUBLECLICK_TIME                300
#define TEXTINPUT_DELAY_TIME            1000000

#define AUTOPROPERTY_OFF                0
#define AUTOPROPERTY_ON                 1
#define AUTOPROPERTY_AUTO               2

#define OBJECT_BUSY	       0x00000001
#define OBJECT_SEEKABLE        0x00000002

#define WIDGET_DISABLED        0x00000010
#define WIDGET_SELECTABLE      0x00000020
#define WIDGET_PRESSED         0x00000040
#define WIDGET_INSIDE          0x00000080
#define WIDGET_HIDDEN          0x00000100
#define WIDGET_CHANGED         0x00000200
#define WIDGET_TRANSPARENT     0x00000400
#define WIDGET_FOCUSED         0x00000800
#define WIDGET_TRANSFORM       0x00001000
#define WIDGET_OVERLAY         0x00002000

#define ALIGN_HORIZ_MASK       0x000000FF
#define ALIGN_VERT_MASK        0x0000FF00
#define ALIGN_HORIZ_CENTER     0x00000001
#define ALIGN_HORIZ_RIGHT      0x00000002
#define ALIGN_HORIZ_LEFT       0x00000004
#define ALIGN_VERT_CENTER      0x00000100
#define ALIGN_VERT_TOP         0x00000200
#define ALIGN_VERT_BOTTOM      0x00000400

#define ARROW_UP	       0x00000001
#define ARROW_DOWN             0x00000002
#define ARROW_LEFT             0x00000003
#define ARROW_RIGHT            0x00000004

#define BORDER_NONE            0x00000000
#define BORDER_NORMAL          0x00000001
#define BORDER_ROUND           0x00000002
#define BORDER_ROUND_DBL       0x00000003

#define BACKGROUND_NONE        0x00000000
#define BACKGROUND_NORMAL      0x00000001
#define BACKGROUND_GRADIENT    0x00000002
#define BACKGROUND_TILED       0x00000003
#define BACKGROUND_FILL        0x00000004
#define BACKGROUND_SCALED      0x00000005
#define BACKGROUND_COLOR       0x00000006

#define SOCK_FILE              0x00010001

#define LISTEN_THREAD          0x00000001
#define LISTEN_SEEK_END        0x00000002
#define LISTEN_SEEK_START      0x00000004
#define LISTEN_ASYNC           0x00000008

#define MAX_UPDATES            50
#define WIDGET_LIST_SIZE       16
#define WIDGET_LIST_INCR       16

#define SMOOTHING_OFF           0
#define SMOOTHING_ON            1

#ifndef MAX
#define MAX(x,y)               ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y)               ((x) < (y) ? (x) : (y))
#endif

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define RMask                   0x000000FF
#define GMask                   0x0000FF00
#define BMask                   0x00FF0000
#define AMask                   0xFF000000
#else
#define RMask                   0xFF000000
#define GMask                   0x00FF0000
#define BMask                   0x0000FF00
#define AMask                   0x000000FF
#endif

#define DBEVAL_STOP            0x00000001
#define DBEVAL_SPLIT           0x00000002

#define IDX_SIZE               4
#define IDX_NORMAL             0
#define IDX_FOCUSED            1
#define IDX_PRESSED            2
#define IDX_DISABLED           3

#define TYPE_OBJECT            0
#define TYPE_WIDGET            1
#define TYPE_GFX               2
#define TYPE_LABEL             3
#define TYPE_LISTBOX           4
#define TYPE_CONTAINER         5
#define TYPE_PAGE              6
#define TYPE_SCREEN            7
#define TYPE_SCROLLBAR         8
#define TYPE_CLOCKLABEL        9
#define TYPE_BUTTON            10
#define TYPE_TOGGLEBUTTON      11
#define TYPE_TEXTFIELD         12
#define TYPE_FILEBROWSER       13
#define TYPE_SURFACE           14
#define TYPE_LAYOUT            15
#define TYPE_VBOXLAYOUT        16
#define TYPE_HBOXLAYOUT        17
#define TYPE_EVENTHANDLER      18
#define TYPE_CALLBACK          19
#define TYPE_TIMER             20
#define TYPE_PLAYER            21
#define TYPE_TV                22
#define TYPE_SOUND             23
#define TYPE_RADIO             24
#define TYPE_WEBBROWSER        25
#define TYPE_SYSTEM            26
#define TYPE_DYNAMICOBJECT     27
#define TYPE_FONT              28
#define TYPE_PICTURE           29

using namespace std;

class Script;
class XML_Node;
class GUI_Page;
class GUI_Container;
class DynamicObject;
typedef struct _TTF_Font TTF_Font;
typedef void xmlcallbackfn(void *, XML_Node *);
typedef void scriptcallbackfn(Script *, void *);

typedef struct Tls {
    char ibuf[16];
    char nabuf[32];
    char threadname[32];
    Tcl_Interp *interp;
    DynamicObject *object;
    char *event;
    struct {
      int len;
      char *data;
    } str[TLS_STRSIZE];
    struct {
      int pos;
      char *ptr;
      char data[1];
    } buf;
};

class PositionEntry {
  public:
    int start_time;
    int duration;
    bool skipped;
};

class FileEntry {
  public:
    FileEntry(char *path, time_t s, time_t d, const char *skip);
    FileEntry(const char *path, time_t s, time_t d, const char *skip);
    void ParseSkip(const char *skip);
    char *name(void);

    string file;
    time_t start_time;
    time_t duration;
    vector < PositionEntry > skip_list;
};

class EventEntry {
  public:
    string type;
    string data1;
    string data2;
};

class KeyBinding {
  public:
    KeyBinding(void);
    virtual ~ KeyBinding(void);

    int mod;
    int key;
    char *action;
};

class Variant {
  public:
    Variant(void);
    virtual ~ Variant(void);

    bool IsEmpty(void);
    int GetType(void);
    void Clear(void);
    void SetType(int type);
    void ClearVars(void);
    const char *c_str(void);

    Variant operator+(Variant value);
    Variant operator+=(const char *value);
    Variant operator=(void *value);
    Variant operator=(string value);
    Variant operator=(const char *value);
    Variant operator=(int value);
    Variant operator=(unsigned long value);
    Variant operator=(double value);
    Variant operator=(bool value);
    bool operator==(Variant value);
    bool operator==(string value);
    bool operator==(const char *value);
    bool operator==(int value);
    bool operator==(unsigned long value);
    bool operator==(double value);
    bool operator==(bool value);
    template < class T > bool operator!=(T value) { return !(operator==(value)); }
    operator char *(void);
    operator const char *(void);
    operator string(void);
    operator int (void);
    operator unsigned long (void);
    operator short (void);
    operator double (void);
    operator bool(void);
    operator void *(void);

    friend ostream & operator<<(ostream & stream, Variant v);

  protected:
    bool empty;
    int intvalue;
    int internaltype;
    double dblvalue;
    string stringvalue;
    unsigned long ulongvalue;
};

const Variant VARNULL;

template < class T > Variant anytovariant(T anything)
{
    Variant v;
    v = anything;
    return v;
};

class XML_Property {
public:
    XML_Property(const char *name, const char *value);
    ~XML_Property(void);

    char *name;
    char *value;
    XML_Property *next;
};

class XML_Node {
public:
    XML_Node(const char *name, const char *data);
    ~XML_Node(void);
    char *GetProperty(const char *name);
    int GetPropertyInt(const char *name);

    char *name;
    char *data;
    XML_Property *properties;
    XML_Node *child;
    XML_Node *next;
};

class XML_Parser {
public:
    XML_Parser(const char *file);
    XML_Parser(const char *buf, int size);
    ~XML_Parser(void);
    XML_Node *GetRoot(void);
    const char *GetFile(void);
    void WalkNode(XML_Node * node);
    void PrintNode(XML_Node * node, int indent);
    void SetCallbacks(void *arg, xmlcallbackfn *startcb, xmlcallbackfn *endcb, xmlcallbackfn *datacb);

protected:
    char *lex_buf;
    char *lex_file;
    int lex_bufsize;
    int lex_bufpos;
    int lex_mode;
    int lex_line;
    int in_comment;
    XML_Node *root;
    xmlcallbackfn *startcb;
    xmlcallbackfn *endcb;
    xmlcallbackfn *datacb;
    void *arg;

    char *ParseEntity(char *tok);
    int ParseToken(char *tok, int tok_size);
    int ParseNode(XML_Node * node, char *root_name, int rec);
};

class MethodHandler {
  public:
    MethodHandler(const char *name, int numargs);
    virtual ~ MethodHandler(void);
    virtual Variant Call(int numargs, Variant args[]) = 0;
    const char *GetName(void);

  protected:
    char *name;
    int numargs;
};

class ObjectProperty {
  public:
    ObjectProperty(const char *name);
    virtual ~ ObjectProperty(void);
    const char *GetName(void);
    const char *GetString(void);
    virtual Variant GetValue(void);
    virtual int SetValue(const char *value);
    virtual int SetValue(Variant value);
    virtual void SetRequired(bool required);
    virtual bool IsRequired(void);
    virtual bool IsReadOnly(void);

  protected:
    char *name;
    char *value;
    bool required;
};

class Script {
  public:
    Script(const char *name, const char *script);
    virtual ~ Script(void);
    void SetCallback(void *arg, scriptcallbackfn *callback1, scriptcallbackfn *callback2);
    void SetScript(const char *script);
    void SetLineOffset(int line);
    void Execute(const char *script);
    void Execute(void);
    const char *GetResult(void);
    const char *GetScript(void);
    const char *GetName(void);
    Tcl_Interp *GetInterp(void);
    void SetFlags(int mask);
    void ClearFlags(int mask);
    int GetFlags(void);

  protected:
    void *arg;
    char *name;
    char *code;
    char *result;
    int flags;
    int lineoffset;
    Tcl_Interp *interp;
    scriptcallbackfn *callback1;
    scriptcallbackfn *callback2;
};

class Object {
  public:
    Object(const char *name);
    virtual ~ Object(void);
    const char *GetType(void);
    const char *GetName(void);
    virtual int IsBusy(void);
    virtual int IncRef(void);
    virtual int DecRef(void);
    virtual int RefCount(void);
    virtual void SetRefCount(int count);
    virtual void OnEvent(void);
    virtual void OnStart(void);
    virtual void OnStop(void);
    virtual void OnExpose(void);
    virtual void OnFlags(int oldflags);
    virtual int GetFlags(void);
    virtual void WriteFlags(int andmask, int ormask);
    virtual void SetFlags(int mask);
    virtual void ClearFlags(int mask);
    virtual void ToggleFlags(int mask);
    virtual int Lock(void);
    virtual int TryLock(void);
    virtual int Unlock(void);
    virtual int IsChanged(void);
    virtual void SetChanged(int flag);
    virtual void SetParent(Object * parent);
    virtual Object *GetParent(void);
    virtual int Keep(Object ** target, Object * source);

  protected:
    char *name;
    uint32_t flags;
    uint16_t type;
    int16_t refcount;
    pthread_mutex_t lock;
};

class EventHandler: public Object {
  public:
    EventHandler(const char *name, Script * eventscript);
    virtual ~ EventHandler(void);
    virtual int Fire(void);
    const char *GetScript(void);
    bool IsEqual(const char *str);

  protected:
    Script *script;
};

template < class T > class MemberFunctionProperty:public ObjectProperty {
    typedef int propsethandler(Variant value);
    typedef Variant propgethandler(void);
    typedef propsethandler(T::*propsetmethod);
    typedef propgethandler(T::*propgetmethod);

  public:
    MemberFunctionProperty < T > (const char *name, T * object, propgetmethod getter, propsetmethod setter, bool required):ObjectProperty(name)
    {
        this->object = object;
        this->getter = getter;
        this->setter = setter;
        this->required = required;
    };
    virtual int SetValue(Variant value) {
        if (setter) {
            int rc = (object->*setter) (value);
            return rc;
        }
        cerr << "Property " << name << " is read-only" << endl;
        return -1;
    };
    virtual Variant GetValue(void) {
        if (getter) {
            Variant rc = (object->*getter) ();
            return rc;
        }
        cerr << "Property " << name << " is write-only" << endl;
        return VARNULL;
    };
    virtual bool IsReadOnly(void) { return (setter == NULL); };
  private:
    propgetmethod getter;
    propsetmethod setter;
    T *object;
};

template < class T > class MemberMethodHandler:public MethodHandler {
    typedef Variant handler(int numargs, Variant args[]);
    typedef handler(T::*cmethod);

  public:
    MemberMethodHandler < T > (const char *name, T * object, int nargs, cmethod method):MethodHandler(name, nargs) {
        this->method = method;
        this->object = object;
    }
    virtual Variant Call(int numargs, Variant args[]) {
        if (numargs == this->numargs) {
            Variant rc = (object->*method) (numargs, args);
            return rc;
        }
        cerr << "Wrong number of arguments passed to method " << name << " of object " << object->GetName();
        cerr << " (" << this->numargs << " expected, " << numargs << " supplied)" << endl;
        return VARNULL;
    }

  protected:
    T * object;
    cmethod method;
};

class DynamicObject:public Object {
  public:
    DynamicObject(const char *name);
    virtual ~ DynamicObject(void);
    virtual int Initialize(void);
    virtual bool HandleEvent(const char *action);
    virtual void AddEventHandler(const char *action, const char *code);
    virtual void RegisterEventHandler(EventHandler *handler);
    virtual void UnregisterEventHandler(EventHandler * handler);
    virtual EventHandler *FindEventHandler(const char *name);
    virtual void DestroyEventHandlers(void);
    virtual void RegisterMethod(MethodHandler *method);
    virtual void UnregisterMethod(MethodHandler * handler);
    virtual MethodHandler *GetMethod(int index);
    virtual int GetMethodCount(void);
    virtual int AddMethod(MethodHandler * mh);
    virtual bool HasMethod(const char *method);
    virtual Variant MethodCall(const char *method, int numargs, Variant args[]);
    virtual Variant GetPropertyValue(const char *property);
    virtual int SetPropertyValue(const char *property, Variant value);
    virtual bool HasProperty(const char *property);
    virtual int AddProperty(ObjectProperty * prop);
    virtual int GetPropertyCount(void);
    virtual ObjectProperty *FindProperty(const char *name);
    virtual ObjectProperty *GetProperty(int index);
    virtual const char *GetDefaultProperty(void);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual int FireEvent(const char *event);
    virtual void FireTimer(void);
    virtual void SetTimer(int interval, const char *script);
    virtual const char *GetDefaultEvent(void);
    virtual DynamicObject *FindObject(const char *name);
    virtual void AddObject(DynamicObject * object);
    virtual void RemoveObject(DynamicObject * object);
    virtual void StartObjects(void);
    virtual void StopObjects(void);
    virtual void DestroyObjects(void);
    virtual Display *GetDisplay(void);
    virtual Window GetWindow(void);
    virtual int SendMessage(const char *msg);
    virtual char *ReadMessage(int timeout);

  protected:
    char *data;
    int timer_interval;
    char *timer_script;
    SDL_TimerID timer_id;
    ObjectProperty *defaultproperty;
    vector < EventHandler * >eventhandlers;
    vector < MethodHandler * >methodhandlers;
    vector < DynamicObject * >containedobjects;
    vector < ObjectProperty * >properties;
    queue < char * >msgqueue;
    pthread_cond_t msgcond;
    pthread_mutex_t msglock;
    char *defaultevent;

    Variant pget_Name(void);
    Variant pget_Type(void);
    Variant pget_MethodCount(void);
    Variant pget_Flags(void);
    int pset_Flags(Variant value);
    Variant pget_Data(void);
    int pset_Data(Variant value);
    Variant pget_TimerInterval(void);
    Variant pget_TimerScript(void);
    Variant pget_EventHandlers(void);
    Variant pget_Objects(void);
    Variant pget_Properties(void);
    Variant m_GetProperty(int numargs, Variant args[]);
    Variant m_GetMethod(int numargs, Variant args[]);
    Variant m_SetTimer(int numargs, Variant args[]);
    Variant m_FireTimer(int numargs, Variant args[]);
    Variant m_AddEventHandler(int numargs, Variant args[]);
    Variant m_RemoveEventHandler(int numargs, Variant args[]);
    Variant m_Message(int numargs, Variant args[]);
};

class GUI_Callback:public Object {
  public:
    GUI_Callback(const char *aname) : Object(aname) {};
    virtual ~GUI_Callback(void) {};
    virtual void Call(Object * sender) = 0;
};

template < class T > class GUI_EventHandler:public GUI_Callback {
    typedef void handler(Object * sender);
    typedef handler(T::*method);

  public:
    GUI_EventHandler < T > (T * t, method m):GUI_Callback("event"), o(t), f(m) {}
    virtual void Call(Object * sender) { (o->*f) (sender); }

  protected:
    T *o;
    method f;
};

class GUI_Surface:public Object {
  public:
    GUI_Surface(GUI_Surface * image);
    GUI_Surface(GUI_Surface * image, int w, int h);
    GUI_Surface(const char *aname, SDL_Surface * image);
    GUI_Surface(const char *aname, int f, int w, int h, int d, int rm, int gm, int bm, int am);
    GUI_Surface(const char *fn);
    virtual ~ GUI_Surface(void);
    void DisplayFormat(void);
    void SetAlpha(Uint32 flag, Uint8 alpha);
    void SetColorKey(Uint32 c);
    void Blit(SDL_Rect * src_r, GUI_Surface * dst, SDL_Rect * dst_r);
    void UpdateRects(int n, SDL_Rect * rects);
    void UpdateRect(int x, int y, int w, int h);
    void Fill(const SDL_Rect * r, SDL_Color c);
    void Fill(const SDL_Rect * r, Uint32 c);
    int GetWidth(void);
    int GetHeight(void);
    SDL_Rect GetArea(void);
    Uint32 MapRGB(int r, int g, int b);
    Uint32 MapRGBA(int r, int g, int b, int a);
    int IsDoubleBuffered(void);
    int IsHardware(void);
    void Flip(void);
    SDL_Surface *GetSurface(void);
    void SetSurface(SDL_Surface * image);
    void SaveBMP(const char *filename);
    SDL_Surface *LoadImage(const char *filename);
    SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth);
    int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy, int smooth);
    int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy);
    SDL_Surface *rotozoomSurfaceXY(SDL_Surface * src, double angle, double zoomx, double zoomy, int smooth);
    SDL_Surface *rotozoomSurface(SDL_Surface * src, double angle, double zoom, int smooth);
    void transformSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos, int flipx, int flipy, int smooth);
    void transformSurfaceY(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos);

  protected:
    SDL_Surface * surface;
};

class GUI_Font:public Object {
  public:
    GUI_Font(const char *fn, int size);
    virtual ~ GUI_Font(void);
    GUI_Surface *RenderFast(const char *s, SDL_Color fg);
    GUI_Surface *RenderQuality(const char *s, SDL_Color fg);
    GUI_Surface *RenderColor(const char *s, SDL_Color *fg);
    GUI_Surface **RenderColorList(const char *s, SDL_Color *fg);
    SDL_Rect GetTextSize(const char *s);
    int GetTextWidth(const char *s);

  protected:
    TTF_Font * ttf;
};

class GUI_Widget:public DynamicObject {
  public:
    GUI_Widget(const char *aname, int x, int y, int w, int h);
    virtual ~ GUI_Widget(void);
    virtual void DoUpdate(int force);
    virtual void Update(int force);
    virtual void UpdateBackground(void);
    virtual void UpdateBorder(int force);
    virtual void FlushUpdates(int force);
    virtual void Draw(GUI_Surface * image, const SDL_Rect * sr, const SDL_Rect * dr);
    virtual void Erase(const SDL_Rect * dr);
    virtual void Fill(const SDL_Rect * dr, SDL_Color c);
    virtual bool HandleKeyEvent(const SDL_Event *event);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual void Clicked(int x, int y);
    virtual void DoubleClicked(int x, int y);
    virtual void OnClick(void);
    virtual void OnResize(void);
    virtual void OnTransform(void);
    virtual void OnMove(void);
    virtual void RemoveWidget(GUI_Widget * widget);
    virtual void SetTransparent(bool flag);
    virtual void SetEnabled(bool flag);
    virtual void SetFocused(bool flag);
    virtual void SetInside(bool flag);
    virtual void SetPressed(bool flag);
    virtual void SetVisible(bool flag);
    virtual void SetSelectable(bool flag);
    virtual int IsInside(void);
    virtual int IsFocused(void);
    virtual int IsClickable(void);
    virtual int IsSelectable(void);
    virtual int IsVisible(void);
    virtual int IsPressed(void);
    virtual int IsDisabled(void);
    virtual int IsTransparent(void);
    virtual SDL_Rect GetArea(void);
    virtual SDL_Rect GetAreaRaw(void);
    virtual SDL_Rect GetAreaOrig(void);
    virtual void SetX(int x);
    virtual void SetY(int y);
    virtual int GetX(void);
    virtual int GetY(void);
    virtual void SetContentX(int x);
    virtual void SetContentY(int y);
    virtual int GetContentX(void);
    virtual int GetContentY(void);
    virtual int GetWidth(void);
    virtual int GetHeight(void);
    virtual int GetAlpha(void);
    virtual void SetArea(SDL_Rect r);
    virtual void SetWidth(int w);
    virtual void SetHeight(int h);
    virtual void SetBorderX(int pixels);
    virtual void SetBorderY(int pixels);
    virtual void SetAlpha(int alpha);
    virtual void SetPosition(int x, int y, int w, int h);
    virtual void SetBorderStyle(int flag);
    virtual int GetBorderStyle(void);
    virtual int GetBackgroundStyle(void);
    virtual int GetBackgroundIdx(void);
    virtual GUI_Surface *GetBackground(void);
    virtual void SetBackgroundStyle(int flag);
    virtual void SetBackground(GUI_Surface * image);
    virtual void SetFocusedBackground(GUI_Surface * image);
    virtual void SetPressedBackground(GUI_Surface * image);
    virtual void SetDisabledBackground(GUI_Surface * image);
    virtual void SetColor(SDL_Color c);
    virtual SDL_Color GetColor(void);
    virtual void SetFocusedColor(SDL_Color c);
    virtual SDL_Color GetFocusedColor(void);
    virtual void SetDisabledColor(SDL_Color c);
    virtual SDL_Color GetDisabledColor(void);
    virtual void SetPressedColor(SDL_Color c);
    virtual SDL_Color GetPressedColor(void);
    virtual void SetBorderColor(SDL_Color c);
    virtual SDL_Color GetBorderColor(void);
    virtual void SetABorderColor(SDL_Color c);
    virtual SDL_Color GetABorderColor(void);
    virtual void SetOBorderColor(SDL_Color c);
    virtual SDL_Color GetOBorderColor(void);
    virtual int GetAlign(void);
    virtual void SetAlign(int flag);
    virtual void SetTextAlign(const char *textalign);
    virtual const char *GetTextAlign(void);
    virtual int OnGotFocus(void);
    virtual int OnLostFocus(void);
    virtual int OnChangeFocus(void);
    virtual int OnMouseOver(void);
    virtual int OnMouseOut(void);
    virtual void SetParent(Object * aparent);
    virtual void ClearParent(void);
    virtual GUI_Widget *GetParent(void);
    virtual GUI_Page *GetPage(void);
    virtual GUI_Surface *GetSurface(SDL_Rect *dr);
    virtual int Keep(Object ** target, Object * source);
    virtual SDL_Rect Adjust(const SDL_Rect * area);
    virtual int ClipRect(SDL_Rect * sr, SDL_Rect * dr, SDL_Rect * clip);
    virtual void TileImage(GUI_Surface * surface, const SDL_Rect * area, int x_offset, int y_offset);
    virtual void DrawClipped(GUI_Surface * surface, GUI_Widget * dest, SDL_Rect destarea, int destx, int desty);
    virtual void DrawPixel(SDL_Surface * surface, int x, int y, SDL_Color c);
    virtual void DrawRect(SDL_Surface * surface, int x1, int y1, int x2, int y2, SDL_Color c);
    virtual void DrawLine(SDL_Surface * surface, int x1, int y1, int x2, int y2, SDL_Color c);
    virtual void DrawVGradient(SDL_Surface * surface, SDL_Color c1, SDL_Color c2, Uint8 alpha);
    virtual void DrawVGradient2(SDL_Surface * surface, SDL_Rect dest, SDL_Color c1, SDL_Color c2, Uint8 alpha);
    virtual void DrawHGradient2(SDL_Surface * surface, SDL_Rect dest, SDL_Color c1, SDL_Color c2, Uint8 alpha);
    virtual void FloodFill(SDL_Surface * screen, int x, int y, SDL_Color c);
    virtual void FillTriangle(SDL_Surface * s, int x[3], int y[3], SDL_Color color);
    virtual void DrawTriangle(SDL_Surface * s, int x[3], int y[3], SDL_Color c);
    virtual void DrawArrow(SDL_Surface * s, int type, int x, int y, int a, SDL_Color color, bool fill, SDL_Color fillcolor);
    virtual void DrawRoundBorder(SDL_Surface * s, SDL_Rect r, SDL_Color c, bool dbl);
    virtual Uint32 GetPixel(SDL_Surface * surface, int x, int y);
    virtual void SetOnGotFocusCb(const char *val);
    virtual void SetOnLostFocusCb(const char *val);

  protected:
    short alpha;
    short align;
    SDL_Rect area;
    SDL_Rect areaorig;
    GUI_Widget * parent;
    SDL_Color color[IDX_SIZE];
    struct {
      short style;
      bool changed[IDX_SIZE];
      GUI_Surface *img[IDX_SIZE];
    } background;
    struct {
      short style;
      short x;
      short y;
      SDL_Color color[3];
    } border;
    Uint32 lastclicktime;
    short contentx;
    short contenty;
    char *ongotfocuscb;
    char *onlostfocuscb;

    Variant pget_AreaOrig(void);
    Variant pget_ParentName(void);
    Variant pget_X(void);
    int pset_X(Variant value);
    Variant pget_Y(void);
    int pset_Y(Variant value);
    Variant pget_ContentX(void);
    int pset_ContentX(Variant value);
    Variant pget_ContentY(void);
    int pset_ContentY(Variant value);
    Variant pget_Width(void);
    int pset_Width(Variant value);
    Variant pget_Height(void);
    int pset_Height(Variant value);
    Variant pget_TextAlign(void);
    int pset_TextAlign(Variant value);
    Variant pget_Alpha(void);
    int pset_Alpha(Variant value);
    Variant pget_BorderStyle(void);
    int pset_BorderStyle(Variant value);
    Variant pget_BorderColor(void);
    int pset_BorderColor(Variant value);
    Variant pget_ABorderColor(void);
    int pset_ABorderColor(Variant value);
    Variant pget_OBorderColor(void);
    int pset_OBorderColor(Variant value);
    Variant pget_Visible(void);
    int pset_Visible(Variant value);
    Variant pget_Selectable(void);
    int pset_Selectable(Variant value);
    Variant pget_Enabled(void);
    int pset_Enabled(Variant value);
    Variant pget_BorderX(void);
    int pset_BorderX(Variant value);
    Variant pget_BorderY(void);
    int pset_BorderY(Variant value);
    Variant pget_Background(void);
    int pset_Background(Variant value);
    Variant pget_FocusedBackground(void);
    int pset_FocusedBackground(Variant value);
    Variant pget_PressedBackground(void);
    int pset_PressedBackground(Variant value);
    Variant pget_DisabledBackground(void);
    int pset_DisabledBackground(Variant value);
    Variant pget_BackgroundStyle(void);
    int pset_BackgroundStyle(Variant value);
    Variant pget_Color(void);
    int pset_Color(Variant value);
    Variant pget_FocusedColor(void);
    int pset_FocusedColor(Variant value);
    Variant pget_PressedColor(void);
    int pset_PressedColor(Variant value);
    Variant pget_DisabledColor(void);
    int pset_DisabledColor(Variant value);
    Variant pget_Area(void);
    int pset_Area(Variant value);
    Variant pget_AreaNoScale(void);
    Variant pget_OnGotFocus(void);
    int pset_OnGotFocus(Variant value);
    Variant pget_OnLostFocus(void);
    int pset_OnLostFocus(Variant value);
    Variant pget_Focus(void);
    int pset_Focus(Variant value);

    Variant m_SetFocus(int numargs, Variant args[]);
    Variant m_Move(int numargs, Variant args[]);
    Variant m_HandleEvent(int numargs, Variant args[]);
    Variant m_TransformScale(int numargs, Variant args[]);
};

class GUI_GFX:public GUI_Widget {
  public:
    GUI_GFX(const char *aname, int x, int y, int w, int h, const char *type);
    virtual ~ GUI_GFX(void);

    virtual void Update(int force);
    virtual void OnResize(void);
    virtual void UpdateBorder(int idx);
    virtual void SetColor(SDL_Color c);
    virtual void SetBorderStyle(int flag);

    void MakeFill(SDL_Color c);
    void MakePoint(int x0, int y0, SDL_Color color);
    void MakeText(int x0, int y0, const char *text, SDL_Color c);
    void MakeLine(int x0, int y0, int x1, int y1, SDL_Color c);
    void MakePolygon(short *x, short *y, int n, SDL_Color c);

    Variant m_Reset(int numargs, Variant args[]);
    Variant m_Fill(int numargs, Variant args[]);
    Variant m_Save(int numargs, Variant args[]);
    Variant m_Text(int numargs, Variant args[]);
    Variant m_Point(int numargs, Variant args[]);
    Variant m_Line(int numargs, Variant args[]);
    Variant m_Polygon(int numargs, Variant args[]);

  protected:
    char *gfx_type;
};

class GUI_Label:public GUI_Widget {
  public:
    GUI_Label(const char *aname, int x, int y, int w, int h, GUI_Font * afont, const char *s);
    virtual ~ GUI_Label(void);
    virtual void FireTimer(void);
    void SetFont(GUI_Font * font);
    void SetFontSize(int size);
    void SetTextColor(SDL_Color c);
    void SetText(const char *s);
    virtual void Update(int force);
    void SetAutoSize(bool autosize);
    void SetWordWrap(bool wordwrap);
    void SetLineOffset(int line);
    void SetLineSpacing(int line);
    const char *GetText(void);
    int GetTextSize(void);
    int GetAutoSize(void);
    int GetWordWrap(void);
    int GetFontSize(void);
    int GetLineOffset(void);
    int GetLineSpacing(void);
    int GetLineCount(void);
    GUI_Font *GetFont(void);
    SDL_Color GetTextColor(void);

  protected:
    GUI_Surface * text;
    GUI_Font *font;
    short fontsize;
    short lineoffset;
    short linespacing;
    SDL_Color textcolor;
    char *textstring;
    char *textbuffer;
    bool autosize;
    bool wordwrap;

    Variant pget_TextSize(void);
    Variant pget_Caption(void);
    int pset_Caption(Variant value);
    Variant pget_Font(void);
    int pset_Font(Variant value);
    Variant pget_FontSize(void);
    int pset_FontSize(Variant value);
    Variant pget_FontColor(void);
    int pset_FontColor(Variant value);
    Variant pget_AutoSize(void);
    int pset_AutoSize(Variant value);
    Variant pget_WordWrap(void);
    int pset_WordWrap(Variant value);
    Variant pget_LineOffset(void);
    int pset_LineOffset(Variant value);
    Variant pget_LineSpacing(void);
    int pset_LineSpacing(Variant value);
    Variant pget_LineCount(void);
};

class GUI_ClockLabel:public GUI_Label {
  public:
    GUI_ClockLabel(const char *aname, int x, int y, int w, int h, GUI_Font * afont, const char *s);
    virtual ~ GUI_ClockLabel(void);
    void SetClockFormat(const char *format);
    virtual void FireTimer(void);
    virtual void OnStart(void);

  protected:
    char *clockformat;

    Variant pget_ClockFormat(void);
    int pset_ClockFormat(Variant value);
};

class GUI_Picture:public GUI_Widget {
  public:
    GUI_Picture(const char *aname, int x, int y, int w, int h, GUI_Surface * an_image);
    virtual ~ GUI_Picture(void);
    void SetImage(GUI_Surface * an_image);
    void SetCaption(GUI_Widget * a_caption);
    void ZoomImage(GUI_Surface * an_image, int w, int h);
    void FadeImage(GUI_Surface * an_image);
    void Rotate(double angle, double zoom);
    void Zoom(int w, int h);
    void Reload(void);
    virtual void Update(int force);
    virtual void Erase(const SDL_Rect * area);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual void OnStart(void);
    virtual void OnTransform(void);

  protected:
    GUI_Label *caption;
    GUI_Surface * image;

    Variant pget_Image(void);
    int pset_Image(Variant value);
    Variant pget_Caption(void);
    int pset_Caption(Variant value);
    Variant pget_Font(void);
    int pset_Font(Variant value);
    Variant pget_FontSize(void);
    int pset_FontSize(Variant value);
    Variant pget_FontColor(void);
    int pset_FontColor(Variant value);
    Variant pget_CaptionX(void);
    int pset_CaptionX(Variant value);
    Variant pget_CaptionY(void);
    int pset_CaptionY(Variant value);

    Variant m_ReloadImage(int numargs, Variant args[]);
    Variant m_FadeImage(int numargs, Variant args[]);
    Variant m_ZoomImage(int numargs, Variant args[]);
    Variant m_RotateImage(int numargs, Variant args[]);
};

class GUI_AbstractButton:public GUI_Widget {
  public:
    GUI_AbstractButton(const char *aname, int x, int y, int w, int h);
    virtual ~ GUI_AbstractButton(void);
    virtual int Initialize(void);
    virtual void RemoveWidget(GUI_Widget * widget);
    void SetCaption(GUI_Label * widget);
    void SetCaptionX(int x);
    void SetCaptionY(int x);
    virtual void Update(int force);
    virtual void OnClick(void);
    virtual void OnResize(void);
    virtual int OnChangeFocus(void);
    virtual void Clicked(int x, int y);
    virtual void SetAction(const char *action);
    virtual const char *GetAction(void);
    virtual void UpdateLabel(void);

  protected:
    char *action;
    bool autosize;
    GUI_Label * caption;

    Variant pget_Action(void);
    int pset_Action(Variant value);
    Variant pget_AutoSize(void);
    int pset_AutoSize(Variant value);
};

class GUI_Button:public GUI_AbstractButton {
  public:
    GUI_Button(const char *aname, int x, int y, int w, int h);
    virtual ~ GUI_Button(void);
    void SetCaptionDropX(int amount);
    void SetCaptionDropY(int amount);
    virtual void SetBackground(GUI_Surface * surface);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual void OnUpDown(void);
    virtual int GetAlign(void);
    virtual void SetAlign(int flag);
    bool IsDown(void);
    void SimulateClick(void);
    void SetText(const char *s);
    void SetFont(GUI_Font *font);
    void SetFontSize(int size);
    void SetFontNormalColor(SDL_Color c);
    void SetFontHighlightColor(SDL_Color c);
    void SetFontDisabledColor(SDL_Color c);
    void SetFontPressedColor(SDL_Color c);
    virtual bool HandleEvent(const char *action);
    virtual void UpdateLabel(void);

  protected:
    SDL_Color fontnormalcolor;
    SDL_Color fonthighlightcolor;
    SDL_Color fontpressedcolor;
    SDL_Color fontdisabledcolor;
    int captiondropx;
    int captiondropy;
    bool down;

    Variant pget_Caption(void);
    int pset_Caption(Variant value);
    Variant pget_Font(void);
    int pset_Font(Variant value);
    Variant pget_FontSize(void);
    int pset_FontSize(Variant value);
    Variant pget_FontColor(void);
    int pset_FontColor(Variant value);
    Variant pget_FontFocusColor(void);
    int pset_FontFocusColor(Variant value);
    Variant pget_FontPressedColor(void);
    int pset_FontPressedColor(Variant value);
    Variant pget_FontDisabledColor(void);
    int pset_FontDisabledColor(Variant value);
    Variant pget_CaptionX(void);
    int pset_CaptionX(Variant value);
    Variant pget_CaptionY(void);
    int pset_CaptionY(Variant value);
    Variant pget_CaptionDropX(void);
    int pset_CaptionDropX(Variant value);
    Variant pget_CaptionDropY(void);
    int pset_CaptionDropY(Variant value);
    Variant m_Click(int numargs, Variant args[]);
    Variant pget_TextAlign(void);
    int pset_TextAlign(Variant value);
    Variant pget_WordWrap(void);
    int pset_WordWrap(Variant value);
};

class GUI_ToggleButton:public GUI_AbstractButton {
  public:
    GUI_ToggleButton(const char *aname, int x, int y, int w, int h);
    virtual ~ GUI_ToggleButton(void);
    virtual void Clicked(int x, int y);
    virtual void SetBackground(GUI_Surface * surface);
    virtual bool HandleEvent(const char *action);
    virtual int GetBackgroundIdx(void);

  protected:
    int checked;

    Variant pget_NormalImage(void);
    int pset_NormalImage(Variant value);
    Variant pget_NormalFocusImage(void);
    int pset_NormalFocusImage(Variant value);
    Variant pget_CheckedImage(void);
    int pset_CheckedImage(Variant value);
    Variant pget_CheckedFocusImage(void);
    int pset_CheckedFocusImage(Variant value);
    Variant pget_Checked(void);
    int pset_Checked(Variant value);
    Variant pget_Action(void);
    int pset_Action(Variant value);
};

class GUI_ScrollBar:public GUI_Widget {
  public:
    GUI_ScrollBar(const char *aname, int x, int y, int w, int h);
    virtual ~ GUI_ScrollBar(void);
    void SetKnobImage(GUI_Surface * image);
    void SetKnobFocusImage(GUI_Surface * image);
    GUI_Surface *GetKnobImage(void);
    GUI_Surface *GetKnobFocusImage(void);
    void SetPageStep(int value);
    void SetMovedCallback(GUI_Callback * callback);
    virtual void Update(int force);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    void SetValue(int value);
    int GetValue(void);
    void SetReversed(bool reversed);
    int GetMaximum(void);
    void SetMaximum(int maximum);
    int GetMinimum(void);
    void SetMinimum(int minimum);
    virtual void Increment(void);
    virtual void Decrement(void);
    virtual void IncrementPage(void);
    virtual void DecrementPage(void);
    virtual bool HandleEvent(const char *action);

  protected:
    GUI_Surface *knob;
    GUI_Surface *knob_focus;
    GUI_Callback *moved_callback;
    int position;
    int tracking_on;
    int tracking_pos;
    int tracking_start;
    bool tracking_update;
    int page_step;
    int maximum;
    int minimum;
    int value;
    int oldvalue;
    bool reversed;

    void RecalcPosition(void);
    void RecalcValue(void);
    virtual void OnChange(Object * sender);

    Variant pget_Knob(void);
    int pset_Knob(Variant value);
    Variant pget_KnobFocus(void);
    int pset_KnobFocus(Variant value);
    Variant pget_Value(void);
    int pset_Value(Variant value);
    Variant pget_Maximum(void);
    int pset_Maximum(Variant value);
    Variant pget_Minimum(void);
    int pset_Minimum(Variant value);
    Variant pget_PageStep(void);
    int pset_PageStep(Variant value);
    Variant pget_Reversed(void);
    int pset_Reversed(Variant value);
    Variant pget_TrackingUpdate(void);
    int pset_TrackingUpdate(Variant value);
};

class GUI_ListBox:public GUI_Widget {
  public:
    GUI_ListBox(const char *aname, int x, int y, int w, int h, GUI_Font * afont);
    virtual ~ GUI_ListBox(void);
    void SetFont(GUI_Font * font);
    void SetTextColor(SDL_Color c);
    void SetSelectedItemTextColor(SDL_Color c);
    void SetSelectedItemTextColorFocus(SDL_Color c);
    void SetSelectedItemImage(GUI_Surface * surface);
    void SetSelectedItemFocusImage(GUI_Surface * surface);
    void SetSelectedItemBackgroundColor(SDL_Color c);
    void SetSelectedItemBackgroundFocusColor(SDL_Color c);
    void SetSelectedItemStyle(int style);
    void SetScrollBarMode(int scrollbarmode);
    void SetScrollBarKnobImage(GUI_Surface * surface);
    void SetScrollBarBackgroundImage(GUI_Surface * surface);
    void SetScrollBarBackgroundStyle(int style);
    void SetScrollBarWidth(int width);
    void SetItemBorderX(int pixels);
    void SetItemBorderY(int pixels);
    void SetItemHeight(int pixels);
    void SetSelItemBorderX(int pixels);
    void SetSelItemBorderY(int pixels);
    void AddItemData(const char *text, const char *data);
    char *MakeItemData(const char *text, const char *data);
    char *GetItemData(const char *item);
    void RemoveItem(int index);
    void SetSearchText(const char *data);
    void SetItemText(int index, const char *text);
    void SetItemData(int index, const char *data);
    void SetSelectedIndex(int index);
    void InternalSetSelectedIndex(int index, bool forceevent);
    int GetSelectedIndex(void);
    int GetVisibleItems(void);
    void SetTopIndex(int index);
    void SelectNext(int count);
    void SelectPrevious(int count);
    void SelectFirst(void);
    void SelectLast(void);
    void Sort(void);
    void Clear(void);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    int ItemAtPoint(int x, int y);
    virtual void SetBorderX(int pixels);
    virtual void SetBorderY(int pixels);
    virtual void Update(int force);
    virtual bool HandleEvent(const char *action);
    virtual void OnStart(void);
    virtual int OnGotFocus(void);
    virtual int OnLostFocus(void);
    virtual void OnSelectionChange(void);
    virtual void DoubleClicked(int x, int y);
    virtual void OnChoose(void);
    virtual void OnScrollBarMove(Object * sender);
    virtual void ChooseSelection(void);

  protected:
    int fontsize;
    GUI_Font * font;
    SDL_Color textcolor;
    SDL_Color selitemtextcolor;
    SDL_Color selitemtextcolorfocus;
    vector < const char * > listitems;
    bool wrap;
    bool oneclick;
    bool scrolling;
    int itemheight;
    int useritemheight;
    int topindex;
    int wheelpagestep;
    int visibleitems;
    int selectedindex;
    int itemborderx;
    int itembordery;
    int selitemborderx;
    int selitembordery;
    int scrollbarmode;
    bool scrollbarvisible;
    GUI_Surface *selected_item_image;
    GUI_Surface *selected_item_focus_image;
    int selected_item_style;
    GUI_ScrollBar *scrollbar;
    char searchtext[32];

    void SetupScrollBar(void);
    void RecalcItemHeight(void);
    void CheckScrollBarVisible(void);

    Variant pget_Wrap(void);
    int pset_Wrap(Variant value);
    Variant pget_OneClick(void);
    int pset_OneClick(Variant value);
    Variant pget_Font(void);
    int pset_Font(Variant value);
    Variant pget_FontSize(void);
    int pset_FontSize(Variant value);
    Variant pget_FontColor(void);
    int pset_FontColor(Variant value);
    Variant pget_SelectedIndex(void);
    int pset_SelectedIndex(Variant value);
    Variant pget_SelectedText(void);
    Variant pget_SelectedData(void);
    Variant pget_SearchText(void);
    Variant pget_Count(void);
    Variant pget_SelItemBackgroundColor(void);
    int pset_SelItemBackgroundColor(Variant value);
    Variant pget_SelItemBackgroundFocusColor(void);
    int pset_SelItemBackgroundFocusColor(Variant value);
    Variant pget_SelItemBackground(void);
    int pset_SelItemBackground(Variant value);
    Variant pget_SelItemBackgroundFocus(void);
    int pset_SelItemBackgroundFocus(Variant value);
    Variant pget_SelItemBackgroundStyle(void);
    int pset_SelItemBackgroundStyle(Variant value);
    Variant pget_SelItemFontColor(void);
    int pset_SelItemFontColor(Variant value);
    Variant pget_SelItemFontColorFocus(void);
    int pset_SelItemFontColorFocus(Variant value);
    Variant pget_ScrollBarBackground(void);
    int pset_ScrollBarBackground(Variant value);
    Variant pget_ScrollBarBackgroundStyle(void);
    int pset_ScrollBarBackgroundStyle(Variant value);
    Variant pget_ScrollBarKnob(void);
    int pset_ScrollBarKnob(Variant value);
    Variant pget_ScrollBarWidth(void);
    int pset_ScrollBarWidth(Variant value);
    Variant pget_ScrollBar(void);
    int pset_ScrollBar(Variant value);
    Variant pget_ItemBorderX(void);
    int pset_ItemBorderX(Variant value);
    Variant pget_ItemBorderY(void);
    int pset_ItemBorderY(Variant value);
    Variant pget_ItemHeight(void);
    int pset_ItemHeight(Variant value);
    Variant pget_ItemList(void);
    Variant pget_DataList(void);
    Variant pget_SelItemBorderX(void);
    int pset_SelItemBorderX(Variant value);
    Variant pget_SelItemBorderY(void);
    int pset_SelItemBorderY(Variant value);
    Variant pget_WheelPageStep(void);
    int pset_WheelPageStep(Variant value);

    Variant m_AddItem(int numargs, Variant args[]);
    Variant m_AddItemData(int numargs, Variant args[]);
    Variant m_RemoveItem(int numargs, Variant args[]);
    Variant m_RemoveLastItem(int numargs, Variant args[]);
    Variant m_Sort(int numargs, Variant args[]);
    Variant m_Clear(int numargs, Variant args[]);
    Variant m_SetItemText(int numargs, Variant args[]);
    Variant m_SetItemData(int numargs, Variant args[]);
    Variant m_ItemText(int numargs, Variant args[]);
    Variant m_ItemData(int numargs, Variant args[]);
    Variant m_MatchItem(int numargs, Variant args[]);
    Variant m_SearchItem(int numargs, Variant args[]);
    Variant m_MatchItemData(int numargs, Variant args[]);
    Variant m_SearchItemData(int numargs, Variant args[]);
};

class GUI_FileBrowser:public GUI_ListBox {
  public:
    GUI_FileBrowser(const char *aname, int x, int y, int w, int h, GUI_Font * afont);
    virtual ~ GUI_FileBrowser(void);
    virtual void ChooseSelection(void);

  private:
    void DoRefresh(void);

  protected:
    string rootpath;
    string curpath;
    string lastpath;

    Variant pget_CurPath(void);
    Variant pget_RootPath(void);
    int pset_RootPath(Variant value);

    Variant pget_FileName(void);
    Variant m_Refresh(int numargs, Variant args[]);
    Variant m_Choose(int numargs, Variant args[]);
};

class GUI_TextField:public GUI_Widget {
  public:
    GUI_TextField(const char *name, int x, int y, int w, int h, GUI_Font * font, int maxlength);
    virtual ~GUI_TextField(void);
    void SetFont(GUI_Font * afont);
    void SetFontNormalColor(SDL_Color c);
    void SetFontFocusedColor(SDL_Color c);
    void SetText(const char *text);
    const char *GetText(void);
    int GetCursorPos(void);
    int GetTextLength(void);
    void SetCursorPos(int pos);
    const char *GetValidChars(void);
    void SetValidChars(const char *validchars);
    virtual void Update(int force);
    virtual bool HandleKeyEvent(const SDL_Event * event);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    bool SendChar(char c, bool insert);
    void Backspace(void);
    void DeleteCurrChar(void);
    virtual bool HandleEvent(const char *action);
    virtual int OnGotFocus(void);
    virtual void OnChange(void);

  protected:
    int fontsize;
    GUI_Font * font;
    SDL_Color fontnormalcolor;
    SDL_Color fontfocusedcolor;
    int buffer_size;
    int buffer_index;
    char *buffer;
    char *cbuffer;
    int cursorpos;
    int startoffset;
    char *validchars;
    int inpdigit;
    const char *inpdigitptr;
    timeval input_time;

    void InputDigit(int chr);

    Variant pget_Text(void);
    int pset_Text(Variant value);
    Variant pget_Font(void);
    int pset_Font(Variant value);
    Variant pget_FontSize(void);
    int pset_FontSize(Variant value);
    Variant pget_FontColor(void);
    int pset_FontColor(Variant value);
    Variant pget_FontFocusedColor(void);
    int pset_FontFocusedColor(Variant value);
    Variant pget_MaxLength(void);
    int pset_MaxLength(Variant value);
    Variant pget_CursorPos(void);
    int pset_CursorPos(Variant value);
    Variant pget_ValidChars(void);
    int pset_ValidChars(Variant value);
};

class GUI_Layout:public Object {
  public:
    GUI_Layout(const char *aname);
    virtual ~ GUI_Layout(void);
    virtual void Layout(GUI_Container * container);
};

class GUI_VBoxLayout:public GUI_Layout {
  public:
    GUI_VBoxLayout(const char *aname);
    virtual ~ GUI_VBoxLayout(void);
    virtual void Layout(GUI_Container * container);
};

class GUI_HBoxLayout:public GUI_Layout {
  public:
    GUI_HBoxLayout(const char *aname);
    virtual ~ GUI_HBoxLayout(void);
    virtual void Layout(GUI_Container * container);
};

class GUI_Container:public GUI_Widget {
  public:
    GUI_Container(const char *name, int x, int y, int w, int h);
    virtual ~ GUI_Container(void);
    virtual void Update(int force);
    virtual void UpdateLayout(void);
    virtual void SetLayout(GUI_Layout * a_layout);
    virtual bool HandleKeyEvent(const SDL_Event * event);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual bool HandleEvent(const char *action);
    virtual int GetWidgetCount(void);
    virtual int ContainsWidget(GUI_Widget * widget);
    virtual void AddWidget(GUI_Widget * widget);
    virtual void RemoveWidget(GUI_Widget * widget);
    virtual GUI_Widget *GetWidget(int index);
    virtual GUI_Widget *FindWidget(const char *name);
    virtual int UpdateFocusWidget(int delta);
    virtual int SetFocusWidget(GUI_Widget *widget);
    virtual int SetFocusWidget(int index);
    virtual GUI_Widget *GetFocusWidget(void);
    virtual int GetFocusIndex(void);
    virtual void Draw(GUI_Surface * image, const SDL_Rect * sr, const SDL_Rect * dr);
    virtual DynamicObject *FindObject(const char *name);
    virtual void Erase(const SDL_Rect * rp);
    virtual void StartObjects(void);
    virtual void StopObjects(void);
    virtual int OnGotFocus(void);
    virtual int OnLostFocus(void);
    string GetWidgetList(void);
    void SetHelp(const char *text);
    void SetPrevPage(const char *text);
    const char *GetPrevPage(void);

  protected:
    int focus_index;
    GUI_Layout * layout;
    vector < GUI_Widget * > widgets;
    char *help;
    char *prevpage;

    Variant pget_WidgetList(void);
    Variant pget_WidgetCount(void);
    Variant pget_FocusIndex(void);
    int pset_FocusIndex(Variant value);
    Variant pget_FocusWidget(void);
    int pset_FocusWidget(Variant value);
    Variant pget_Help(void);
    int pset_Help(Variant value);
    Variant pget_PrevPage(void);
    int pset_PrevPage(Variant value);

    Variant m_UpdateFocus(int numargs, Variant args[]);
};

class GUI_Page:public GUI_Container {
  public:
    GUI_Page(const char *name, int x, int y, int w, int h);
    virtual ~ GUI_Page(void);
    virtual void FlushUpdates(int force);
    virtual int OnGotFocus(void);
    virtual int OnLostFocus(void);

  protected:
    GUI_Surface * surface;
    Display *display;
    Visual *visual;
    XImage *ximage;
    Window window;
    int depth;
    GC gc;
};

class GUI_Screen:public GUI_Container {
  public:
    GUI_Screen(const char *aname, SDL_Surface * surface);
    virtual ~ GUI_Screen(void);
    virtual int ShowOverlay(GUI_Page *page);
    virtual void Draw(GUI_Surface * image, const SDL_Rect * sr, const SDL_Rect * dr);
    virtual void Erase(const SDL_Rect * dr);
    virtual void Fill(const SDL_Rect * dr, SDL_Color c);
    virtual bool HandleEvent(const char *action);
    virtual bool HandleKeyEvent(const SDL_Event * event);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual void OnExpose(void);
    virtual GUI_Surface *GetSurface(SDL_Rect *dr);
    virtual void FlushUpdates(int force);
    virtual void UpdateRect(const SDL_Rect * r);
    virtual void Update(int force);
    virtual void Flip(void);

  protected:
    GUI_Page *overlay;
    GUI_Surface * screen;
    SDL_Rect *updates;
    int n_updates;
};

class TimerObject:public DynamicObject {
  public:
    TimerObject(const char *name, int interval, bool autostart);
    virtual ~ TimerObject(void);
    void SetInterval(int interval);
    virtual void SetEnabled(bool enable);
    void SetAutoStart(bool autostart);
    virtual void Fire(void);
    virtual void OnStart(void);
    virtual void OnStop(void);

  protected:
    int interval;
    bool enabled;
    bool autostart;
    SDL_TimerID timer_id;

    Variant pget_Interval(void);
    int pset_Interval(Variant value);
    Variant pget_Enabled(void);
    int pset_Enabled(Variant value);

    Variant m_Start(int numargs, Variant args[]);
    Variant m_Stop(int numargs, Variant args[]);
};

class PlayerObject:public GUI_Widget {
  public:
    PlayerObject(const char *name);
    virtual ~ PlayerObject(void);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual void SetPosition(int x, int y, int w, int h);
    virtual SDL_Rect GetAreaRaw(void);
    virtual void OnExpose(void);
    virtual void OnResize(void);
    virtual Window GetWindow(void);
    virtual Display *GetDisplay(void);
    virtual int WindowInit(void);
    virtual int WindowShow(void);
    virtual int WindowDestroy(void);
    virtual int WindowHide(void);
    virtual int WindowSync(void);
    virtual int InfoInit(void);
    virtual void InfoDestroy(void);
    virtual int IsPlaying(void);
    virtual int PlayerInit(void);
    virtual void PlayerExpose(void);
    virtual int Play(void);
    virtual void Pause(void);
    virtual void Stop(void);
    virtual void Destroy(void);
    virtual void PlaylistIndex(int step);
    virtual void PlaylistClear(void);
    virtual int PlaylistNext(void);
    virtual int PlaylistPrev(void);
    virtual void OnEvent(void);
    virtual void FileAdd(const char *files, const char *pattern, time_t start, time_t duration, const char *skip);
    virtual int FileFind(const char *file);
    virtual char *FileInfo(const char *mrl);
    virtual int Seek(int offset, bool absolute);
    virtual int GetTimePosition(int *len);
    virtual void SetStartPosition(int val);
    virtual int Snapshot(const char *filename);
    virtual void OsdInit(void);
    virtual void OsdFree(void);
    virtual void OsdInfo(const char *text);
    virtual void OsdText(int x, int y, const char *text);
    virtual void OsdSlider(int val, int max, const char *str);
    virtual int OsdSet(const char *cmd);
    virtual char *PlayerEvent(const char *action);
    virtual void DisplayLock(void);
    virtual void DisplayUnlock(void);
    virtual void SetVideoDriver(const char *dname);
    virtual void SetAudioDriver(const char *dname);
    virtual void SetWindowBorder(int border);
    virtual void GetAutoPlay(const char *mrl);
    virtual char *GetPlaylist(void);
    virtual char *GetPlaylistFile(void);
    virtual void SetPlaylistFile(const char *value);
    virtual char *GetPlaylistTitle(void);
    virtual void *SetPlaylistTitle(const char *value);
    virtual char *GetPlaylistAlbum(void);
    virtual void *SetPlaylistAlbum(const char *value);
    virtual char *GetPlaylistArtist(void);
    virtual void *SetPlaylistArtist(const char *value);
    virtual char *GetPlaylistGenre(void);
    virtual void *SetPlaylistGenre(const char *value);
    virtual char *GetPlaylistComment(void);
    virtual void *SetPlaylistComment(const char *value);
    virtual int GetPlaylistSize(void);
    virtual int GetPlaylistIndex(void);
    virtual void SetPlaylistIndex(int idx);
    virtual int GetPlaylistShuffle(void);
    virtual void SetPlaylistShuffle(int value);
    virtual int GetPlaylistRepeat(void);
    virtual void SetPlaylistRepeat(int val);
    virtual char *GetVideoDriver(void);
    virtual char *GetAudioDriver(void);
    virtual int GetAVOffset(void);
    virtual void SetAVOffset(int val);
    virtual int GetSPUOffset(void);
    virtual void SetSPUOffset(int val);
    virtual int GetVolume(void);
    virtual void SetVolume(int val);
    virtual int GetMute(void);
    virtual void SetMute(int val);
    virtual int GetAmpMute(void);
    virtual void SetAmpMute(int val);
    virtual int GetAmpLevel(void);
    virtual void SetAmpLevel(int val);
    virtual int GetHue(void);
    virtual void SetHue(int val);
    virtual int GetSaturation(void);
    virtual void SetSaturation(int val);
    virtual int GetBrightness(void);
    virtual void SetBrightness(int val);
    virtual int GetContrast(void);
    virtual void SetContrast(int val);
    virtual int GetAudioChannel(void);
    virtual void SetAudioChannel(int val);
    virtual int GetSPUChannel(void);
    virtual void SetSPUChannel(int val);
    virtual int GetSpeed(void);
    virtual void SetSpeed(char *value);
    virtual int GetLength(void);
    virtual char *GetDeinterlaceFilters(void);
    virtual char *GetVideoFilter(void);
    virtual void SetVideoFilter(const char *value);
    virtual char *GetAudioFilter(void);
    virtual void SetAudioFilter(const char *value);
    virtual int GetBroadcastPort(void);
    virtual void SetBroadcastPort(int value);
    virtual int GetVideoBorder(void);
    virtual void SetVideoBorder(int pixels);
    virtual double GetAspect(void);
    virtual void SetAspect(double value);
    virtual int HasVideo(void);
    virtual int HasAudio(void);
    virtual char *GetVisualizationFilters(void);
    virtual const char *GetStatus(void);
    virtual int GetVideoWidth(void);
    virtual int GetVideoHeight(void);

    int osd_flag;
    bool has_video;
    bool has_audio;
    char *video_driver;
    char *audio_driver;
    vector < FileEntry *>playlist_files;
    char *playlist_title;
    char *playlist_artist;
    char *playlist_genre;
    char *playlist_album;
    char *playlist_comment;
    int playlist_index;
    int playlist_repeat;
    int playlist_shuffle;
    double aspect;
    int audio_channel;
    int spu_channel;
    int vo_width;
    int vo_height;
    int vo_border;
    int volume;
    int amp_level;
    time_t seek_time;
    time_t pos_time;
    time_t end_time;
    int skip_flag;
    Window window;
    Display *display;
    int visual_type;
    int broadcast_port;

    Variant pget_Length(void);
    Variant pget_Status(void);
    Variant pget_Info(void);
    Variant pget_PlayList(void);
    Variant pget_Title(void);
    Variant pget_Album(void);
    Variant pget_Artist(void);
    Variant pget_Genre(void);
    Variant pget_Comment(void);
    Variant pget_File(void);
    int pset_File(Variant value);
    Variant pget_PlayListSize(void);
    Variant pget_PlayListIdx(void);
    int pset_PlayListIdx(Variant value);
    Variant pget_VideoDriver(void);
    int pset_VideoDriver(Variant value);
    Variant pget_AudioDriver(void);
    int pset_AudioDriver(Variant value);
    Variant pget_Repeat(void);
    int pset_Repeat(Variant value);
    Variant pget_Shuffle(void);
    int pset_Shuffle(Variant value);
    Variant pget_Hue(void);
    int pset_Hue(Variant value);
    Variant pget_Saturation(void);
    int pset_Saturation(Variant value);
    Variant pget_Contrast(void);
    int pset_Contrast(Variant value);
    Variant pget_Brightness(void);
    int pset_Brightness(Variant value);
    Variant pget_AmpMute(void);
    int pset_AmpMute(Variant value);
    Variant pget_Mute(void);
    int pset_Mute(Variant value);
    Variant pget_Volume(void);
    int pset_Volume(Variant value);
    Variant pget_AmpLevel(void);
    int pset_AmpLevel(Variant value);
    Variant pget_AudioChannel(void);
    int pset_AudioChannel(Variant value);
    Variant pget_SPU_Channel(void);
    int pset_SPU_Channel(Variant value);
    Variant pget_Aspect(void);
    int pset_Aspect(Variant value);
    Variant pget_TimePosition(void);
    int pset_TimePosition(Variant value);
    Variant pget_AV_Offset(void);
    int pset_AV_Offset(Variant value);
    Variant pget_SPU_Offset(void);
    int pset_SPU_Offset(Variant value);
    Variant pget_Speed(void);
    int pset_Speed(Variant value);
    int pset_WindowBorder(Variant value);
    Variant pget_VideoFilter(void);
    int pset_VideoFilter(Variant value);
    Variant pget_AudioFilter(void);
    int pset_AudioFilter(Variant value);
    Variant pget_VideoBorder(void);
    int pset_VideoBorder(Variant value);
    Variant pget_BroadcastPort(void);
    int pset_BroadcastPort(Variant value);
    Variant pget_DeinterlaceFilters(void);
    Variant pget_VisualizationFilters(void);
    Variant pget_HasVideo(void);
    Variant pget_HasAudio(void);
    Variant pget_VideoWidth(void);
    Variant pget_VideoHeight(void);

    Variant m_Play(int numargs, Variant args[]);
    Variant m_Stop(int numargs, Variant args[]);
    Variant m_Hide(int numargs, Variant args[]);
    Variant m_Show(int numargs, Variant args[]);
    Variant m_Pause(int numargs, Variant args[]);
    Variant m_Next(int numargs, Variant args[]);
    Variant m_Prev(int numargs, Variant args[]);
    Variant m_Clear(int numargs, Variant args[]);
    Variant m_Add(int numargs, Variant args[]);
    Variant m_AddFiles(int numargs, Variant args[]);
    Variant m_AddSkip(int numargs, Variant args[]);
    Variant m_Sync(int numargs, Variant args[]);
    Variant m_Seek(int numargs, Variant args[]);
    Variant m_Position(int numargs, Variant args[]);
    Variant m_Event(int numargs, Variant args[]);
    Variant m_Osd(int numargs, Variant args[]);
    Variant m_OsdText(int numargs, Variant args[]);
    Variant m_OsdInfo(int numargs, Variant args[]);
    Variant m_OsdSlider(int numargs, Variant args[]);
    Variant m_Snapshot(int numargs, Variant args[]);
    Variant m_AutoPlay(int numargs, Variant args[]);
    Variant m_Info(int numargs, Variant args[]);
    Variant m_Find(int numargs, Variant args[]);
};

#ifdef HAVE_XINE
class XinePlayerObject:public PlayerObject {
  public:
    XinePlayerObject(const char *name);
    virtual ~ XinePlayerObject(void);
    virtual int InfoInit(void);
    virtual void OsdInit(void);
    virtual void OsdFree(void);
    virtual void InfoDestroy(void);
    virtual int PlayerInit(void);
    virtual void PlayerExpose(void);
    virtual int IsPlaying(void);
    virtual int Play(void);
    virtual void Pause(void);
    virtual void Stop(void);
    virtual void Destroy(void);
    virtual int Seek(int offset, bool absolute);
    virtual void OnEvent(void);
    virtual int GetTimePosition(int *len);
    virtual int Snapshot(const char *filename);
    virtual int OsdSet(const char *cmd);
    virtual void OsdInfo(const char *text);
    virtual void OsdText(int x, int y, const char *text);
    virtual void OsdSlider(int val, int max, const char *str);
    virtual char *FileInfo(const char *mrl);
    virtual char *PlayerEvent(const char *action);
    virtual void VideoWireFilter(void);
    virtual void VideoUnwireFilter(void);
    virtual void VideoParseFilter(const char *filters);
    virtual void AudioWireFilter(void);
    virtual void AudioUnwireFilter(void);
    virtual void AudioParseFilter(const char *filter);
    virtual void GetAutoPlay(const char *mrl);
    virtual char *GetStreamInfo(xine_stream_t *stream);
    virtual int GetAVOffset(void);
    virtual void SetAVOffset(int val);
    virtual int GetSPUOffset(void);
    virtual void SetSPUOffset(int val);
    virtual int GetVolume(void);
    virtual void SetVolume(int val);
    virtual int GetMute(void);
    virtual void SetMute(int val);
    virtual int GetAmpMute(void);
    virtual void SetAmpMute(int val);
    virtual int GetAmpLevel(void);
    virtual void SetAmpLevel(int val);
    virtual int GetHue(void);
    virtual void SetHue(int val);
    virtual int GetSaturation(void);
    virtual void SetSaturation(int val);
    virtual int GetBrightness(void);
    virtual void SetBrightness(int val);
    virtual int GetContrast(void);
    virtual void SetContrast(int val);
    virtual int GetAudioChannel(void);
    virtual void SetAudioChannel(int val);
    virtual int GetSPUChannel(void);
    virtual void SetSPUChannel(int val);
    virtual int GetSpeed(void);
    virtual void SetSpeed(char *value);
    virtual int GetLength(void);
    virtual char *GetDeinterlaceFilters(void);
    virtual char *GetVideoFilter(void);
    virtual void SetVideoFilter(const char *value);
    virtual char *GetAudioFilter(void);
    virtual void SetAudioFilter(const char *value);
    virtual void SetBroadcastPort(int value);
    virtual char *GetVisualizationFilters(void);
    virtual const char *GetStatus(void);

    xine_t *xine;
    xine_osd_t *osd;
    xine_stream_t *stream;
    xine_audio_port_t *ao;
    xine_video_port_t *vo;
    xine_post_t *audio_post;
    xine_event_queue_t *events;
    xine_stream_t *stream_info;
    xine_audio_port_t *ao_info;
    xine_video_port_t *vo_info;
    vector < xine_post_t *>video_post;
    vector < char *>video_filter;
    char *audio_filter;
};
#endif

#ifdef HAVE_VLC
class VLCPlayerObject:public PlayerObject {
  public:
    VLCPlayerObject(const char *name);
    virtual ~ VLCPlayerObject(void);
    virtual int PlayerInit(void);
    virtual int PlaylistNext(void);
    virtual int PlaylistPrev(void);
    virtual int Play(void);
    virtual void Pause(void);
    virtual void Stop(void);
    virtual int Seek(int offset, bool absolute);
    virtual int GetTimePosition(int *len);
    virtual int GetVolume(void);
    virtual void SetVolume(int val);
    virtual void SetMute(int val);
    virtual int GetLength(void);
    virtual const char *GetStatus(void);
    virtual int GetPlaylistIndex(void);

    int vlc;
};

#endif

class TVObject:public virtual DynamicObject {
  public:
    TVObject(const char *name);
    virtual ~ TVObject(void);
    int TVOpen(void);
    int TVClose(void);
    int TVGetMute(void);
    int TVSetMute(int flag);
    int TVSetChannel(void);
    const char *TVGetChannel(void);
    const char *TVGetNorm(void);
    int TVInput(void);
    int TVGetPicture(int what);
    int TVSetPicture(int what, int value);
    long TVGetMixer(char *name);
    int TVSetMixer(char *name, int value);

protected:
    int fd;
    SDL_Rect area;
    int version;
    int norm;
    int depth;
    int input;
    int audio;
    int muted;
    int channel;
    int chanlist;
    string device;

    // V4L version 1
    struct video_audio vaudio;
    struct video_channel vchan;
    struct video_tuner vtuner;
    struct video_capability vcaps;
    struct video_picture vpic;
    // V4L version 2
    struct v4l2_frequency v2freq;
    struct v4l2_audio v2audio;
    struct v4l2_tuner v2tuner;
    struct v4l2_control v2ctrl;
    struct v4l2_queryctrl v2qctrl;
    struct v4l2_standard v2std;
    struct v4l2_input v2input;
    struct v4l2_capability v2caps;
    struct v4l2_format v2fmt;

    Variant pget_Version(void);
    int pset_Version(Variant value);
    Variant pget_Info(void);
    Variant pget_Hue(void);
    int pset_Hue(Variant value);
    Variant pget_Saturation(void);
    int pset_Saturation(Variant value);
    Variant pget_Contrast(void);
    int pset_Contrast(Variant value);
    Variant pget_Brightness(void);
    int pset_Brightness(Variant value);
    Variant pget_Mute(void);
    int pset_Mute(Variant value);
    Variant pget_Volume(void);
    int pset_Volume(Variant value);
    Variant pget_Norm(void);
    int pset_Norm(Variant value);
    Variant pget_Input(void);
    int pset_Input(Variant value);
    Variant pget_Audio(void);
    int pset_Audio(Variant value);
    Variant pget_ChanList(void);
    int pset_ChanList(Variant value);
    Variant pget_Channel(void);
    int pset_Channel(Variant value);
    Variant pget_Device(void);
    int pset_Device(Variant value);
    Variant pget_Depth(void);
    int pset_Depth(Variant value);
    Variant pget_Frequency(void);
    int pset_Frequency(Variant value);
    Variant pget_Status(void);
    Variant pget_CardType(void);
    Variant pget_VideoInputs(void);
    Variant pget_AudioInputs(void);

    Variant m_Open(int numargs, Variant args[]);
    Variant m_Close(int numargs, Variant args[]);
    Variant m_Next(int numargs, Variant args[]);
    Variant m_Prev(int numargs, Variant args[]);
    Variant m_GetChannel(int numargs, Variant args[]);
    Variant m_GetFrequency(int numargs, Variant args[]);
    Variant m_Mixer(int numargs, Variant args[]);
};

#ifdef HAVE_MIXER
class SoundObject:public DynamicObject {
  public:
    SoundObject(const char *name);
    virtual ~ SoundObject(void);

    Variant pget_File(void);
    int pset_Volume(Variant value);
    Variant pget_Volume(void);
    Variant pget_Repeat(void);
    int pset_Repeat(Variant value);

    Variant m_Play(int numargs, Variant args[]);
    Variant m_Stop(int numargs, Variant args[]);
    Variant m_Pause(int numargs, Variant args[]);
    Variant m_Close(int numargs, Variant args[]);

  protected:
    Mix_Music * music;
    string file;
    int repeat;
    int volume;
    int init;
};
#endif

class RadioObject:public DynamicObject {
  public:
    RadioObject(const char *name);
    virtual ~ RadioObject(void);

    Variant pget_Frequency(void);
    int pset_Frequency(Variant value);
    Variant pget_Volume(void);
    int pset_Volume(Variant value);
    Variant pget_Tuner(void);
    int pset_Tuner(Variant value);
    Variant pget_Device(void);
    int pset_Device(Variant value);
    Variant pget_Stereo(void);
    int pset_Stereo(Variant value);
    Variant pget_Status(void);

    Variant m_Play(int numargs, Variant args[]);
    Variant m_Stop(int numargs, Variant args[]);
    Variant m_Close(int numargs, Variant args[]);
    Variant m_Seek(int numargs, Variant args[]);
    Variant m_SetLimits(int numargs, Variant args[]);

  protected:

    int RadioOpen(void);
    void RadioStop(void);
    void RadioClose(void);

    struct video_audio va;
    struct video_tuner vt;

    struct v4l2_audio v2audio;
    struct v4l2_tuner v2tuner;
    struct v4l2_control v2ctrl;
    struct v4l2_frequency v2freq;
    struct v4l2_capability v2caps;
    string device;
    int version;
    int fd;
    int flags;
    int volume;
    int stereo;
    int tuner;
    int status;
    double freq;
    double freq_step;
    double freq_level;
    double freq_min;
    double freq_max;
};

#ifdef HAVE_MOZILLA
class WebBrowserObject:public GUI_Widget {
  public:
    WebBrowserObject(const char *name);
    virtual ~ WebBrowserObject(void);
    virtual void SetPosition(int x, int y, int w, int h);
    virtual void SetVisible(int flag);
    virtual bool HandleEvent(const char *action);
    virtual bool HandleKeyEvent(nsIDOMKeyEvent *event);
    int PerformMouseAction(nsIDOMMouseEvent *event);
    int BrowserOpen(void);
    int BrowserClose(void);
    void BrowserButtons(void);
    void BrowserEvent(const char *etype, const char *data);
    void BrowserSnapshot(const char *fname);
    void BrowserData(const char *data);
    void BrowserSetChrome(guint mask);
    GtkWidget *BrowserGetEmbed(void);
    void SetStatus(const char *status);
    void SetLocation(const char *url);
    char *GetLocation(void);
    char *GetStatus(void);
    virtual bool HandleMouseEvent(const SDL_Event * event, int xoffset, int yoffset);
    virtual int OnGotFocus(void);
    virtual void OnEvent(void);
    virtual Display *GetDisplay(void);
    virtual Window GetWindow(void);
    void Back(void);
    void Stop(void);
    void Forward(void);
    void Reload(void);
    void Refresh(void);

    char *status;
    char *location;
    int loadPercent;
    int bytesLoaded;
    int maxBytesLoaded;
    bool toolbar_flag;
    GtkWidget *mozilla;
    GtkWidget *toplevel;
    GtkWidget *toplevelVBox;
    GtkWidget *toolbarHBox;
    GtkWidget *toolbar;
    GtkWidget *closeButton;
    GtkWidget *backButton;
    GtkWidget *stopButton;
    GtkWidget *forwardButton;
    GtkWidget *reloadButton;
    GtkWidget *urlEntry;
    GtkWidget *progressAreaHBox;
    GtkWidget *progressHBox;
    GtkWidget *progressBar;
    GtkWidget *statusAlign;
    GtkWidget *statusBar;

    Variant pget_Status(void);
    Variant pget_Version(void);
    Variant pget_Browsers(void);
    Variant pget_Location(void);
    int pset_Location(Variant value);

    Variant m_Data(int numargs, Variant args[]);
    Variant m_Open(int numargs, Variant args[]);
    Variant m_Back(int numargs, Variant args[]);
    Variant m_Stop(int numargs, Variant args[]);
    Variant m_Forward(int numargs, Variant args[]);
    Variant m_Reload(int numargs, Variant args[]);
    Variant m_Refresh(int numargs, Variant args[]);
    Variant m_Close(int numargs, Variant args[]);
    Variant m_Event(int numargs, Variant args[]);
};
#endif

class SystemObject:public DynamicObject {
  public:
    SystemObject(const char *name);
    virtual ~SystemObject(void);
    int Init(void);
    void Shutdown(void);
    void Run(void);
    void Quit(void);
    bool IsQuit(void);
    bool IsRunning(void);
    bool IsShutdown(void);
    int SwitchPage(GUI_Page * page);
    int SwitchPage(const char *pagename);
    int ShowOverlay(const char *pagename);
    GUI_Page *GetCurrentPage(void);
    void AddPage(GUI_Container * page);
    GUI_Page *GetPage(unsigned int index);
    int GetPageCount(void);
    virtual DynamicObject *FindObject(const char *name);
    int Chdir(const char *path);
    void Log(int level, const char *fmt, ...);
    void SetLogLevel(int level);
    int GetLogLevel(void);
    const char *GetNewPage(void);
    const char *GetPrevPage(void);
    char *GetCwd(void);
    char *GetHome(void);
    bool IsEvents(void);
    void SetEvents(bool flag);
    void SetFreeze(int nfreeze);
    void SetFocus(int focus);
    bool HasFocus(void);
    void SetPointer(int x, int y);
    int GetScreenWidth(void);
    int GetScreenHeight(void);
    double GetXScale(void);
    void SetXScale(double scale);
    double GetYScale(void);
    void SetYScale(double scale);
    int GetIdleTime(void);
    unsigned long GetBootTime(void);
    unsigned long GetThreadID(void);
    void SetIdleTime(int time);
    void KeyBind(const char *key, const char *action);
    const char *KeyToAction(int key);
    string KeyBindings(void);
    void SetAppName(const char *app);
    const char *GetAppName(void);
    virtual bool HandleKeyEvent(const SDL_Event *event);
    virtual bool HandleMouseEvent(const SDL_Event *event, int xoffset, int yoffset);
    virtual void PostEvent(const char *action, const char *data1, const char *data2);
    char *GetEvent(void);
    char *GetEventData(int mode);
    char *FindFile(const char *name);
    GUI_Font *LoadFont(string filename, int fontsize);
    GUI_Surface *LoadImage(string filename, bool cache);
    GUI_Screen *GetScreen(void);
    Window CreateWindow(Display *display, Visual *visual, int depth, int x, int y, int w, int h, int border);
    Visual *FindVisual(Display *display, int *depth);
    void SetWindowBorder(Display *display, Window window, int border);
    int IsWindowViewable(Display *display, Window window);
    void LockDisplay(void);
    void UnlockDisplay(void);
    virtual Display *GetDisplay(void);
    virtual Window GetWindow(void);
    virtual void Flip(void);
    virtual void DoUpdate(int force);
    virtual void OnExpose(void);
    void WaitForObject(const char *name, int timeout);
    int FireObject(const char *name, const char *event);
    void configSetInt(const char *name, int value);
    void configSetValue(const char *name, const char *value);
    const char *configGetValue(const char *name, const char *def);
    int configGetInt(const char *name, int def);
    double configGetDouble(const char *name, double def);
    void configList(Tcl_Interp * interp, const char *mask);
    void configUnset(const char *mask);
    const char *varGet(const char *name);
    void varSet(const char *name, const char *value);
    void varAppend(const char *name, const char *value);
    void varUnset(const char *mask);
    void varList(Tcl_Interp * interp, const char *mask);
    int varIncr(const char *name);
    const char *cdromStatus(void);
    int cdromEject(void);
    int cdromClose(void);
    int dbInit(void);
    void dbClose(void);
    void dbSync(void);
    void dbCompact(void);
    char *dbList(void);
    unsigned long dbNextId(const char *key);
    int dbOpen(const char *dbname, int flags);
    int dbDrop(const char *dbname);
    DB *dbFind(const char *dbname);
    int dbDel(const char *dbname, const char *name);
    int dbExists(const char *dbname, const char *name);
    int dbPut(const char *dbname, const char *name, const char *value);
    int dbEval(const char *dbname, void *arg, int (*script)(void *, char*, char*), const char *keystr, int op_start, int op_next);
    unsigned long dbStat(const char *dbname, const char *param);
    char *dbGet(const char *dbname, const char *name);
    void RegisterExternalEventHandler(Object *obj);
    void UnregisterExternalEventHandler(Object *obj);

  protected:
    pthread_mutex_t configmutex;
    pthread_mutex_t cachemutex;
    pthread_mutex_t eventmutex;
    pthread_mutex_t switchmutex;
    Tcl_HashTable cachetable;
    Tcl_HashTable configtable;
    vector < char *>fontpath;
    vector < char *>imagepath;
    vector < GUI_Font * >fonts;
    vector < int >fontsizes;
    vector < GUI_Surface * > images;
    vector < Object * > externaleventhandlers;
    vector < KeyBinding * > keybindings;
    queue < EventEntry > eventqueue;
    EventEntry event;
    char *appname;
    char *cwd;
    char *homedir;
    char *newpage;
    double xscale;
    double yscale;
    int loglevel;
    int freeze;
    DB_ENV *db_env;
    vector < DB * >db_list;
    bool quit_flag;
    bool shutdown_flag;
    bool running_flag;
    int sdlopts;
    int sdlflags;
    bool sdlevents;
    GUI_Screen *screen;
    SDL_SysWMinfo wm;
    SDL_Joystick *joystick;
    unsigned long threadID;
    unsigned long bootTime;
    unsigned long eventTime;

    Variant pget_LogLevel(void);
    int pset_LogLevel(Variant value);
    Variant pget_ThreadID(void);
    Variant pget_Version(void);
    Variant pget_ScreenWidth(void);
    Variant pget_ScreenHeight(void);
    Variant pget_CurrentFile(void);
    Variant pget_PageCount(void);
    Variant pget_BootTime(void);
    Variant pget_IdleTime(void);
    int pset_IdleTime(Variant value);
    Variant pget_Focus(void);
    Variant pget_XScale(void);
    int pset_XScale(Variant value);
    Variant pget_YScale(void);
    int pset_YScale(Variant value);
    Variant pget_FontCache(void);
    Variant pget_ImageCache(void);

    Variant m_Quit(int numargs, Variant args[]);
    Variant m_FocusNext(int numargs, Variant args[]);
    Variant m_FocusPrevious(int numargs, Variant args[]);
    Variant m_SelectFocused(int numargs, Variant args[]);
    Variant m_HandleEvent(int numargs, Variant args[]);
    Variant m_GotoPage(int numargs, Variant args[]);
    Variant m_Refresh(int numargs, Variant args[]);
};

extern SystemObject *systemObject;

extern const SDL_Color DEFAULT_COLOR;
extern const SDL_Color DEFAULT_FONTCOLOR;
extern const SDL_Color DEFAULT_SELECTED_FONTCOLOR;
extern const bool gfxfont[256][8][8];
extern const char *digitchars[];

// Keyboard/modifiers API

void printKeyNames(void);
string keyToName(int key);
string modToName(int mod);
int nameToKey(const char *name);
int nameToMod(const char *name);
bool checkModKeys(int pressedmod, int expectedmod);

// String API

char *intString(int val);
char *uintString(unsigned long val);
char *trimString(const char *string);
int countString(const char *str, char c);
char *stripHtmlString(char *str,char *tags[]);
char *strTrim(char *s, const char *spaces);
char * strUpper(char *str);
char * strLower(char *str);
char *strFileRead(const char *fname, int *size);
int matchString(const char *str, const char *pattern);
int regexString(const char *str, const char *pattern);
SDL_Color strToColor(const char *str);
string colorToStr(SDL_Color color);
unsigned int colorToInt(SDL_Color c);
int styleStrToInt(const string & stylestr);
string styleIntToStr(int style);
int borderStrToInt(const string & stylestr);
string borderIntToStr(int style);
bool strToBool(const char *str);
bool isEmptyString(const char *str);
char *urlDecodeString(const char *str);
char *urlEncodeString(const char *data);
char *decode64String(const char *p);
char *encode64String(const char *str);
char *encodeHexString(const char *str);
char *decodeHexString(const char *buf, int len);
int strSplitn(vector<char*> *lst, const char *str, const char *delim, int limit, char *quotes);
int strSplit(vector<char*> *lst, const char *str, const char *delim);

// System API

double getTime(void);
bool compareTimes(timeval * t1, timeval * t2);
regex_t *parseRegex(const char *str);
bool matchRegex(const char *str, regex_t *regex);
void freeRegex(regex_t *regex);
int parseFile(const char *filename);
int parsePath(const char *path);
void parseFree(void);
void tlsInit(void);
void tlsShutdown(void);
Tls *tlsGet(void);
char *tlsAlloc(int idx, int len);
char *tlsStr(int idx, const char *s);
char *tlsStrAppend(int idx, const char *s);
char *tlsSprintf(int idx, const char *fmt, ...);
int rectInside(const SDL_Rect * r1, const SDL_Rect * r2);
int Inside(int x, int y, SDL_Rect * r);
uint8_t *yuy2torgb (const uint8_t *data, const int width, const int height);
uint8_t *yv12torgb (const uint8_t *data, const int width, const int height);
int sortcmp(const struct dirent **a, const struct dirent **b);
char *lmbox_vprintf(const char *fmt, va_list ap);
char *lmbox_printf(const char *fmt, ...);
char *lmbox_strdup(const char *str);
void lmbox_free(void *ptr);
void lmbox_destroy(void **ptr);
int scriptInit(void);
void scriptShutdown(void);
void scriptDealloc(Tcl_Interp *interp);
Tcl_Interp *scriptAlloc(void);
int threadCreate(void *(*func)(void*), void *arg);
void threadNameSet(const char *name, ...);
void threadNameClear(const char *name);
void TTF_Quit(void);
int TTF_Init(void);

// Socket API

int sock_listen_udp(int port);
int sock_listen_tcp(int port);
int sock_connect(struct sockaddr_in *server, int timeout);
int sock_select(int fd, int mode, int timeout);
int sock_blocking(int fd);
int sock_nonblocking(int fd);
void sock_nodelay(int fd);
char *sock_inet_str(struct in_addr addr);
int sock_is_ipaddr(const char *addr);
u_long sock_inet_addr(const char *str);
void sock_buffer(int fd, int snd, int size);
int socket_write(int fd, const char *buf, int len, int timeout);
char *sock_inet_ntoa(unsigned long ip, char *buf);
int sock_recv(int fd, char *buf, size_t toread, int timeout);
int sock_read(int fd, char *buf, int len, int timeout);
int sock_read_line(int fd, char *data, int size, int timeout);
int sock_read_data(int fd, char **data, int size, int timeout);
u_long sock_gethostbyname(const char *host, vector <u_long> *alist);
int sock_geturl(const char *mrl, int timeout, const char *headers, char **odata, int *osize, string *oheaders);

#endif
