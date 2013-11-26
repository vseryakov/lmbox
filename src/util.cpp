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

#define CLIP_8BIT(val)           ((val) < 0 ? 0 : (val) > 255 ? 255 : (val))
#define RFC850_STRFTIME          "%A, %d-%b-%Y %H:%M:%S GMT"
#define RFC1123_STRFTIME         "%a, %d %b %Y %H:%M:%S GMT"
#define BASE64_VALUE_SZ          256
#define BUFSIZE                  64

#define URL_XALPHAS              (unsigned char) 1
#define URL_XPALPHAS             (unsigned char) 2
#define URL_PATH                 (unsigned char) 4

/* This matches the escape set when mask is 7 (default.) */
#define IS_OK(C, mask)           (netCharType[((unsigned char) (C))] & (mask))

// The maximum number of digits of accuracy in a floating-point conversion.
#define MAXDIG                   20

// Conversion types fall into various categories as defined by the following enumeration.
#define RADIX                   1   // Integer types.  %d, %x, %o, and so forth 
#define FLOAT                   2   // Floating point.  %f 
#define EXP                     3   // Exponentional notation. %e and %E 
#define GENERIC                 4   // Floating or exponential, depending on exponent. %g 
#define SIZE                    5   // Return number of characters processed so far. %n 
#define STRING                  6   // Strings. %s 
#define PERCENT                 7   // Percent symbol. %% 
#define CHAR                    8   // Characters. %c 
#define ERROR                   9   // Used to indicate no such conversion type 

// The rest are extensions, not normally found in printf()
#define CHARLIT                 10  // Literal characters.  %' 
#define SEEIT                   11  // Strings with visible control characters. %S 
#define MEM_STRING              12  // A string which should be deleted after use. %z 
#define ORDINAL                 13  // 1st, 2nd, 3rd and so forth 
#define ESCAPE                  14  // string which should be escaped as per RFC1738 
#define HEX                     15  // string which should be converted into hex 
#define BASE64                  16  // string which should be converted into base64 encoding 
#define LOCALTIME               17  // RFC1123 local date time string 
#define GMTTIME                 18  // RFC1123 GMT date time string 
#define RFC850TIME              19  // RFC850 GMT date time string 
#define IPADDR                  20  // IP address string represenation 

// Each builtin conversion character (ex: the 'd' in "%d") is described
// by an instance of the following structure
typedef struct {             // Information about each format field
  int  fmttype;              // The format field code letter
  int  base;                 // The base for radix conversion
  char *charset;             // The character set for conversion
  int  flag_signed;          // Is the quantity signed?
  char *prefix;              // Prefix on non-zero values in alt format
  int type;                  // Conversion paradigm
} ixprintf;

typedef struct {
  char *base;                // A base allocation
  char *text;               // The string collected so far
  int  size;                 // Length of the string so far
  int  alloc;                // Amount of space allocated in text
} mxprintf;

static const unsigned char netCharType[256] =
// Bit 0      xalpha      -- the alphas
// Bit 1      xpalpha     -- as xalpha but converts spaces to plus and plus to %20
// Bit 2 ...  path        -- as xalphas but doesn't escape '/'

  //   0 1 2 3 4 5 6 7 8 9 A B C D E F
  {    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       // 0x 
       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       // 1x 
       0,0,0,0,0,0,0,0,0,0,7,4,0,7,7,0,       // 2x   !"#$%&'()*+,-./  
       7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,0,       // 3x  0123456789:;<=>?  
       7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       // 4x  @ABCDEFGHIJKLMNO  
       7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,7,       // 5X  PQRSTUVWXYZ[\]^_  
       0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       // 6x  `abcdefghijklmno  
       7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,       // 7X  pqrstuvwxyz{\}~  DEL 
       0, };

// The following table is searched linearly, so it is good to put the
// most frequently used conversion types first.
static ixprintf fmtinfo[] = {
  { 'd',  10,  "0123456789",       1,    0, RADIX,      },
  { 's',   0,  0,                  0,    0, STRING,     }, 
  { 'S',   0,  0,                  0,    0, SEEIT,      },
  { '@',   0,  0,                  0,    0, ESCAPE,     },
  { 'B',   0,  0,                  0,    0, BASE64,     },
  { 'H',   0,  0,                  0,    0, HEX,        },
  { 't',   0,  0,                  0,    0, LOCALTIME,  },
  { 'T',   0,  0,                  0,    0, GMTTIME,    },
  { 'R',   0,  0,                  0,    0, RFC850TIME, },
  { '^',   0,  0,                  0,    0, IPADDR,     },
  { 'z',   0,  0,                  0,    0, MEM_STRING, },
  { 'c',   0,  0,                  0,    0, CHAR,       },
  { 'o',   8,  "01234567",         0,  "0", RADIX,      },
  { 'u',  10,  "0123456789",       0,    0, RADIX,      },
  { 'x',  16,  "0123456789abcdef", 0, "x0", RADIX,      },
  { 'X',  16,  "0123456789ABCDEF", 0, "X0", RADIX,      },
  { 'r',  10,  "0123456789",       0,    0, ORDINAL,    },
  { 'f',   0,  0,                  1,    0, FLOAT,      },
  { 'e',   0,  "e",                1,    0, EXP,        },
  { 'E',   0,  "E",                1,    0, EXP,        },
  { 'g',   0,  "e",                1,    0, GENERIC,    },
  { 'G',   0,  "E",                1,    0, GENERIC,    },
  { 'i',  10,  "0123456789",       1,    0, RADIX,      },
  { 'n',   0,  0,                  0,    0, SIZE,       },
  { 'S',   0,  0,                  0,    0, SEEIT,      },
  { '%',   0,  0,                  0,    0, PERCENT,    },
  { 'b',   2,  "01",               0, "b0", RADIX,      }, // Binary notation 
  { 'p',  10,  "0123456789",       0,    0, RADIX,      }, // Pointers 
  { '\'',  0,  0,                  0,    0, CHARLIT,    }, // Literal char 
  { 0,     0,  0,                  0,    0, 0,          }
};

static pthread_key_t key;
const char hexchars[] = "0123456789ABCDEF";
static int base64_value[BASE64_VALUE_SZ];
const char base64_code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
// This array defines what characters are available using keypad (digit) text input
const char *digitchars[] = { "0 -+/@#$%&*", "1.,:;?!\'\"", "abc2", "def3", "ghi4", "jkl5", "mno6", "pqrs7", "tuv8", "wxyz9", NULL };

static const struct {
    const char *keystr;
    int key;
} keynamelist[] = {
    {
    "backspace", SDLK_BACKSPACE}, {
    "tab", SDLK_TAB}, {
    "enter", SDLK_RETURN}, {
    "pause", SDLK_PAUSE}, {
    "escape", SDLK_ESCAPE}, {
    "space", SDLK_SPACE}, {
    "quote", SDLK_QUOTE}, {
    "comma", SDLK_COMMA}, {
    "minus", SDLK_MINUS}, {
    "plus", SDLK_PLUS}, {
    "dot", SDLK_PERIOD}, {
    "slash", SDLK_SLASH}, {
    "0", SDLK_0}, {
    "1", SDLK_1}, {
    "2", SDLK_2}, {
    "3", SDLK_3}, {
    "4", SDLK_4}, {
    "5", SDLK_5}, {
    "6", SDLK_6}, {
    "7", SDLK_7}, {
    "8", SDLK_8}, {
    "9", SDLK_9}, {
    "semicolon", SDLK_SEMICOLON}, {
    "equals", SDLK_EQUALS}, {
    "leftbracket", SDLK_LEFTBRACKET}, {
    "backslash", SDLK_BACKSLASH}, {
    "rightbracket", SDLK_RIGHTBRACKET}, {
    "backquote", SDLK_BACKQUOTE}, {
    "a", SDLK_a}, {
    "b", SDLK_b}, {
    "c", SDLK_c}, {
    "d", SDLK_d}, {
    "e", SDLK_e}, {
    "f", SDLK_f}, {
    "g", SDLK_g}, {
    "h", SDLK_h}, {
    "i", SDLK_i}, {
    "j", SDLK_j}, {
    "k", SDLK_k}, {
    "l", SDLK_l}, {
    "m", SDLK_m}, {
    "n", SDLK_n}, {
    "o", SDLK_o}, {
    "p", SDLK_p}, {
    "q", SDLK_q}, {
    "r", SDLK_r}, {
    "s", SDLK_s}, {
    "t", SDLK_t}, {
    "u", SDLK_u}, {
    "v", SDLK_v}, {
    "w", SDLK_w}, {
    "x", SDLK_x}, {
    "y", SDLK_y}, {
    "z", SDLK_z}, {
    "delete", SDLK_DELETE}, {
    "keypad0", SDLK_KP0}, {
    "keypad1", SDLK_KP1}, {
    "keypad2", SDLK_KP2}, {
    "keypad3", SDLK_KP3}, {
    "keypad4", SDLK_KP4}, {
    "keypad5", SDLK_KP5}, {
    "keypad6", SDLK_KP6}, {
    "keypad7", SDLK_KP7}, {
    "keypad8", SDLK_KP8}, {
    "keypad9", SDLK_KP9}, {
    "keypaddot", SDLK_KP_PERIOD}, {
    "keypadslash", SDLK_KP_DIVIDE}, {
    "keypadstar", SDLK_KP_MULTIPLY}, {
    "keypadminus", SDLK_KP_MINUS}, {
    "keypadplus", SDLK_KP_PLUS}, {
    "keypadenter", SDLK_KP_ENTER}, {
    "keypadequals", SDLK_KP_EQUALS}, {
    "uparrow", SDLK_UP}, {
    "downarrow", SDLK_DOWN}, {
    "rightarrow", SDLK_RIGHT}, {
    "leftarrow", SDLK_LEFT}, {
    "insert", SDLK_INSERT}, {
    "home", SDLK_HOME}, {
    "end", SDLK_END}, {
    "pageup", SDLK_PAGEUP}, {
    "pagedown", SDLK_PAGEDOWN}, {
    "f1", SDLK_F1}, {
    "f2", SDLK_F2}, {
    "f3", SDLK_F3}, {
    "f4", SDLK_F4}, {
    "f5", SDLK_F5}, {
    "f6", SDLK_F6}, {
    "f7", SDLK_F7}, {
    "f8", SDLK_F8}, {
    "f9", SDLK_F9}, {
    "f10", SDLK_F10}, {
    "f11", SDLK_F11}, {
    "f12", SDLK_F12}, {
    "f13", SDLK_F13}, {
    "f14", SDLK_F14}, {
    "f15", SDLK_F15}, {
    "numlock", SDLK_NUMLOCK}, {
    "capslock", SDLK_CAPSLOCK}, {
    "scrolllock", SDLK_SCROLLOCK}, {
    "rshift", SDLK_RSHIFT}, {
    "lshift", SDLK_LSHIFT}, {
    "rctrl", SDLK_RCTRL}, {
    "lctrl", SDLK_LCTRL}, {
    "ralt", SDLK_RALT}, {
    "lalt", SDLK_LALT}, {
    "lwin", SDLK_LSUPER}, {
    "rwin", SDLK_RSUPER}, {
    "printscreen", SDLK_PRINT}, {
    "menu", SDLK_MENU}, {
    "joystickup", SDL_HAT_UP}, {
    "joystickdown", SDL_HAT_DOWN}, {
    "joystickleft", SDL_HAT_LEFT}, {
    "joystickright", SDL_HAT_RIGHT}, {
    NULL, SDLK_UNKNOWN}
};

// Modifier key string to constant map
static const struct {
    const char *modstr;
    int mod;
} modkeynamelist[] = {
    {
    "shift", KMOD_SHIFT}, {
    "lshift", KMOD_LSHIFT}, {
    "rshift", KMOD_RSHIFT}, {
    "ctrl", KMOD_CTRL}, {
    "lctrl", KMOD_LCTRL}, {
    "rctrl", KMOD_RCTRL}, {
    "alt", KMOD_ALT}, {
    "lalt", KMOD_LALT}, {
    "ralt", KMOD_RALT}, {
    "caps", KMOD_CAPS}, {
    "num", KMOD_NUM}, {
    "lmeta", KMOD_LMETA}, {
    "rmeta", KMOD_RMETA}, {
    "none", KMOD_NONE}, {
    NULL, KMOD_NONE}
};

static int htod(char c)
{
     if (c >= '0' && c <= '9') {
         return c - '0';
     }
     if (c >= 'A' && c <= 'F') {
         return c - 'A' + 10;
     }
     if (c >= 'a' && c <= 'f') {
         return c - 'a' + 10;
     }
     return 0;
}

static void tlsFree(void *arg)
{
    Tls *tls = (Tls*)arg;
    if (tls->interp) {
       scriptDealloc(tls->interp);
    }
    for (int i = 0; i < TLS_STRSIZE; i++) {
        lmbox_free(tls->str[i].data);
    }
    threadNameClear(tls->threadname);
    lmbox_free(tls->event);
    free(tls);
    pthread_setspecific(key, NULL);
}

void tlsInit()
{
    pthread_key_create(&key, tlsFree);
    for(int i = 0; i < BASE64_VALUE_SZ; i++) base64_value[i] = -1;
    for(int i = 0; i < 64; i++) base64_value[(int) base64_code[i]] = i;
    base64_value['='] = 0;
}

void tlsShutdown(void)
{
    Tls *tls = (Tls*)pthread_getspecific(key);
    if (tls) {
        tlsFree(tls);
        pthread_setspecific(key, NULL);
    }
}

Tls *tlsGet(void)
{
    Tls *tls;

    tls = (Tls*)pthread_getspecific(key);
    if (tls == NULL) {
        tls = (Tls*)calloc(1, sizeof(Tls) + BUFFER_SIZE + 1);
        pthread_setspecific(key, tls);
    }
    return tls;
}

char *tlsAlloc(int idx, int len)
{
    Tls *tls = tlsGet();
    if (len >= tls->str[idx].len) {
        len = tls->str[idx].len + len * 2 + 1;
        tls->str[idx].data = (char*)realloc(tls->str[idx].data, len);
        tls->str[idx].data[tls->str[idx].len] = 0;
        tls->str[idx].len = len;
    }
    return tls->str[idx].data;
}

char *tlsStr(int idx, const char *str)
{
    char *s = tlsAlloc(idx, str ? strlen(str) : 32);
    strcpy(s, str ? str : "");
    return s;
}

char *tlsStrAppend(int idx, const char *str)
{
    char *s = tlsAlloc(idx, strlen(tlsAlloc(idx, 0)) + strlen(str));
    strcat(s, str);
    return s;
}

char *tlsSprintf(int idx, const char *fmt, ...)
{
    Tls *tls = tlsGet();
    va_list ap;
    va_start(ap, fmt);
    if (!tls->str[idx].data) {
        tls->str[idx].len = strlen(fmt) + 1;
        tls->str[idx].data = (char*)malloc(tls->str[idx].len);
    }
    while(1) {
      int sz = vsnprintf(tls->str[idx].data, tls->str[idx].len, fmt, ap);
      if (sz < 0 || sz >= tls->str[idx].len) {
          tls->str[idx].len *= 2;
          tls->str[idx].data = (char*)realloc(tls->str[idx].data, tls->str[idx].len);
      } else {
        break;
      }
    }
    va_end(ap);
    return tls->str[idx].data;
}

void printKeyNames()
{
    cout << "Keyboard key names:" << endl;
    for (int i = 0; keynamelist[i].keystr; i++) {
        cout << keynamelist[i].keystr << " ";
    }
    cout << endl << endl << "Keyboard modifier flags:" << endl;
    for (int i = 0; modkeynamelist[i].modstr; i++) {
        cout << modkeynamelist[i].modstr << " ";
    }
    cout << endl;
}

string keyToName(int key)
{
    for (int i = 0; keynamelist[i].keystr; i++) {
        if (keynamelist[i].key == key) {
            return keynamelist[i].keystr;
        }
    }
    return "";
}

int nameToKey(const char *name)
{
    for (int i = 0; keynamelist[i].keystr; i++) {
        if (!strcmp(name, keynamelist[i].keystr)) {
            return keynamelist[i].key;
        }
    }
    return -1;
}

int nameToMod(const char *name)
{
    for (int i = 0; modkeynamelist[i].modstr; i++) {
        if (!strcmp(name, modkeynamelist[i].modstr)) {
            return modkeynamelist[i].mod;
        }
    }
    return -1;
}

string modToName(int mod)
{
    string ret;
    for (int i = 0; modkeynamelist[i].modstr; i++) {
        if (modkeynamelist[i].mod != KMOD_NONE) {
            if ((mod & modkeynamelist[i].mod) == modkeynamelist[i].mod) {
                if (ret.length() > 0) {
                    ret += "+";
                }
                ret += modkeynamelist[i].modstr;
            }
        }
    }
    return ret;
}

bool checkModKeys(int pressedmod, int expectedmod)
{
    if (expectedmod == KMOD_NONE && pressedmod == KMOD_NONE) {
        return true;
    }
    if (expectedmod == KMOD_META && pressedmod & KMOD_META) {
        return true;
    }
    if (pressedmod & expectedmod) {
        return true;
    }
    return false;
}

int sortcmp(const struct dirent **a, const struct dirent **b)
{
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

int matchString(const char *str, const char *pattern)
{
    int x, y;

    for (x = 0, y = 0; pattern[y]; ++y, ++x) {
        if (!str[x] && pattern[y] != '*') {
            return 1;
        }
        if (pattern[y] == '*') {
            while (pattern[++y] == '*');
            if (!pattern[y]) {
                return 0;
            }
            while (str[x]) {
                if (!matchString(&str[x++], &pattern[y])) {
                    return 0;
                }
            }
            return 1;
        } else
        if (pattern[y] != '?' && tolower(str[x]) != tolower(pattern[y])) {
            return 1;
        }
    }
    return str[x] != '\0';
}

char *stripHtmlString(char *str,char *tags[])
{
    int i;
    char *tag = 0,*ptr,*buf,*end,c;
           
    ptr = buf = str = lmbox_strdup(str);
           
    while (ptr && *ptr) {
      switch (*ptr) {
       case '<':
           tag = ++ptr;
           if (!strncmp(tag,"!--",3)) {
               if ((end = strstr(tag+3,"-->"))) {
                   ptr = end+2;
                   tag = 0;
                   break;
               }
               *ptr = 0;
               tag = 0;
               break;
           }
           break;
       case '>':
           if (tag) {
               while(*tag == ' ') tag++;
               for (end = tag;end < ptr && *end != ' ';end++);
               c = *end,*end = 0;
               for (i = 0; tags && tags[i]; i++) {
                   if (!strcasecmp(tag,tags[i])) {
                       break;
                   }
               }
               *end = c;
               // No tags means strip all
               if (tags && !tags[i]) {
                  *buf++ = '<';
                  while (tag <= ptr) {
                      *buf++ = *tag++;
                  }
               }
               tag = 0;
           }
           ptr++;
           break;
       default:
           if (!tag) {
               *buf++ = *ptr;
           }         
           ptr++;
           break;
      }
    }
    *buf = '\0';
    return str;
}

// Trim off the whitespace at the start and end of the string
char *trimString(const char *string)
{
    char *newStart = 0;
    const char *p;
    int lastpos;

    if (string) {
        p = string;
        for (unsigned int i = 0; i < strlen(string); i++) {
            if (isspace(string[i])) {
                p++;
            } else {
                break;
            }
        }
        // Zero out all whitespace at the end of the string
        newStart = strdup(p);
        lastpos = strlen(newStart) - 1;
        for (int i = lastpos; i > 0; i--) {
            if (isspace(newStart[i])) {
                newStart[i] = '\0';
            } else {
                break;
            }
        }
    }
    return newStart;
}

char *strTrim(char *s, const char *spaces)
{
    static char *defspaces = " \r\n";
    char *start, *end;

    if (s && *s) {
        if (!spaces || !*spaces) {
            spaces = defspaces;
        }
        for (start = s; *start && strchr(spaces, *start);start++) {}
        for (end = s + strlen(s) - 1; end > s && strchr(spaces, *end); end--) {
            *end = '\0';
        }
        if (s != start) {
            memmove(s, start, strlen(start)+1);
        }
    }
    return s;
}

char * strLower(char *string)
{
    char *s = string;

    while (*s != '\0') {
        if (isupper(*s)) {
            *s = tolower(*s);
        }
        ++s;
    }
    return string;
}

char * strUpper(char *string)
{
    char *s = string;

    while (*s != '\0') {
        if (islower(*s)) {
            *s = toupper(*s);
        }
        ++s;
    }
    return string;
}

char *strFileRead(const char *fname, int *size)
{
    int fd;
    char *buf = 0;
    struct stat st;

    if (stat(fname, &st) < 0) {
        return 0;
    }
    if ((fd = open(fname, O_RDONLY)) < 0) {
        return 0;
    }
    buf = (char*)malloc(st.st_size + 1);
    st.st_size = read(fd, buf, st.st_size);
    close(fd);
    if (size) {
        *size = st.st_size;
    }
    buf[st.st_size] = 0;
    return buf;
}

// quotes consits from pairs of symbols, opening and closing symbols "" '' [] {}
int strSplitn(vector<char*> *lst, const char *str, const char *delim, int limit, char *quotes)
{
   char *q, *r, *s, *p;
   int i, count = 0, qlen = quotes ? strlen(quotes) : 0;

   if (!str || !delim) {
       return 0;
   }
   s = r = strdup(str);
   while (1) {
       q = r + strspn(r,delim);    /* skip leading separators */
       if (!*q) {
           break;
       }
       count++;
       // Token can be inside "string"
       for (i = 0;i < qlen;i += 2) {
           if (quotes[i] == *q) {
               break;
           }
       }
       if (i < qlen && (r = strchr(q + 1, quotes[i+1]))) {
          q++;
       } else {
          r = strpbrk(q, delim);
       }
       // Create item using size or to the end of the string
       p = strndup(q, (r ? r-q : strlen(q)));
       lst->push_back(p);
       if (!r || --limit == 0) {
           break;
       }
       // Move past delimiter
       r++;
   }
   free(s);
   return lst->size();
}

int strSplit(vector<char*> *lst, const char *str, const char *delim)
{
    return strSplitn(lst, str, delim, 0, 0);
}

// Convert an HTML colour string (#rrggbb) into an SDL_Color structure
SDL_Color strToColor(const char *str)
{
    SDL_Color color = { 0, 0, 0, 0 };
    int h;

    if (str && str[0] == '#') {
        sscanf(str + 1, "%6X", &h);
        color.r = (h & 0xFF0000) >> 16;
        color.g = (h & 0xFF00) >> 8;
        color.b = (h & 0xFF);
    }
    return color;
}

// Convert an SDL_Color structure into an HTML colour string (#rrggbb)
string colorToStr(SDL_Color color)
{
    char colorstr[10];
    snprintf(colorstr, 8, "#%02X%02X%02X", color.r, color.g, color.b);
    return anytovariant(colorstr);
}

// Convert a string value into a boolean
bool strToBool(const char *str)
{
    if (strcasecmp(str, "enable") == 0 ||
        strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "on") == 0 ||
        strcasecmp(str, "yes") == 0 ||
        atoi(str) != 0) {
        return true;
    }
    return false;
}

// Routine to get the current time as float
double getTime(void)
{
    struct timeval tv;
    double curtime;
    gettimeofday(&tv, NULL);
    curtime = (double) tv.tv_sec + 1.0e-6 * (double) tv.tv_usec;
    return curtime;
}

bool isEmptyString(const char *str)
{
    for (int i = 0; str && i < strlen(str); i++) {
        if (!isspace(str[i])) {
            return false;
        }
    }
    return true;
}

int borderStrToInt(const string & style)
{
    if (style == "normal")
        return BORDER_NORMAL;
    else
    if (style == "round")
        return BORDER_ROUND;
    else
    if (style == "round_double")
        return BORDER_ROUND_DBL;
    else
        return BORDER_NONE;
}

string borderIntToStr(int style)
{
    switch (style) {
     case BORDER_ROUND:
          return "round";
     case BORDER_ROUND_DBL:
          return "round_double";
     case BORDER_NORMAL:
          return "normal";
     default:
          return "none";
   }
}
int styleStrToInt(const string & style)
{
    if (style == "normal")
        return BACKGROUND_NORMAL;
    else
    if (style == "tiled")
        return BACKGROUND_TILED;
    else
    if (style == "fill")
        return BACKGROUND_FILL;
    else
    if (style == "gradient")
        return BACKGROUND_GRADIENT;
    else
    if (style == "scaled")
        return BACKGROUND_SCALED;
    else
    if (style == "color")
        return BACKGROUND_COLOR;
    else
        return BACKGROUND_NONE;
}

string styleIntToStr(int style)
{
    switch (style) {
     case BACKGROUND_FILL:
          return "fill";
     case BACKGROUND_SCALED:
          return "scaled";
     case BACKGROUND_TILED:
          return "tiled";
     case BACKGROUND_GRADIENT:
          return "gradient";
     case BACKGROUND_NORMAL:
          return "normal";
     case BACKGROUND_COLOR:
          return "color";
     default:
        return "none";
   }
}

int regexString(const char *str, const char *pattern)
{
    int rc = -1;
    regex_t *regex = parseRegex(pattern);
    if (regex) {
        rc = matchRegex(str, regex);
        freeRegex(regex);
    }
    return rc;
}

regex_t *parseRegex(const char *str)
{
    regex_t *regex = (regex_t *) calloc(1, sizeof(regex_t));

    int rc = regcomp(regex, str, REG_EXTENDED | REG_ICASE | REG_NOSUB);
    if (rc != 0) {
        char buffer[255];
        regerror(rc, regex, buffer, sizeof(buffer));
        systemObject->Log(0, "parseRegex: %s", buffer);
        freeRegex(regex);
        return 0;
    }
    return regex;
}

bool matchRegex(const char *str, regex_t *regex)
{
    int rc = regexec(regex, str, 0, NULL, 0);
    if (rc && rc != REG_NOMATCH) {
        char buffer[255];
        regerror(rc, regex, buffer, sizeof(buffer));
        systemObject->Log(0, "matchRegex: %s", buffer);
    }
    return rc;
}

void freeRegex(regex_t *regex)
{
    if (regex) {
        regfree(regex);
        free(regex);
    }
}

char *intString(int val)
{
    Tls *tls = tlsGet();
    sprintf(tls->ibuf, "%d", val); 
    return tls->ibuf;
}

char *uintString(unsigned long val)
{
    Tls *tls = tlsGet();
    sprintf(tls->ibuf, "%ul", val); 
    return tls->ibuf;
}

char *urlDecodeString(const char *str)
{
    Tls *tls = tlsGet();
    string rc;

    while(str && *str) {
     if (str[0] == '%' && str[1] && str[2]) {
         rc += htod(str[1]) * 16 + htod(str[2]);
         str += 3;
     } else
     if (str[0] == '+') {
         rc += " ";
         str++;
     } else {
         rc += str[0];
         str++;
     }
    }
    return tlsStr(1, rc.c_str());
}

char *urlEncodeString(const char *data)
{
    unsigned char ch,*newdata;
    int mask =  URL_XALPHAS | URL_XPALPHAS | URL_PATH;
    int newlength = 0,i,ni,len = strlen((char*)data);

    if (!data) {
        return 0;
    }
    // Take a first pass and see how big the result string will need to be.
    for (i = 0,newlength = len; i < len; i++) {
        if ((ch = data[i]) < 128 && IS_OK(ch, mask)) {
            continue;
        }
        // The character will be encoded as '+'
        if (mask == URL_XPALPHAS && ch == ' ') {
            continue;
        }
        newlength += 2; // The character will be encoded as %XX
    }
    newdata = (unsigned char *)malloc(newlength + 1);
    for (i = 0, ni = 0; i < len; i++) {
        if ((ch = data[i]) < 128 && IS_OK(ch, mask)) {
            newdata[ni++] = ch;
        } else {
            if (mask == URL_XPALPHAS && ch == ' ') {
                newdata[ni++] = '+'; // convert spaces to pluses
            } else {
                newdata[ni++] = '%';
                newdata[ni++] = digits[ch >> 4];
               newdata[ni++] = digits[ch & 0xF];
            }
        }
    }
    newdata[newlength] = 0;
    return (char*)newdata;
}

// Count the number of instances of character c in string str
int countString(const char *str, char c)
{
    int count = 0;
    const char *p = str;
    while (true) {
        p = strchr(p, c);
        if (p) {
            count++;
            p++;
        } else
            break;
    }
    return count;
}

/*
 * return true if r1 is inside r2 
 */

int rectInside(const SDL_Rect * r1, const SDL_Rect * r2)
{
    return r1->x >= r2->x && r1->x + r1->w <= r2->x + r2->w && r1->y >= r2->y && r1->y + r1->h <= r2->y + r2->h;
}

int Inside(int x, int y, SDL_Rect * r)
{
    return (x >= r->x) && (x < r->x + r->w) && (y >= r->y) && (y < r->y + r->h);
}

bool compareTimes(timeval * t1, timeval * t2)
{
    timeval t2a;

    t2a.tv_usec = t2->tv_usec;
    t2a.tv_sec = t2->tv_sec;

    /* Perform the carry for the later subtraction by updating y. */
    if (t1->tv_usec < t2a.tv_usec) {
        int nsec = (t2a.tv_usec - t1->tv_usec) / 1000000 + 1;
        t2a.tv_usec -= 1000000 * nsec;
        t2a.tv_sec += nsec;
    }
    if (t1->tv_usec - t2a.tv_usec > 1000000) {
        int nsec = (t1->tv_usec - t2a.tv_usec) / 1000000;
        t2a.tv_usec += 1000000 * nsec;
        t2a.tv_sec -= nsec;
    }
    return t1->tv_sec < t2a.tv_sec;
}

uint8_t *yuy2torgb (const uint8_t *data, const int width, const int height)
{
  /* Packed YUV 422 is: Y1, U, Y2, V for each (horizontal) pixel pair.
   * Left pixel is Y1,U,V, right pixel is Y2,U,V.
   */

  int i;
  uint8_t *rgb, *ptr;

  ptr = rgb = (uint8_t*)malloc (width * height * 3);
  if (!rgb) {
    return NULL;
  }
  for (i = 0; i < width * height / 2; ++i) {
    int y = (data[0] - 16) * 11644;
    int u = data[1] - 128;
    int v = data[3] - 128;

    *ptr++ = CLIP_8BIT ((y + 15960 * v) / 10000);               /* R */
    *ptr++ = CLIP_8BIT ((y - 3918 * u - 8130 * v) / 10000);     /* G */
    *ptr++ = CLIP_8BIT ((y + 20172 * u) / 10000);               /* B */

    y = (data[2] - 16) * 11644;
    data += 4;

    *ptr++ = CLIP_8BIT ((y + 15960 * v) / 10000);               /* R */
    *ptr++ = CLIP_8BIT ((y - 3918 * u - 8130 * v) / 10000);     /* G */
    *ptr++ = CLIP_8BIT ((y + 20172 * u) / 10000);               /* B */
  }
  return rgb;
}

uint8_t *yv12torgb (const uint8_t *data, const int width, const int height)
{
  /* YV12 is planar: Y h*w, U (h/2)*(w/2), V (h/2)*(w/2).
   * => Each U and V byte is for 4 pixels (2x2).
   */

  const uint8_t *src_y = data;                          /* [h][w] */
  const uint8_t *src_u = data + width * height;         /* [h/2][w/2] */
  const uint8_t *src_v = data + width * height * 5 / 4; /* [h/2][w/2] */
  int i, j;
  uint8_t *rgb, *ptr;

  ptr = rgb = (uint8_t*)malloc (width * height * 3);
  if (!rgb) {
    return NULL;
  }
  for (i = 0; i < height; ++i) {
    for (j = 0; j < width; ++j) {
      int y = (src_y[j] - 16) * 11644;
      int u = src_u[j / 2] - 128;
      int v = src_v[j / 2] - 128;

      *ptr++ = CLIP_8BIT ((y + 15960 * v) / 10000);             /* R */
      *ptr++ = CLIP_8BIT ((y - 3918 * u - 8130 * v) / 10000);   /* G */
      *ptr++ = CLIP_8BIT ((y + 20172 * u) / 10000);             /* B */
    }
    src_y += width;
    if (i & 1) {
      src_u += width / 2;
      src_v += width / 2;
    }
  }
  return rgb;
}

// Convert string into XX hexadecimal string
// Allocate a new string and return a pointer to converted string
char *encodeHexString(const char *buf, int len)
{
    char *s;
    int i,j;

    if (!buf || !len) {
        return 0;
    }
    s = (char*)calloc(2,len + 1);
    for (j = 0,i = 0;i < len;i++) {
        s[j++] = hexchars[(buf[i] >> 4) & 0x0F];
        s[j++] = hexchars[buf[i] & 0x0F];
    }
    return s;
}

// Convert xx in url string to a character
// Allocate a new string and return a pointer to converted string
char *decodeHexString(const char *str)
{
    char code[] = "00";
    char *p,*s,*t;
    if(!str) return 0;
    p = strdup(str);
    for(s = t = p;*s;) {
      if(isxdigit(*s) && isxdigit(*(s+1))) {
        code[0] = *s++;
        code[1] = *s++;
        *t++ = (char)strtol(code,NULL,16);
      } else
        *t++ = *s++;
    }
    *t = 0;
    return p;
}

char *decode64String(const char *p)
{   
    char *result;
    int j,len,c;
    unsigned int k;
    long val;

    if (!p) {
        return NULL;
    }
        
    val = c = 0;
    len = strlen(p)*2;
    result = (char*)calloc(1,len + 1);
          
    for (j = 0;*p && j + 3 < len; p++) {
        k = (int) *p % BASE64_VALUE_SZ;
        if (base64_value[k] < 0) {
            continue;
        }
        val <<= 6;
        val += base64_value[k];
        if (++c < 4) {
            continue;
        }
        /* One quantum of four encoding characters/24 bit */
        result[j++] = val >> 16;  /* High 8 bits */
        result[j++] = (val >> 8) & 0xff;  /* Mid 8 bits */
        result[j++] = val & 0xff; /* Low 8 bits */
        val = c = 0;
    }
    result[j] = 0;
    result = (char*)realloc(result,j+1);
    return result;
}

char *encode64String(const char *str)
{
    char *result;
    int len,bits = 0,char_count = 0,out_cnt = 0,c;

    if (!str) {
        return 0;
    }

    len = strlen(str)*2;
    result = (char*)calloc(1,len + 1);

    while ((c = *str++) && out_cnt < len) {
          bits += c;
          char_count++;
          if (char_count == 3) {
              result[out_cnt++] = base64_code[bits >> 18];
              result[out_cnt++] = base64_code[(bits >> 12) & 0x3f];
              result[out_cnt++] = base64_code[(bits >> 6) & 0x3f];
              result[out_cnt++] = base64_code[bits & 0x3f];
              bits = 0;
              char_count = 0;
          } else {
              bits <<= 8;
          }
    }
    if (char_count != 0) {
        bits <<= 16 - (8 * char_count);
        result[out_cnt++] = base64_code[bits >> 18];
        result[out_cnt++] = base64_code[(bits >> 12) & 0x3f];
        if (char_count == 1) {
            result[out_cnt++] = '=';
            result[out_cnt++] = '=';
        } else {
            result[out_cnt++] = base64_code[(bits >> 6) & 0x3f];
            result[out_cnt++] = '=';
        }
    }
    result[out_cnt] = '\0';
    result = (char*)realloc(result,out_cnt+1);
    return result;
}

/*
** "*val" is a double such that 0.1 <= *val < 10.0
** Return the ascii code for the leading digit of *val, then
** multiply "*val" by 10.0 to renormalize.
**
** Example:
**     input:     *val = 3.14159
**     output:    *val = 1.4159    function return = '3'
**
** The counter *cnt is incremented each time.  After counter exceeds
** 16 (the number of significant digits in a 64-bit float) '0' is
** always returned.
*/
static int getdigit(long double *val, int *cnt)
{
    int digit;
    long double d;
    if ((*cnt)++ >= MAXDIG) {
        return '0';
    }
    digit = (int)*val;
    d = digit;
    digit += '0';
    *val = (*val - d)*10.0;
    return digit;
}

/*
**
** COPYRIGHT:
**   Copyright (c) 1990 by D. Richard Hipp.  This code is an original
**   work and has been prepared without reference to any prior
**   implementations of similar functions.  No part of this code is
**   subject to licensing restrictions of any telephone company or
**   university.
**
**   This copyright was released and the code placed in the public domain
**   by the author, D. Richard Hipp, on October 3, 1996.
**
** DESCRIPTION:
**   This program is an enhanced replacement for the "printf" programs
**   found in the standard library.  The following enhancements are
**   supported:
**           *  mprintf --  Similar to sprintf.  Writes output to memory
**                          obtained from mem_alloc.
**
**      +  A few extensions to the formatting notation are supported:
**
**           *  The "=" flag (similar to "-") causes the output to be
**              be centered in the appropriately sized field.
**
**           *  The %b field outputs an integer in binary notation.
**
**           *  The %c field now accepts a precision.  The character output
**              is repeated by the number of times the precision specifies.
**
**           *  The %' field works like %c, but takes as its character the
**              next character of the format string, instead of the next
**              argument.  For example,  printf("%.78'-")  prints 78 minus
**              signs, the same as  printf("%.78c",'-').
** The root program.  All variations call this core.
**
** INPUTS:
**   func   This is a pointer to a function taking three arguments
**            1. A pointer to the list of characters to be output
**               (Note, this list is NOT null terminated.)
**            2. An integer number of characters to be output.
**               (Note: This number might be zero.)
**            3. A pointer to anything.  Same as the "arg" parameter.
**
**   arg    This is the pointer to anything which will be passed as the
**          third argument to "func".  Use it for whatever you like.
**
**   fmt    This is the format string, as in the usual print.
**
**   ap     This is a pointer to a list of arguments.  Same as in
**          vfprint.
**
** OUTPUTS:
**          The return value is the total number of characters sent to
**          the function "func".  Returns -1 on a error.
**
** Note that the order in which automatic variables are declared below
** seems to make a big difference in determining how fast this beast
** will run.
*/
static int vxprintf(void (*func)(char*,int,void*),void *arg,const char *format,va_list ap)
{
  register const char *fmt; /* The format string. */
  register int c;           /* Next character in the format string */
  register char *bufpt;     /* Pointer to the conversion buffer */
  register int  precision;  /* Precision of the current field */
  register int  length;     /* Length of the field */
  register int  idx;        /* A general purpose loop counter */
  int count;                /* Total number of characters output */
  int width;                /* Width of the current field */
  int flag_leftjustify;     /* True if "-" flag is present */
  int flag_plussign;        /* True if "+" flag is present */
  int flag_blanksign;       /* True if " " flag is present */
  int flag_alternateform;   /* True if "#" flag is present */
  int flag_zeropad;         /* True if field width constant starts with zero */
  int flag_long;            /* True if "l" flag is present */
  int flag_center;          /* True if "=" flag is present */
  unsigned long long longvalue;  /* Value for long long types */
  long double realvalue;    /* Value for real types */
  ixprintf *infop;              /* Pointer to the appropriate info structure */
  char buf[BUFSIZE];        /* Conversion buffer */
  char prefix;              /* Prefix character.  "+" or "-" or " " or '\0'. */
  int  errorflag = 0;       /* True if an error is encountered */
  int xtype;                /* Conversion paradigm */
  char *zMem = 0;           /* String to be freed */
  struct tm tbuf;
  static char spaces[] = "                                                    ";
  int  exp;                 /* exponent of real numbers */
  long double rounder;      /* Used for rounding floating point values */
  int flag_dp;              /* True if decimal point should be shown */
  int flag_rtz;             /* True if trailing zeros should be removed */
  int flag_exp;             /* True to force display of the exponent */
  int nsd;                  /* Number of significant digits returned */

  fmt = format;                     /* Put in a register for speed */
  count = length = 0;
  bufpt = 0;
  for (; (c = (*fmt)) != 0; ++fmt) {
      if (c != '%') {
          register int amt;
          bufpt = (char *)fmt;
          amt = 1;
          while ((c = (*++fmt))!='%' && c != 0) {
              amt++;
          }
          (*func)(bufpt, amt, arg);
          count += amt;
          if (c == 0) {
              break;
          }
      }
      if ((c = (*++fmt)) == 0) {
          errorflag = 1;
          (*func)("%", 1, arg);
          count++;
          break;
      }
      /* Find out what flags are present */
      flag_leftjustify = flag_plussign = flag_blanksign =
      flag_alternateform = flag_zeropad = flag_center = 0;
      do {
         switch(c) {
          case '-': flag_leftjustify = 1; c = 0; break;
          case '+': flag_plussign = 1; c = 0; break;
          case ' ': flag_blanksign = 1; c = 0; break;
          case '#': flag_alternateform = 1; c = 0; break;
          case '0': flag_zeropad = 1; c = 0; break;
          case '=': flag_center = 1; c = 0; break;
          default: break;
        }
      } while (c == 0 && (c =(*++fmt)) != 0);
      if (flag_center) {
          flag_leftjustify = 0;
      }
      /* Get the field width */
      width = 0;
      if (c == '*') {
          width = va_arg(ap,int);
          if (width < 0) {
              flag_leftjustify = 1;
              width = -width;
          }
          c = *++fmt;
      } else {
         while (isdigit(c)) {
             width = width*10 + c - '0';
             c = *++fmt;
         }
      }
      if (width > BUFSIZE-10) {
          width = BUFSIZE-10;
      }
      /* Get the precision */
      if (c == '.') {
          precision = 0;
          c = *++fmt;
          if (c == '*') {
              precision = va_arg(ap,int);
              /* This is sensible, but SUN OS 4.1 doesn't do it. */
              if (precision < 0) {
                  precision = -precision;
              }
              c = *++fmt;
          } else {
              while (isdigit(c)) {
                  precision = precision*10 + c - '0';
                  c = *++fmt;
              }
          }
          /* Limit the precision to prevent overflowing buf[] during conversion */
          if (precision > BUFSIZE - 40) {
              precision = BUFSIZE-40;
          }
      } else {
          precision = -1;
      }
      /* Get the conversion type modifier */
      if (c == 'l') {
          flag_long = 1;
          c = *++fmt;
      } else
      if (c == 'L') {
          flag_long = 2;
          c = *++fmt;
      } else
      if (c == 'h') {
          flag_long = 3;
          c = *++fmt;
      } else {
          flag_long = 0;
      }
      /* Fetch the info entry for the field */
      infop = 0;
      for (idx = 0; fmtinfo[idx].fmttype; idx++) {
          if (c == fmtinfo[idx].fmttype) {
              infop = &fmtinfo[idx];
              break;
          }
      }
      /* No info entry found.  It must be an error. */
      if (infop == 0) {
          xtype = ERROR;
      } else {
          xtype = infop->type;
      }
    /*
    ** At this point, variables are initialized as follows:
    **
    **   flag_alternateform          TRUE if a '#' is present.
    **   flag_plussign               TRUE if a '+' is present.
    **   flag_leftjustify            TRUE if a '-' is present or if the
    **                               field width was negative.
    **   flag_zeropad                TRUE if the width began with 0.
    **   flag_long                   TRUE if the letter 'l' (ell) prefixed
    **                               the conversion character.
    **   flag_blanksign              TRUE if a ' ' is present.
    **   width                       The specified field width.  This is
    **                               always non-negative.  Zero is the default.
    **   precision                   The specified precision.  The default
    **                               is -1.
    **   xtype                       The class of the conversion.
    **   infop                       Pointer to the appropriate info struct.
    */
    switch(xtype) {
      case ORDINAL:
      case RADIX:
        if(flag_long == 1)
          longvalue = va_arg(ap,long);
	else
        if(flag_long == 2)
          longvalue = va_arg(ap,long long);
	else
        if(flag_long == 3) {
          longvalue = va_arg(ap,int);
	} else
          longvalue = va_arg(ap,int);
        /* More sensible: turn off the prefix for octal (to prevent "00"),
        ** but leave the prefix for hex. */
        if(longvalue == 0 && infop->base == 8) flag_alternateform = 0;
        if(infop->flag_signed) {
          if(*(long long*)&longvalue < 0) {
            longvalue = -*(long long*)&longvalue;
            prefix = '-';
          } else
          if(flag_plussign)
             prefix = '+';
          else
          if(flag_blanksign)
            prefix = ' ';
          else
            prefix = 0;
        } else
          prefix = 0;
        if(flag_zeropad && precision < width - (prefix != 0))
          precision = width-(prefix != 0);
        bufpt = &buf[BUFSIZE];
        if(xtype == ORDINAL) {
          long long a,b;
          a = longvalue%10;
          b = longvalue%100;
          bufpt -= 2;
          if(a == 0 || a>3 || (b>10 && b<14)) {
            bufpt[0] = 't';
            bufpt[1] = 'h';
          } else
          if(a==1) {
            bufpt[0] = 's';
            bufpt[1] = 't';
          } else
          if(a==2) {
            bufpt[0] = 'n';
            bufpt[1] = 'd';
          } else
          if(a==3) {
            bufpt[0] = 'r';
            bufpt[1] = 'd';
          }
        }
        {
          register char *cset;      /* Use registers for speed */
          register int base;
          cset = infop->charset;
          base = infop->base;
          do{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          } while(longvalue > 0);
	}
        length = (int)(&buf[BUFSIZE]-bufpt);
        for(idx = precision-length;idx > 0;idx--) *(--bufpt) = '0';  /* Zero pad */
        if(prefix) *(--bufpt) = prefix;               /* Add sign */
        if(flag_alternateform && infop->prefix) {      /* Add "0" or "0x" */
          char *pre, x;
          pre = infop->prefix;
          if(*bufpt != pre[0])
            for(pre = infop->prefix;(x = (*pre)) != 0;pre++) *(--bufpt) = x;
        }
        length = (int)(&buf[BUFSIZE]-bufpt);
        break;
      case FLOAT:
      case EXP:
      case GENERIC:
        realvalue = va_arg(ap,double);
        if(precision<0) precision = 6;         /* Set default precision */
        if(precision>BUFSIZE-10) precision = BUFSIZE-10;
        if(realvalue<0.0) {
          realvalue = -realvalue;
          prefix = '-';
	}else{
          if(flag_plussign)          prefix = '+';
          else if(flag_blanksign)    prefix = ' ';
          else                         prefix = 0;
	}
        if(infop->type==GENERIC && precision>0) precision--;
        rounder = 0.0;
        /* It makes more sense to use 0.5 */
        if(precision>MAXDIG-1) idx = MAXDIG-1;
        else                     idx = precision;
        for(rounder=0.5; idx>0; idx--, rounder*=0.1);
        if(infop->type==FLOAT) realvalue += rounder;
        /* Normalize realvalue to within 10.0 > realvalue >= 1.0 */
        exp = 0;
        if(realvalue>0.0) {
          int k = 0;
          while(realvalue>=1e8 && k++<100) { realvalue *= 1e-8; exp+=8; }
          while(realvalue>=10.0 && k++<100) { realvalue *= 0.1; exp++; }
          while(realvalue<1e-8 && k++<100) { realvalue *= 1e8; exp-=8; }
          while(realvalue<1.0 && k++<100) { realvalue *= 10.0; exp--; }
          if(k>=100) {
            bufpt = "NaN";
            length = 3;
            break;
          }
	}
        bufpt = buf;
        /*
        ** If the field type is GENERIC, then convert to either EXP
        ** or FLOAT, as appropriate.
        */
        flag_exp = xtype==EXP;
        if(xtype!=FLOAT) {
          realvalue += rounder;
          if(realvalue>=10.0) { realvalue *= 0.1; exp++; }
        }
        if(xtype==GENERIC) {
          flag_rtz = !flag_alternateform;
          if(exp<-4 || exp>precision) {
            xtype = EXP;
          }else{
            precision = precision - exp;
            xtype = FLOAT;
          }
	}else{
          flag_rtz = 0;
	}
        /*
        ** The "exp+precision" test causes output to be of type EXP if
        ** the precision is too large to fit in buf[].
        */
        nsd = 0;
        if(xtype==FLOAT && exp+precision<BUFSIZE-30) {
          flag_dp = (precision>0 || flag_alternateform);
          if(prefix) *(bufpt++) = prefix;         /* Sign */
          if(exp<0)  *(bufpt++) = '0';            /* Digits before "." */
          else for(; exp>=0; exp--) *(bufpt++) = getdigit(&realvalue,&nsd);
          if(flag_dp) *(bufpt++) = '.';           /* The decimal point */
          for(exp++; exp<0 && precision>0; precision--, exp++) {
            *(bufpt++) = '0';
          }
          while((precision--)>0) *(bufpt++) = getdigit(&realvalue,&nsd);
          *(bufpt--) = 0;                           /* Null terminate */
          if(flag_rtz && flag_dp) {     /* Remove trailing zeros and "." */
            while(bufpt>=buf && *bufpt=='0') *(bufpt--) = 0;
            if(bufpt>=buf && *bufpt=='.') *(bufpt--) = 0;
          }
          bufpt++;                            /* point to next free slot */
	}else{    /* EXP or GENERIC */
          flag_dp = (precision>0 || flag_alternateform);
          if(prefix) *(bufpt++) = prefix;   /* Sign */
          *(bufpt++) = getdigit(&realvalue,&nsd);  /* First digit */
          if(flag_dp) *(bufpt++) = '.';     /* Decimal point */
          while((precision--)>0) *(bufpt++) = getdigit(&realvalue,&nsd);
          bufpt--;                            /* point to last digit */
          if(flag_rtz && flag_dp) {          /* Remove tail zeros */
            while(bufpt>=buf && *bufpt=='0') *(bufpt--) = 0;
            if(bufpt>=buf && *bufpt=='.') *(bufpt--) = 0;
          }
          bufpt++;                            /* point to next free slot */
          if(exp || flag_exp) {
            *(bufpt++) = infop->charset[0];
            if(exp<0) { *(bufpt++) = '-'; exp = -exp; } /* sign of exp */
            else       { *(bufpt++) = '+'; }
            if(exp>=100) {
              *(bufpt++) = (exp/100)+'0';                /* 100's digit */
              exp %= 100;
  	    }
            *(bufpt++) = exp/10+'0';                     /* 10's digit */
            *(bufpt++) = exp%10+'0';                     /* 1's digit */
          }
	}
        /* The converted number is in buf[] and zero terminated. Output it.
        ** Note that the number is in the usual order, not reversed as with
        ** integer conversions. */
        length = (int)(bufpt-buf);
        bufpt = buf;

        /* Special case:  Add leading zeros if the flag_zeropad flag is
        ** set and we are not left justified */
        if(flag_zeropad && !flag_leftjustify && length < width) {
          int i;
          int nPad = width - length;
          for(i=width; i>=nPad; i--) {
            bufpt[i] = bufpt[i-nPad];
          }
          i = prefix!=0;
          while(nPad--) bufpt[i++] = '0';
          length = width;
        }
        break;
      case SIZE:
        *(va_arg(ap,int*)) = count;
        length = width = 0;
        break;
      case PERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        break;
      case CHARLIT:
      case CHAR:
        c = buf[0] = (xtype==CHAR ? va_arg(ap,int) : *++fmt);
        if(precision>=0) {
          for(idx=1; idx<precision; idx++) buf[idx] = c;
          length = precision;
	}else{
          length =1;
	}
        bufpt = buf;
        break;
      case LOCALTIME:
          longvalue = va_arg(ap,time_t);
          localtime_r((const time_t*)&longvalue, &tbuf);
          strftime(buf, 64, "%a, %d %b %Y %H:%M:%S %Z", &tbuf);
          length = strlen(buf);
          bufpt = buf;
          break;
      case GMTTIME:
          longvalue = va_arg(ap,time_t);
          gmtime_r((const time_t*)&longvalue, &tbuf);
          strftime(buf, 64, RFC1123_STRFTIME,&tbuf);
          length = strlen(bufpt);
          bufpt = buf;
          break;
      case RFC850TIME:
          longvalue = va_arg(ap,time_t);
          gmtime_r((const time_t*)&longvalue, &tbuf);
          strftime(buf,64,RFC850_STRFTIME, &tbuf);
          length = strlen(bufpt);
          bufpt = buf;
          break;
      case IPADDR: {
          longvalue = va_arg(ap,u_long);
          zMem = bufpt = (char*)malloc(16);
          sock_inet_ntoa(longvalue,bufpt);
          length = strlen(bufpt);
          break;
      }
      case STRING:
      case MEM_STRING:
      case ESCAPE:
      case HEX:
      case BASE64:
        if((zMem = bufpt = va_arg(ap,char*)))
          switch(xtype) {
           case STRING:
           case MEM_STRING:
               break;
           case ESCAPE:
               zMem = bufpt = (char*)urlEncodeString(bufpt);
               break;
           case HEX:
               zMem = bufpt = (char*)encodeHexString(bufpt,strlen(bufpt));
               break;
           case BASE64:
               zMem = bufpt = (char*)encode64String(bufpt);
               break;
          }
        else
          bufpt = "";
        length = strlen(bufpt);
        if(precision >= 0 && precision < length) length = precision;
        break;
      case SEEIT:
        {
          int i;
          int c;
          char *arg = va_arg(ap,char*);
          for(i=0; i<BUFSIZE-1 && (c = *arg++)!=0; i++) {
            if(c<0x20 || c>=0x7f) {
              buf[i++] = '^';
              buf[i] = (c&0x1f)+0x40;
            }else{
              buf[i] = c;
            }
          }
          bufpt = buf;
          length = i;
          if(precision>=0 && precision<length) length = precision;
        }
        break;
      case ERROR:
        buf[0] = '%';
        buf[1] = c;
        errorflag = 0;
        idx = 1+(c!=0);
        (*func)("%",idx,arg);
        count += idx;
        if(c==0) fmt--;
        break;
    }
    /*
    ** The text of the conversion is pointed to by "bufpt" and is
    ** "length" characters long.  The field width is "width".  Do
    ** the output.
    */
    if(!flag_leftjustify) {
      register int nspace;
      nspace = width-length;
      if(nspace > 0) {
        if(flag_center) {
          nspace = nspace/2;
          width -= nspace;
          flag_leftjustify = 1;
	}
        count += nspace;
        while(nspace>=(sizeof(spaces)-1)) {
          (*func)(spaces,(sizeof(spaces)-1),arg);
          nspace -= (sizeof(spaces)-1);
        }
        if(nspace > 0) (*func)(spaces,nspace,arg);
      }
    }
    if(length > 0) {
      (*func)(bufpt,length,arg);
      count += length;
    }
    switch(xtype) {
     case MEM_STRING:
     case ESCAPE:
     case HEX:
     case BASE64:
     case IPADDR:
         lmbox_free(zMem);
    }
    if(flag_leftjustify) {
      register int nspace;
      nspace = width-length;
      if(nspace > 0) {
        count += nspace;
        while(nspace>=(sizeof(spaces)-1)) {
          (*func)(spaces,(sizeof(spaces)-1),arg);
          nspace -= (sizeof(spaces)-1);
        }
        if(nspace > 0) (*func)(spaces,nspace,arg);
      }
    }
  }
  return errorflag ? -1 : count;
} 

static void vxprintf_out(char *str, int size, void *arg)
{
    mxprintf *m = (mxprintf*)arg;

    if (m->size + size + 1 > m->alloc) {
        m->alloc = m->size + size*2 + 1;
        if (m->text == m->base) {
            m->text = (char*)malloc(m->alloc);
            if (m->text && m->size) {
                memcpy(m->text, m->base, m->size);
            }
        } else {
            m->text = (char*)realloc(m->text, m->alloc);
        }
    }
    if (m->text) {
        memcpy(&m->text[m->size], str, size);
        m->size += size;
        m->text[m->size] = 0;
    }
}

char *lmbox_printf(const char *zFormat, ...)
{
    va_list ap;
    mxprintf m;
    char *str;
    char buf[200];

    va_start(ap,zFormat);
    m.size = 0;
    m.alloc = sizeof(buf);
    m.text = buf;
    m.base = buf;
    vxprintf(vxprintf_out, &m, zFormat, ap);
    va_end(ap);
    if (m.text == m.base) {
        m.text = strdup(buf);
    }
    return m.text;
}

char *lmbox_vprintf(const char *zFormat, va_list ap)
{
    mxprintf m;
    char buf[200];

    m.size = 0;
    m.text = buf;
    m.alloc = sizeof(buf);
    m.base = buf;
    vxprintf(vxprintf_out, &m, zFormat, ap);
    if (m.text == m.base) {
        m.text = strdup(buf);
    }
    return m.text;
}

void lmbox_free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}

void lmbox_destroy(void **ptr)
{
    lmbox_free(*ptr);
    *ptr = 0;
}

char *lmbox_strdup(const char *str)
{
    if (str) {
        return strdup(str);
    }
    return 0;
}
