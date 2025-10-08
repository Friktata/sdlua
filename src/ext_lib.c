#include "../include/ext_lib.h"

#include "../include/scripts.h"
#include "../include/exec.h"
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
int l_lua_listenv(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));
    char arg_name[LUA_ERR_LEN] = "";

    if (lua_gettop(state) == 0) {
        lua_newtable(state);

        for (int index = 0; index < app->scripts.size; index++) {
            snprintf(arg_name, LUA_ERR_LEN, "env_%i", index);
            lua_pushstring(state, app->scripts.id[index]);
            lua_setfield(state, -2, arg_name);
            fprintf(stdout, ">>> Set %s = %s\n", arg_name, app->scripts.id[index]);
        }
    }
    else {
        const char *id = NULL;
        int env;

        if (lua_type(state, -1) != LUA_TSTRING) {
            if (lua_type(state, -1) != LUA_TNUMBER) {
                lua_pushstring(state, "listenv(): Parameter must be a string or a number\n");
                return 1;
            }

            int env = lua_tointeger(state, 1);

            if (env < 0 || env >= app->scripts.size) {
                lua_pushfstring(state, "listenv(): Index %d out of range\n", env);
                return 1;
            }
        }
        else {
            id = lua_tostring(state, -1);
            if ((env = scriptenv_find(&app->scripts, id)) < 0) {
                lua_pushfstring(state, "listenv(): Environment %s not found", id);
                return 1;
            }
        }

        lua_newtable(state);

        for (int index = 0; index < app->scripts.scriptenv[env]->size; index++) {
            snprintf(arg_name, LUA_ERR_LEN, "%s", app->scripts.scriptenv[env]->script[index]);
            lua_pushstring(state, app->scripts.scriptenv[env]->script[index]);
            lua_setfield(state, -2, arg_name);
            fprintf(stdout, ">>> Set %s = %s\n", arg_name, app->scripts.scriptenv[env]->script[index]);
        }
    }

    return 1;
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

