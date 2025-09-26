#include "../include/lua_lib.h"

/**
 *
 */
int __lua_error_msg(
    lua_State                   *state,
    const char                  *fmt,
    ...
) {
    char err_msg[LUA_ERR_LEN] = {0};
    va_list list;
    
    va_start(list, fmt);
    vsnprintf(err_msg, LUA_ERR_LEN, fmt, list);
    va_end(list);

    lua_pushstring(state, err_msg);
    return 1;
}

/**
 *
 */
const char *__lua_table_get_string(
    lua_State                   *state,
    char                        *function_name,
    int                         pos,
    char                        *id
) {
    lua_getfield(state, pos, id);

    if (lua_isnoneornil(state, -1)) {
        __lua_error_msg(state, "%s No window[\"%s\"] specified\n", function_name, id);
        return NULL;
    }
    if (lua_type(state, -1) != LUA_TSTRING) {
        __lua_error_msg(state, "%s: The window[\"%s\"] parameter must be a string\n", function_name, id);
        return NULL;
    }

    const char *result = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    return result;
}

/**
 *
 */
char *__lua_table_get_integer(
    lua_State                   *state,
    char                        *function_name,
    int                         pos,
    char                        *id,
    int                         *number
) {
    lua_getfield(state, pos, id);

    if (lua_isnoneornil(state, -1)) {
        __lua_error_msg(state, "%s: No window[\"%s\"] specified\n", function_name, id);
        return NULL;
    }
    if (lua_type(state, -1) != LUA_TNUMBER) {
        __lua_error_msg(state, "%s: The window[\"%s\"] parameter must be an integer\n", function_name, id);
        return NULL;
    }

    *number = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    return "OK";
}

/**
 *
 */
char *__lua_table_get_boolean(
    lua_State                   *state,
    char                        *function_name,
    int                         pos,
    char                        *id,
    bool                        *bln
) {
    lua_getfield(state, pos, id);

    if (lua_isnoneornil(state, -1)) {
        __lua_error_msg(state, "%s: No window[\"%s\"] specified\n", function_name, id);
        return NULL;
    }
    if (lua_type(state, -1) != LUA_TBOOLEAN) {
        __lua_error_msg(state, "%s: The window[\"%s\"] parameter must be a boolean\n", function_name, id);
        return NULL;
    }

    *bln = lua_toboolean(state, -1);
    lua_pop(state, 1);

    return "OK";
}

/**
 *
 */
char *__lua_table_get_rgba(
    lua_State                   *state,
    char                        *function_name,
    int                         pos,
    SDL_Color                   *rgba
) {
    int red, green, blue, alpha;

    if (! __lua_table_get_integer(state, function_name, pos, "red", &red)) {
        return NULL;
    }
    if (! __lua_table_get_integer(state, function_name, pos, "green", &green)) {
        return NULL;
    }
    if (! __lua_table_get_integer(state, function_name, pos, "blue", &blue)) {
        return NULL;
    }
    if (! __lua_table_get_integer(state, function_name, pos, "alpha", &alpha)) {
        return NULL;
    }

    rgba->r = (uint8_t) red;
    rgba->g = (uint8_t) green;
    rgba->b = (uint8_t) blue;
    rgba->a = (uint8_t) alpha;

    return "OK";
}

/**
 *
 */
char *__lua_table_get_area(
    lua_State                   *state,
    char                        *function_name,
    int                         pos,
    SDL_FRect                   *area
) {
    int x, y, w, h;

    if (! __lua_table_get_integer(state, function_name, pos, "x", &x)) {
        return NULL;
    }
    if (! __lua_table_get_integer(state, function_name, pos, "y", &y)) {
        return NULL;
    }
    if (! __lua_table_get_integer(state, function_name, pos, "width", &w)) {
        return NULL;
    }
    if (! __lua_table_get_integer(state, function_name, pos, "height", &h)) {
        return NULL;
    }

    area->x = x;
    area->y = y;
    area->w = w;
    area->h = h;

    return "OK";
}
