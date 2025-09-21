#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lib.h"
#include "sdl_lib.h"

#define SCRIPTS_ERR_LEN     1024

#define SCRIPTS_F_CONCUR    0x01
#define SCRIPTS_F_LUALIBS   0x02
#define SCRIPTS_F_EXTLIBS   0x04
#define SCRIPTS_F_SDLLIBS   0x08

typedef struct __scriptenv {
    char                    **script;
    int                     size;
    lua_State               *state;
    unsigned char           flags;
} SCRIPTENV;

typedef struct __scripts {
    SCRIPTENV               **scriptenv;
    char                    **id;
    int                     size;
    int                     scripts;
    unsigned char           flags;
} SCRIPTS;

int         scriptenv_find  (const SCRIPTS *, const char *);
char        *scriptenv_new  (SCRIPTS *, const char *, unsigned char);
char        *scriptenv_add  (SCRIPTS *, const char *, const char *, unsigned char);
char        *scriptenv_free (SCRIPTS *, const char *);
SCRIPTS     scripts_new     (unsigned char);
void        scripts_free    (SCRIPTS *);

#endif
