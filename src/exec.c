#include "../include/exec.h"

/**
 *
 */
char *__get_script_directory(
    const char              *path
) {
    static char buffer[PATH_MAX];

    strncpy(buffer, path, PATH_MAX - 1);
    buffer[PATH_MAX - 1] = '\0';

#ifdef _WIN32
    char *last_slash = strrchr(buffer, '\\');
#else
    char *last_slash = strrchr(buffer, '/');
#endif

    if (last_slash) {
        *last_slash = '\0';
    }
    else {
        return NULL;
    }

    return buffer;
}

/**
 *
 */
char *__get_script_name(
    const char              *path
) {
    static char buffer[PATH_MAX];

    strncpy(buffer, path, PATH_MAX - 1);
    buffer[PATH_MAX - 1] = '\0';

#ifdef _WIN32
    char *last_slash = strrchr(buffer, '\\');
#else
    char *last_slash = strrchr(buffer, '/');
#endif

    if (last_slash) {
        return (last_slash + 1);
    }

    return (char *) path;
}

/**
 *
 */
char *exec_concurrent(
    FILE                    *log,
    const char              *path,
    lua_State               *state,
    const unsigned char     flags,
    const char              *cwd
) {
    static char err_msg[EXEC_ERR_LEN];

    if (log) {
        fprintf(log, ">>> Executing script %s\n", path);
        fflush(log);
    }

    if (! path) {
        snprintf(err_msg, EXEC_ERR_LEN, "Error in exec_concurrent(): The path pointer is NULL\n");
        return &err_msg[0];
    }
    if (! state) {
        snprintf(err_msg, EXEC_ERR_LEN, "Error in exec_concurrent(): The state pointer is NULL\n");
        return &err_msg[0];
    }

    if (flags & SCRIPTS_F_LUALIBS) {
        luaL_openlibs(state);
    }
    if (flags & SCRIPTS_F_EXTLIBS) {
        register_lib(&ext_lib[0], state);
    }
    if (flags & SCRIPTS_F_SDLLIBS) {
        register_lib(&sdl_lib[0], state);
        reg_sdl_init_flags(state);
        reg_sdl_win_flags(state);
        reg_sdl_cursor_types(state);
    }

    char *base_path = __get_script_directory(path);
    if (base_path) {
        if (chdir(base_path) < 0) {
            snprintf(err_msg, EXEC_ERR_LEN, "Error in exec_concurrent(): Base path %s not found - %s\n", base_path, strerror(errno));
            return &err_msg[0];
        }
        if (log) {
            fprintf(log, ">>> Changing to directory %s\n", base_path);
        }
    }

    if (luaL_dofile(state, __get_script_name(path)) != LUA_OK) {
        snprintf(err_msg, EXEC_ERR_LEN, "Error in exec_concurrent(): %s\n", lua_tostring(state, -1));
        return &err_msg[0];
    }

    if (base_path) {
        if (log) {
            fprintf(log, ">>> Returning to directory %s\n", cwd);
        }
        chdir(cwd);
    }

    return NULL;
}

/**
 *
 */
char *exec_scripts(
    FILE                    *log,
    const char              **script,
    const int               scripts,
    lua_State               *state,
    const unsigned char     flags,
    LuaStore                *return_status,
    const char              *cwd
) {
    static char err_msg[EXEC_ERR_LEN];
    char *err = NULL;

    if (! script) {
        snprintf(err_msg, EXEC_ERR_LEN, "Error in exec_scripts(): The script pointer is NULL\n");
        return &err_msg[0];
    }
    if (! state) {
        snprintf(err_msg, EXEC_ERR_LEN, "Error in exec_scripts(): The state pointer is NULL\n");
        return &err_msg[0];
    }

    for (int index = 0; index < scripts; index++) {
        lua_pushstring(state, "__return_status");
        lua_pushnil(state);
        lua_settable(state, LUA_REGISTRYINDEX);

        err = exec_concurrent(log, script[index], state, flags, cwd);

        if (err) {
            break;
        }

        lua_getfield(state, LUA_REGISTRYINDEX, "__return_status");

        if (!lua_isnil(state, -1)) {
            if (return_status->type == LUA_T_STRING) {
                free(return_status->data.string);
                return_status->data.string = NULL;
            }
            l_store_set(return_status, state);
        }
        else {
            return_status->type = LUA_T_EMPTY;
        }

        lua_pop(state, 1);
    }

    return err;
}

/**
 *
 */
char *exec_all(
    FILE                    *log,
    SCRIPTS                 *scripts,
    lua_State               **l_state
) {
    static char err_msg[EXEC_ERR_LEN];
    char *err = NULL;

    LuaStore return_status = { LUA_T_EMPTY };

    char current_path[FILENAME_MAX] = {0};
    char *cwd = getcwd(&current_path[0], FILENAME_MAX);

    if (! cwd) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in sort_args(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    if (log) {
        fprintf(log, ">>> SDLua initialising in directory %s\n", cwd);
    }

    if (! scripts) {
        snprintf(err_msg, EXEC_ERR_LEN, "Error in exec_all(): The scripts pointer is NULL\n");
        return &err_msg[0];
    }

    if (log) {
        fprintf(log, ">>> Running %d environments, %d total scripts...\n", scripts->size, scripts->scripts);
    }

    for (int scriptenv = 0; scriptenv < scripts->size; scriptenv++) {
        const char *env = scripts->id[scriptenv];

        const char **all_scripts = (const char **) scripts->scriptenv[scriptenv]->script;
        const int total_scripts = (const int) scripts->scriptenv[scriptenv]->size;
        const unsigned char flags = (const unsigned char) scripts->scriptenv[scriptenv]->flags;

        lua_State *state = scripts->scriptenv[scriptenv]->state;
        *l_state = state;

        if (log) {
            if (log) {
                fprintf(log, ">>> Executing %d scripts in environment %s...\n", scripts->scriptenv[scriptenv]->size, env);
                fflush(log);
            }
        }

        if (return_status.type != LUA_T_EMPTY) {
            l_store_update(&return_status, state);
        }

        err = exec_scripts(log, all_scripts, total_scripts, state, flags, &return_status, cwd);

        if (err) {
            break;
        }
    }

    if (return_status.type == LUA_T_STRING) {
        free(return_status.data.string);
        return_status.data.string = NULL;
    }

    l_store_free(&return_status);

    return err;
}
