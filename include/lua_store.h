#ifndef LUA_STORE_H
#define LUA_STORE_H

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef enum {
    LUA_T_EMPTY,
    LUA_T_STRING,
    LUA_T_NUMBER,
    LUA_T_BOOLEAN
} LuaT;

typedef struct {
    LuaT                    type;
    union {
        char                *string;
        double              number;
        bool                boolean;
    } data;
} LuaStore;

void        l_store_set     (LuaStore *, lua_State *);
void        l_store_update  (LuaStore *, lua_State *);
void        l_store_free    (LuaStore *);

#endif
