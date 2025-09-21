#include "../include/lib.h"

/**
 *
 */
void register_lib(const 
    luaL_Reg                *lib,
    lua_State               *state
) {
    for (const luaL_Reg *func = lib; func->name != NULL; func++) {
        lua_pushcfunction(state, func->func);
        lua_setglobal(state, func->name);
    }
}
