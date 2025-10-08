#ifndef EXEC_H
#define EXEC_H

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define PATH_SEPARATOR '\\'
    #ifndef PATH_MAX
        #define PATH_MAX MAX_PATH
    #endif
#else
    #include <unistd.h>
    #include <limits.h>
    #define PATH_SEPARATOR '/'
    #ifndef PATH_MAX
        #define PATH_MAX 4096
    #endif
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "scripts.h"
#include "lib.h"
#include "ext_lib.h"
#include "sdl_lib.h"

#include "lua_store.h"

#define EXEC_ERR_LEN            1024

char        *exec_concurrent    (FILE *, const char *, lua_State *, unsigned char, const char *, const char *);
char        *exec_scripts       (FILE *, const char **, const int, lua_State *, unsigned char, LuaStore *, const char *, const char *);
char        *exec_all           (FILE *, SCRIPTS *, lua_State **, unsigned char, LuaStore *);

#endif
