#ifndef LIB_H
#define LIB_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void register_lib(const luaL_Reg *, lua_State *);

#endif
