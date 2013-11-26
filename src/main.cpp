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

// Global variables
GUI_Page *page;
Script *script;
SystemObject *systemObject;

#ifdef HAVE_MOZILLA

// This function is used for taking automatic screenshots from various
// Web pages, including flash games
static gboolean webbrowser_snapshot(gpointer data)
{
    WebBrowserObject *browser = (WebBrowserObject*)data;
    browser->BrowserSnapshot(0);
    browser->BrowserClose();
    exit(0);
}

static void webbrowser(int argc, char **argv, const char *url)
{
    gtk_init(&argc, &argv);
    WebBrowserObject *browser = new WebBrowserObject("webbrowser");
    browser->SetLocation(url);
    browser->BrowserOpen();
    // Snapshot support
    int ss = systemObject->configGetInt("webss", 0);
    if (ss > 0) {
        g_timeout_add(ss*1000, webbrowser_snapshot, browser);
    }
    gtk_main();
    exit(0);
}
#endif

static void usage()
{
    cout << "lmbox " << LMBOX_VERSION << endl << endl;
    cout << "  Syntax: lmbox [options] [file] " << endl;
    cout << "  Options:" << endl;
    cout << "    -r rootdir    Change dir to rootdir on start" << endl;
    cout << "    -d depth      Screen depth" << endl;
    cout << "    -b bgname     Initial background image" << endl;
    cout << "    -i initfile   Startup Tcl file" << endl;
    cout << "    -c initproc   Run Tcl command on startup" << endl;
    cout << "    -l tcldir     Load all Tcl files from specified directory" << endl;
    cout << "    -x xmldir     Load all xml files from specified directory" << endl;
    cout << "    -p pagename   Initial page" << endl;
    cout << "    -w width      Set width of display" << endl;
    cout << "    -h height     Set height of display" << endl;
    cout << "    -f            Use full-screen display" << endl;
    cout << "    -m            Hide the mouse cursor" << endl;
    cout << "    -j            Enable joystick support" << endl;
    cout << "    -g            Disable GUI, run initproc and exit" << endl;
    cout << "    -l            Disable LIRC support" << endl;
    cout << "    -k            Dump all keyboard names" << endl;
    cout << "    -v            Print this help screen" << endl;
#ifdef HAVE_MOZILLA
    cout << "    -ss secs      Take window snapshot in specified interval and exit" << endl;
    cout << "    -web url      Run embedded web browser with given url" << endl;
#endif
    cout << endl;
    cout << "  If you do not specify xmlfile, lmbox will look for lmbox.xml by default" << endl;
#ifdef HAVE_MOZILLA
    cout << "  If lmbox is runing as lmboxweb url, it is similar to invoking lmbox -web url" << endl;
#endif
}

// Returns last argument which should be xml file name
static char *parse_args(int stage, int argc, char **argv)
{
    int arg;
    static char *configmap[] = {
        "-r", "rootdir", "1",
        "-w", "width", "1",
        "-h", "height", "1",
        "-xs", "xscale", "1",
        "-ys", "yscale", "1",
        "-i", "initfile", "1",
        "-l", "tcldir", "1",
        "-x", "xmldir", "1",
        "-c", "initproc", "1",
        "-p", "pagename", "1",
        "-b", "bgimage", "1",
        "-d", "depth", "1",
        "-f", "fullscreen", 0,
        "-m", "nocursor", 0,
        "-j", "nojoystick", "0",
        "-l", "nolirc", 0,
        "-g", "nogui", 0,
        "-ss", "webss", "1",
        "-web", "weburl", "1",
        0, 0, 0
    };

    for (arg = 1; arg < argc; arg++) {
        if (!strcmp(argv[arg], "-v")) {
            usage();
        } else
        if (!strcmp(argv[arg], "-k")) {
            printKeyNames();
            exit(0);
        } else
        if (!strcmp(argv[arg], "-xmldebug") ||
            !strcmp(argv[arg], "-xmldump") ||
            !strcmp(argv[arg], "-xmlload")) {
            if (argv[arg + 1]) {
                if (!strcmp(argv[arg], "-xmldebug")) {
                    systemObject->configSetValue("xml_debug", "1");
                }
                XML_Parser xml(argv[arg + 1]);
                if (!strcmp(argv[arg], "-xmldump")) {
                   xml.PrintNode(xml.GetRoot(), 0);
                }
            }
            exit(0);
        }
        bool flag = false;
        char *value = "1";
        for (int i = 0; configmap[i]; i += 3) {
            if (!strcmp(argv[arg], configmap[i])) {
                if (configmap[i + 2]) {
                    if (!argv[arg + 1]) {
                        usage();
                        exit(1);
                    }
                    value = argv[++arg];
                }
#ifdef HAVE_MOZILLA
                if (!strcmp(configmap[i], "-web")) {
                    webbrowser(argc, argv, value);
                } else
#endif
                systemObject->configSetValue(configmap[i + 1], value);
                flag = true;
                break;
            }
        }
        if (!flag)
            break;
    }
    char *p = strrchr(argv[0], '/');
    if (p) p++; else p = argv[0];
    systemObject->SetAppName(p);
#ifdef HAVE_MOZILLA
    if (!strcmp(p, "lmboxweb")) {
       webbrowser(argc, argv, argv[arg]);
    }
#endif
    if (arg >= argc) {
        return 0;
    }
    return argv[arg];
}

static void sigint(int sig)
{
    systemObject->Quit();
}

int main(int argc, char **argv)
{
    tlsInit();
    scriptInit();
    systemObject = new SystemObject("system");
    script = new Script("tcl", 0);
    // Parse arguments before we load first Tcl interp to
    // get initfile if specified in the command line
    parse_args(0, argc, argv);
    // Change working directory if specified
    systemObject->Chdir(systemObject->configGetValue("rootdir", "./"));
    // Initialize database early
    if (systemObject->dbInit()) {
        Tcl_Exit(1);
    }
    signal(SIGINT, sigint);
    // Read config file
    script->Execute("source [file normalize ~/.lmbox/config]");
    // Run specified Tcl command on start
    script->Execute(systemObject->configGetValue("initproc", "lmbox_init"));
    // Last argument is xml file name
    const char *xmldir = NULL;
    const char *xmlfile = parse_args(1, argc, argv);
    if (xmlfile) {
        systemObject->configSetValue("xmlfile", xmlfile);
    }

    xmlfile = systemObject->configGetValue("xmlfile", "lmbox.xml");
    if (access(xmlfile, R_OK)) {
        goto done;
    }

    // Initialize systemObject and other subsystems
    if (!systemObject->Init()) {
        goto done;
    }

    // Parse skin file
    if (parseFile(xmlfile)) {
        goto done;
    }

    // Load all xml files from configured directory
    xmldir = systemObject->configGetValue("xmldir", "./modules");
    if (*xmldir && parsePath(xmldir)) {
        goto done;
    }

    parseFree();
    if (!systemObject->GetPageCount()) {
        systemObject->Log(0, "XML file %s does not contain any pages", xmlfile);
        goto done;
    }
    // Focus initial page
    systemObject->StartObjects();
    systemObject->FireEvent("OnLoad");
    if (systemObject->SwitchPage(systemObject->configGetValue("pagename", 0))) {
        goto done;
    }

    // Start event processing
    systemObject->Run();
    // Unfocus current page
    page = systemObject->GetCurrentPage();
    if (page) {
        page->OnLostFocus();
        page->DecRef();
    }
    systemObject->FireEvent("OnUnload");

done:
    // Run Tcl proc on exit
    script->Execute(systemObject->configGetValue("closeproc", "lmbox_close"));
    delete script;
    systemObject->Shutdown();
    scriptShutdown();
    delete systemObject;
    tlsShutdown();
    Tcl_Exit(0);
}
