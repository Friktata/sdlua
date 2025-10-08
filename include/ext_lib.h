#ifndef EXT_LIB_H
#define EXT_LIB_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int     l_ext_status        (lua_State *);
int     l_lua_listenv       (lua_State *);
int     l_lua_arg           (lua_State *);

static const struct luaL_Reg ext_lib[] = {
    { "status",             l_ext_status },
    { "listenv",            l_lua_listenv },
    { "arg",                l_lua_arg },
    { NULL,                 NULL }
};

#endif
