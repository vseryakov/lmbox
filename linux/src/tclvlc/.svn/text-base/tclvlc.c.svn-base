#include <tcl.h>
#include <vlc/vlc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int VLCCmd(ClientData arg, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    char **argv = 0;
    int i, cmd, vlc;
    vlc_value_t varg;

    static CONST char *opts[] = {
        "create", "add", "play", "stop", "pause", "cleanup", "destroy",
        0
    };
    enum {
        cmdCreate, cmdAdd, cmdPlay, cmdStop, cmdPause, cmdCleanup, cmdDestroy
    };

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], opts, "option", 0, (int *) &cmd) != TCL_OK) {
        return TCL_ERROR;
    }

    if (cmd != cmdCreate) {
        if (objc < 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "vlc_id ?arg?");
            return TCL_ERROR;
        }
        vlc = atoi(Tcl_GetString(objv[2]));
    }
    switch (cmd) {
    case cmdCreate:
         vlc = VLC_Create();
         if (vlc > -1) {
             if (objc - 2 > 0) {
                 argv = (char**)calloc(sizeof(char*), objc);
                 argv[0] = strdup(Tcl_GetString(objv[0]));
                 for (i = 2; i < objc; i++) {
                      argv[i - 1] = strdup(Tcl_GetString(objv[i]));
                 }
             }
             i = VLC_Init(vlc, argv ? objc - 1 : 0, argv);
             if (i < 0) {
                 if (argv) {
                     for (i = 0; argv[i]; i++) {
                         free(argv[i]);
                     }
                     free(argv);
                 }
                 Tcl_AppendResult(interp, VLC_Error(i), 0);
                 VLC_Destroy(vlc);
                 return TCL_ERROR;
             }
             if (argv) {
                 varg.p_address = argv;
                 VLC_VariableSet(vlc, "tcl.argv", varg);
             }
             Tcl_SetObjResult(interp, Tcl_NewIntObj(vlc));
             break;
         }
         Tcl_AppendResult(interp, VLC_Error(vlc), 0);
         return TCL_ERROR;

    case cmdAdd:
         for (i = 3; i < objc; i++) {
             VLC_AddTarget(vlc, Tcl_GetString(objv[i]), 0, 0, PLAYLIST_APPEND, PLAYLIST_END);
         }
         break;

    case cmdPlay:
         Tcl_SetObjResult(interp, Tcl_NewIntObj(VLC_Play(vlc)));
         break;

    case cmdStop:
         Tcl_SetObjResult(interp, Tcl_NewIntObj(VLC_Stop(vlc)));
         break;

    case cmdPause:
         Tcl_SetObjResult(interp, Tcl_NewIntObj(VLC_Pause(vlc)));
         break;

    case cmdCleanup:
         Tcl_SetObjResult(interp, Tcl_NewIntObj(VLC_CleanUp(vlc)));
         break;

    case cmdDestroy:
         if (VLC_VariableGet(vlc, "tcl.argv", &varg) == VLC_SUCCESS) {
             argv = varg.p_address;
             for (i = 0; argv[i]; i++) {
                 free(argv[i]);
             }
             free(argv);
         }
         VLC_CleanUp(vlc);
         Tcl_SetObjResult(interp, Tcl_NewIntObj(VLC_Destroy(vlc)));
         break;
    }
    return TCL_OK;
}

int Tclvlc_Init(Tcl_Interp *interp)
{
   static int initialized = 0;

   if(initialized) return 0;
   initialized = 1;
   Tcl_CreateObjCommand(interp, "vlc", VLCCmd, 0, 0);
   return 0;
}

