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

class TemplateProperty {
public:
    TemplateProperty(const char *name, const char *value);
    ~TemplateProperty();

    char *name;
    char *value;
    struct TemplateProperty *next;
};

class Template {
public:
    Template(const char *name, const char *type);
    ~Template();

    char *name;
    char *type;
    int origin;
    TemplateProperty *events;
    TemplateProperty *properties;
};

typedef int xmlParseProc(XML_Parser *doc, XML_Node *node, void *arg);

static vector < Template * >templates;
static pthread_mutex_t templatemutex = PTHREAD_MUTEX_INITIALIZER;

static int parseXmlFile(const char *filename, xmlParseProc *xmlproc, void* arg);
static int parseRoot(XML_Parser *doc, XML_Node *node, void *arg);
static int parsePages(XML_Parser *doc, XML_Node *node);
static int parseSystem(XML_Parser *doc, XML_Node *node);
static int parseInclude(XML_Parser *doc, XML_Node *node);
static int parseObject(XML_Parser *doc, XML_Node *node);
static int parsePage(XML_Parser *doc, XML_Node *node, void *arg);
static int parsePageInclude(XML_Parser *doc, XML_Node *node, GUI_Page *page);
static int parseTimer(XML_Parser *doc, XML_Node *node, DynamicObject * parent);
static int parseEvent(XML_Parser *doc, XML_Node *node, DynamicObject * parent);
static int parseProperties(XML_Parser *doc, XML_Node *node, DynamicObject *object);
static int parseTemplate(XML_Parser *doc, XML_Node *node);
static GUI_Widget *parseWidget(XML_Parser *doc, XML_Node *node);
static Template *findTemplate(const char *templatename);
static void freeTemplates(void);

int parseFile(const char *filename)
{
    return parseXmlFile(filename, parseRoot, 0);
}

int parsePath(const char *path)
{
    int rc = 0;
    string file;
    struct dirent *entry, **files = 0;

    int count = scandir(path, &files, NULL, (int (*)(const void *, const void *)) sortcmp);
    for (int i = 0; !rc && i < count; ++i) {
        if (files[i]->d_name[0] == '.' || matchString(files[i]->d_name, "*.xml")) {
            lmbox_free(files[i]);
            continue;
        }
        file = path;
        file += '/';
        file += files[i]->d_name;
        rc = parseFile(file.c_str());
        lmbox_free(files[i]);
    }
    lmbox_free(files);
    return rc;
}

void parseFree(void)
{
    freeTemplates();
}

static int parseXmlFile(const char *filename, xmlParseProc *xmlproc, void *arg)
{
    int rc = 0;

    systemObject->Log(2, "parseXmlFile: %s", filename);

    char *file = systemObject->FindFile(filename);
    if (!file) {
        systemObject->Log(0, "parseXmlFile: Failed to load XML file %s", filename);
        return -1;
    }
    XML_Parser *doc = new XML_Parser(file);
    free(file);
    for (XML_Node *node = doc->GetRoot(); node; node = node->next) {
        if (!strcmp(node->name, "lmbox")) {
            rc = xmlproc(doc, node, arg);
        }
    }
    delete doc;
    return rc;
}

static int parseRoot(XML_Parser *doc, XML_Node *node, void *arg)
{
    int rc = 0;

    for (node = node->child; node && !rc; node = node->next) {
        if (!strcmp(node->name, "include")) {
            rc = parseInclude(doc, node);
        } else
        if (!strcmp(node->name, "template")) {
            rc = parseTemplate(doc, node);
        } else
        if (!strcmp(node->name, "page")) {
            rc = parsePage(doc, node, 0);
        } else
        if (!strcmp(node->name, "system")) {
            rc = parseSystem(doc, node);
        } else
        if (!strcmp(node->name, "object")) {
            rc = parseObject(doc, node);
        } else {
            systemObject->Log(0, "parseRoot: unsupported object %s", node->name);
        }
    }
    return rc;
}

static int parseInclude(XML_Parser *doc, XML_Node *node)
{
    int rc = 0;

    systemObject->Log(6, "parseInclude: %s:", node->name);

    char *file = node->GetProperty("file");
    if (file) {
        rc = parseFile(file);
    }
    return rc;
}

static int parseSystem(XML_Parser *doc, XML_Node *node)
{
    char *str;
    int rc = 0;

    systemObject->Log(6, "parseSystem: %s:", node->name);

    if ((str = node->GetProperty("xscale"))) {
        systemObject->SetXScale(atof(str));
    }
    if ((str = node->GetProperty("yscale"))) {
        systemObject->SetYScale(atof(str));
    }
    for (node = node->child; node && !rc; node = node->next) {
        if (!strcmp(node->name, "event")) {
            rc = parseEvent(doc, node, systemObject);
        } else
        if (!strcmp(node->name, "timer")) {
            rc = parseTimer(doc, node, systemObject);
        } else {
            systemObject->Log(0, "parseSystem: %s: unsupported object %s", node->name, doc->GetFile());
        }
    }
    return 0;
}

static int parseObject(XML_Parser *doc, XML_Node *node)
{
    int rc = 0;
    XML_Node *cur;
    DynamicObject *obj = 0;

    systemObject->Log(6, "parseObject: %s:", node->name);

    char *name = node->GetProperty("name");
    if (!name) {
        systemObject->Log(0, "parseObject: %s: name should be specified for <object> tag", doc->GetFile());
        return 1;
    }
    char *ptr = name;
    while (ptr && !rc) {
        char *end = strchr(ptr, ',');
        if (end) {
            *end++ = 0;
        }
        obj = systemObject->FindObject(ptr);
        if (obj) {
            cur = node->child;
            while (cur != NULL) {
                if (!strcmp(cur->name, "event")) {
                    parseEvent(doc, cur, obj);
                }
                cur = cur->next;
            }
            rc = parseProperties(doc, node, obj);
            obj->DecRef();
        }
        ptr = end;
    }
    return rc;
}

static int parsePage(XML_Parser *doc, XML_Node *node, void *arg)
{
    XML_Node *cur;
    char *str, *name;
    SDL_Color bgcolor;
    GUI_Container *page = (GUI_Container*)arg;

    systemObject->Log(6, "parsePage: %s:", node->name);

    if (!page) {
        name = node->GetProperty("name");
        if (!name) {
            systemObject->Log(0, "parsePage: %s: name should be specified for <page> tag", doc->GetFile());
            return -1;
        }
        page = new GUI_Page(name, 0, 0, systemObject->GetScreenWidth(), systemObject->GetScreenHeight());
    }
    for (cur = node->child; cur; cur = cur->next) {
        if (!strcmp(cur->name, "help")) {
            if (cur->data) {
                page->SetHelp(cur->data);
            }
        } else
        if (!strcmp(cur->name, "include")) {
            str = cur->GetProperty("file");
            if (str) {
                parseXmlFile(str, parsePage, (void*)page);
            }
        } else
        if (!strcmp(cur->name, "timer")) {
            parseTimer(doc, cur, page);
        } else
        if (!strcmp(cur->name, "event")) {
            parseEvent(doc, cur, page);
        } else {
            GUI_Widget *widget = parseWidget(doc, cur);
            if (!widget) {
                delete page;
                return -1;
            }
            if (widget->Initialize()) {
                page->AddWidget(widget);
            }
            widget->DecRef();
        }
    }
    // Parse properties/templates
    if (parseProperties(doc, node, page)) {
        delete page;
        return -1;
    }
    if (page->Initialize()) {
        systemObject->AddPage(page);
    }
    page->DecRef();
    return 0;
}

static GUI_Widget *parseWidget(XML_Parser *doc, XML_Node *node)
{
    char *name, nbuf[256];
    GUI_Widget *widget = 0;
    static int count = 0;

    name = node->GetProperty("name");
    if (!name) {
        snprintf(nbuf, sizeof(nbuf), "_%s%d", node->name, ++count);
        name = nbuf;
    }
    if (!strcmp(node->name, "line") ||
        !strcmp(node->name, "hline") ||
        !strcmp(node->name, "vline") ||
        !strcmp(node->name, "box") ||
        !strcmp(node->name, "rectangle")) {
        widget = new GUI_GFX(name, 0, 0, 0, 0, node->name);
    } else
    if (!strcmp(node->name, "button")) {
        widget = new GUI_Button(name, 0, 0, 0, 0);
    } else
    if (!strcmp(node->name, "label")) {
        widget = new GUI_Label(name, 0, 0, 0, 0, NULL, 0);
    } else
    if (!strcmp(node->name, "clocklabel")) {
        widget = new GUI_ClockLabel(name, 0, 0, 0, 0, NULL, 0);
    } else
    if (!strcmp(node->name, "picture")) {
        widget = new GUI_Picture(name, 0, 0, 0, 0, NULL);
    } else
    if (!strcmp(node->name, "listbox")) {
        widget = new GUI_ListBox(name, 0, 0, 0, 0, NULL);
    } else
    if (!strcmp(node->name, "filebrowser")) {
        widget = new GUI_FileBrowser(name, 0, 0, 0, 0, NULL);
    } else
    if (!strcmp(node->name, "togglebutton")) {
        widget = new GUI_ToggleButton(name, 0, 0, 0, 0);
    } else
    if (!strcmp(node->name, "scrollbar")) {
        widget = new GUI_ScrollBar(name, 0, 0, 0, 0);
    } else
    if (!strcmp(node->name, "textfield")) {
        widget = new GUI_TextField(name, 0, 0, 0, 0, NULL, 255);
    }
    if (!widget) {
        systemObject->Log(0, "parseWidget: %s: unsupported widget type %s", doc->GetFile(), node->name);
        return NULL;
    }
    // Parse properties/templates
    if (parseProperties(doc, node, widget)) {
        delete widget;
        return NULL;
    }
    // Parse widget events
    for (node = node->child; node; node = node->next) {
        if (!strcmp(node->name, "event")) {
            parseEvent(doc, node, widget);
        } else {
           systemObject->Log(0, "parseWidget: %s: unsupported object %s", doc->GetFile(), node->name);
           delete widget;
           return NULL;
        }
    }
    return widget;
}

static int parseProperties(XML_Parser *doc, XML_Node *node, DynamicObject *object)
{
    bool propset;
    Variant value;
    ObjectProperty *prop;
    Template *tmpl = NULL;
    TemplateProperty * attr;

    systemObject->Log(6, "parseProperties: %s: count=%d", node->name, object->GetPropertyCount());

    // Grab template
    char *str = node->GetProperty("template");
    if (str) {
        tmpl = findTemplate(str);
        if (!tmpl) {
            systemObject->Log(0, "parseProperties: Non-existent template %s specified", str);
            return 1;
        }
    }

    // Go through all properties
    for (int i = 0; i < object->GetPropertyCount(); i++) {
        prop = object->GetProperty(i);
        if (prop->IsReadOnly()) {
            continue;
        }
        propset = false;
        str = node->GetProperty(prop->GetName());

        systemObject->Log(6, "parseProperties: %s: name %s", node->name, prop->GetName());

        if (str) {
            value = str;
            int rc = prop->SetValue(value);
            if (rc != 0) {
                systemObject->Log(0, "parseProperties: Unable to set property value %s to %s", prop->GetName(), object->GetName());
                return 1;
            }
            propset = true;
        } else {
            // Check for template value
            if (tmpl) {
                for (attr = tmpl->properties; attr; attr = attr->next) {
                    if (strcmp(attr->name, prop->GetName())) {
                        continue;
                    }
                    value = attr->value;
                    int rc = prop->SetValue(value);
                    if (rc != 0) {
                        systemObject->Log(0, "Unable to set property value %s to %s", prop->GetName(), object->GetName());
                        return 1;
                    }
                    propset = true;
                }
            }
        }
        if (!propset && prop->IsRequired()) {
            systemObject->Log(0, "Required property %s of object %s has not been specified", prop->GetName(), object->GetName());
            return 1;
        }
    }

    // Add events and actions handlers
    if (tmpl) {
        for (attr = tmpl->events; attr; attr = attr->next) {
             object->AddEventHandler(attr->name, attr->value);
        }
    }
    // Grab default event
    if (object->GetDefaultEvent()) {
        if (!isEmptyString(node->data)) {
            object->AddEventHandler(object->GetDefaultEvent(), node->data);
        }
    } else
    if (object->GetDefaultProperty()) {
        if (!isEmptyString(node->data)) {
            object->SetPropertyValue(object->GetDefaultProperty(), anytovariant(node->data));
        }
    }

    return 0;
}

static int parseEvent(XML_Parser *doc, XML_Node *node, DynamicObject * parent)
{
    char *name;

    systemObject->Log(6, "parseEvent: %s:", node->name);

    if (!isEmptyString(node->data)) {
        if (name = node->GetProperty("name")) {
            parent->AddEventHandler(name, node->data);
        } else {
            systemObject->Log(0, "parseEvent: %s: no name in event: %s", doc->GetFile(), node->data);
        }
    }
    return 0;
}

static int parseTimer(XML_Parser *doc, XML_Node *node, DynamicObject *parent)
{
    char *str;
    char *name, nbuf[256];
    int interval = 1000;
    bool autostart = false;
    TimerObject *timer;
    static int count = 0;

    systemObject->Log(6, "parseTimer: %s:", node->name);

    // Grab name if specified, otherwise make one up (useful for debugging)
    name = node->GetProperty("name");
    if (name == NULL) {
        snprintf(nbuf, sizeof(nbuf), "_timer%d", ++count);
        name = nbuf;
    }
    str = node->GetProperty("interval");
    if (str) {
        interval = atoi(str);
    }
    str = node->GetProperty("enabled");
    if (str) {
        autostart = strToBool(str);
    }
    // Create the timer
    timer = new TimerObject(name, interval, autostart);
    // Grab attached event handlers
    if (node->data && strlen(node->data) > 0) {
        Script *script = new Script(name, node->data);
        EventHandler *handler = new EventHandler("OnTimer", script);
        timer->RegisterEventHandler(handler);
        handler->DecRef();
    }
    parent->AddObject(timer);
    return 0;
}

static int parseTemplate(XML_Parser *doc, XML_Node *node)
{
    char *str;
    Template *tmpl;
    TemplateProperty *attr;
    XML_Property *prop;

    systemObject->Log(6, "parseTemplate: %s:", node->name);

    str = node->GetProperty("name");
    if (str == NULL) {
        systemObject->Log(0, "Required property name should be specified for template");
        return 1;
    }
    tmpl = new Template(str, node->name);
    // Parse properties
    for (prop = node->properties; prop; prop = prop->next) {
        if (strcmp(prop->name, "name")) {
            attr = new TemplateProperty(prop->name, prop->value);
            attr->next = tmpl->properties;
            tmpl->properties = attr;
        }
    }
    // Parse events and actions
    for (node = node->child; node; node = node->next) {
        if (!strcmp(node->name, "event") &&
            !isEmptyString(node->data) &&
            (str = node->GetProperty("name"))) {
            attr = new TemplateProperty(str, node->data);
            attr->next = tmpl->events;
            tmpl->events = attr;
        }
    }
    templates.push_back(tmpl);
    return 0;
}

Template *findTemplate(const char *name)
{
    for (unsigned int i = 0; i < templates.size(); i++) {
        if (!strcmp(templates[i]->name, name)) {
            return templates[i];
        }
    }
    return NULL;
}

void freeTemplates(void)
{
    while (templates.size() > 0) {
        delete templates.back();
        templates.pop_back();
    }
}

Template::Template(const char *name, const char *type)
{
    this->name = lmbox_strdup(name);
    this->type = lmbox_strdup(type);
    this->events = 0;
    this->properties = 0;
}

Template::~Template()
{
    lmbox_free(name);
    lmbox_free(type);
    delete events;
    delete properties;
}

TemplateProperty::TemplateProperty(const char *name, const char *value)
{
    this->next = 0;
    this->name = lmbox_strdup(name);
    this->value = lmbox_strdup(value);
}

TemplateProperty::~TemplateProperty()
{
    if (next) {
        delete next;
    }
    lmbox_free(name);
    lmbox_free(value);
}

