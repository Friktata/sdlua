#ifndef EXEC_H
#define EXEC_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "scripts.h"
#include "lib.h"
#include "ext_lib.h"
#include "sdl_lib.h"

#include "lua_store.h"

#define EXEC_ERR_LEN            1024

char        *exec_concurrent    (FILE *, const char *, lua_State *, unsigned char);
char        *exec_scripts       (FILE *, const char **, const int, lua_State *, unsigned char, LuaStore *);
char        *exec_all           (FILE *, SCRIPTS *, lua_State **);

#endif
