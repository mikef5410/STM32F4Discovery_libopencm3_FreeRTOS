
#ifndef _DEBUG_SHELL_INCLUDED
#define _DEBUG_SHELL_INCLUDED

#ifdef GLOBAL_DEBUG_SHELL
#define DEBUG_SHELLGLOBAL
#define DEBUG_SHELLPRESET(A) = (A)
#else
#define DEBUG_SHELLPRESET(A)
#ifdef __cplusplus
#define DEBUG_SHELLGLOBAL extern "C"
#else
#define DEBUG_SHELLGLOBAL extern
#endif
#endif

#define DEBUG_SHELL_MAXARGS 32
#define COMMAND_SIZE 127

#ifndef NULL
#define NULL (void *)0
#endif

#include "OSandPlatform.h"

typedef struct dent {
    const char *context;          //command context or namespace
    const char *cmd;              //command name
    const char *shortHelp;        //one line help 
    int (*cmdp)(int argc, char** argp);    //how to execute the command (return 0 for success)
    void (*helpfunc)(void);           //prints a longer help
} dispatchEntry;

DEBUG_SHELLGLOBAL char currentContext[32];
DEBUG_SHELLGLOBAL int  cmdErr DEBUG_SHELLPRESET(0);
DEBUG_SHELLGLOBAL uint32_t cmd_term_get (char *buf, uint32_t maxlen); /* Get character(s) from a terminal based cmd. */
DEBUG_SHELLGLOBAL void cmdParse(char *); /* Parse a term/exteral command */
DEBUG_SHELLGLOBAL portTASK_FUNCTION(vDebugShell, pvParameters);



extern dispatchEntry* (dispatchTableTable)[]; //Always implemented per-instrument in command.c

#ifndef GLOBAL_DEBUG_SHELL
extern dispatchEntry commonCommands[];
#endif

#endif //_DEBUG_SHELL_INCLUDED
