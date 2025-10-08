#include "../include/ext_lib.h"
#include "../include/lua_store.h"

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
            snprintf(arg_name, LUA_ERR_LEN, "script_%i", index);
            lua_pushstring(state, app->scripts.scriptenv[env]->script[index]);
            lua_setfield(state, -2, arg_name);
        }
    }

    return 1;
}

/**
 *
 */
int l_lua_execenv(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (lua_gettop(state) != 1) {
        lua_pushstring(state, "execenv(): Function expexts exactly 1 parameter");
        return 1;
    }

    if (lua_type(state, -1) != LUA_TSTRING) {
        lua_pushfstring(state, "execenv(): String expected for first parameter");
        return 1;
    }

    const char *id = lua_tostring(state, -1);
    int env = scriptenv_find(&app->scripts, id);

    if (env < 0) {
        lua_pushfstring(state, "execenv(): Environment %s not found", id);
        return 1;
    }

    LuaStore return_status = { LUA_T_EMPTY };

    char cwd[FILENAME_MAX];

    getcwd(&cwd[0], FILENAME_MAX);

    char *err = exec_scripts(
        app->log,
        (const char **) app->scripts.scriptenv[env]->script,
        app->scripts.scriptenv[env]->size,
        app->scripts.scriptenv[env]->state,
        (SCRIPTS_F_LUALIBS | SCRIPTS_F_EXTLIBS | SCRIPTS_F_SDLLIBS),
        &return_status,
        cwd,
        id
    );

    if (err) {
        lua_pushstring(state, err);
        return 1;
    }

    if (return_status.type == LUA_T_STRING) {
        lua_pushstring(state, return_status.data.string);
        lua_setglobal(state, "__return_status");
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_lua_execscript(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (lua_gettop(state) != 2) {
        lua_pushstring(state, "execscript(): Function expexts exactly 2 parameters");
        return 1;
    }

    if (lua_type(state, 1) != LUA_TSTRING) {
        lua_pushfstring(state, "execscript(): String expected for first parameter");
        return 1;
    }
    if (lua_type(state, 2) != LUA_TSTRING) {
        lua_pushfstring(state, "execscript(): String expected for second parameter");
        return 1;
    }

    const char *id = lua_tostring(state, 1);
    const char *path = lua_tostring(state, 2);

    int index = scriptenv_find(&app->scripts, id);

    if (index < 0) {
        lua_pushfstring(state, "execscript(): Environment %s not found", id);
        return 1;
    }

    int script_index = -1;

    for (int script = 0; script < app->scripts.scriptenv[index]->size; script++) {
        if (strcmp(app->scripts.scriptenv[index]->script[script], path) == 0) {
            script_index = index;
            break;
        }
    }

    if (script_index < 0) {
        lua_pushfstring(state, "execscript(): Script %s not found in the %s environment", path, id);
        return 1;
    }

    char cwd[FILENAME_MAX];

    getcwd(&cwd[0], FILENAME_MAX);

    char *err = exec_concurrent(
        app->log,
        path,
        app->scripts.scriptenv[index]->state,
        (SCRIPTS_F_LUALIBS | SCRIPTS_F_EXTLIBS | SCRIPTS_F_SDLLIBS),
        cwd,
        id
    );
    
    if (err) {
        lua_pushstring(state, err);
        return 1;
    }

    LuaStore return_status;

    lua_getfield(app->scripts.scriptenv[index]->state, LUA_REGISTRYINDEX, "__return_status");

    if (! lua_isnil(app->scripts.scriptenv[index]->state, -1)) {
        if (return_status.type == LUA_T_STRING) {
            free(return_status.data.string);
            return_status.data.string = NULL;
        }
        l_store_set(&return_status, app->scripts.scriptenv[index]->state);
    }
    else {
        return_status.type = LUA_T_EMPTY;
    }

    if (return_status.type == LUA_T_STRING) {
        lua_pushstring(state, return_status.data.string);
        lua_setglobal(state, "__return_status");
    }

    lua_pushstring(state, "OK");
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

