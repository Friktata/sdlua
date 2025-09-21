#include "../include/ext_lib.h"

/**
 *
 */
int l_ext_status(
    lua_State                   *state
) {
    const int args = (int) lua_gettop(state);

    if (args != 1) {
        return luaL_error(state, "Error: status() requires exactly one argument\n");
    }

    lua_pushstring(state, "__return_status");
    lua_pushvalue(state, 1);
    lua_settable(state, LUA_REGISTRYINDEX);

    return 0;
}
