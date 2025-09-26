#include "../include/ext_lib.h"

#include "../include/scripts.h"
#include "../include/app.h"

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

/**
 *
 */
int l_lua_arg(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (lua_gettop(state) == 0) {
        lua_pushinteger(state, app->argc);
        return 1;
    }

    int arg = lua_tointeger(state, 1);

    if (arg < 0 || arg >= app->argc) {
        lua_pushnil(state);
    }
    else {
        lua_pushstring(state, app->argv[arg]);
    }

    return 1;
}

/**
 * 
 */
int l_lua_argv(
    lua_State                   *state
) {
    return 0;
}
