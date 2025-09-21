#include "../include/lua_store.h"

void l_store_set(
    LuaStore                *store,
    lua_State               *state
) {
    if (store->type == LUA_T_STRING) {
        free(store->data.string);
        store->data.string = NULL;
    }

    if (lua_type(state, -1) == LUA_TNUMBER) {
        store->type = LUA_T_NUMBER;
        store->data.number = lua_tonumber(state, -1);
    }
    else if (lua_type(state, -1) == LUA_TSTRING) {
        store->type = LUA_T_STRING;
        store->data.string = strdup(lua_tostring(state, -1));
    }
    else if (lua_type(state, -1) == LUA_TBOOLEAN) {
        store->type = LUA_T_BOOLEAN;
        store->data.number = lua_toboolean(state, -1);
    }
    else {
        store->type = LUA_T_EMPTY;
    }

    l_store_update(store, state);
}

void l_store_update(
    LuaStore                *store,
    lua_State               *state
) {
    switch(store->type) {
        case LUA_T_NUMBER:
            lua_pushnumber(state, store->data.number);
            break;
        case LUA_T_STRING:
            lua_pushstring(state, store->data.string);
            free(store->data.string);
            store->data.string = NULL;
            break;
        case LUA_T_BOOLEAN:
            lua_pushboolean(state, store->data.boolean);
            break;
        default:
            lua_pushnil(state);
            break;
    }

    lua_setglobal(state, "return_status");
}

/**
 *
 */
void l_store_free(LuaStore *store) {
    if (store->type == LUA_T_STRING && store->data.string) {
        free(store->data.string);
        store->data.string = NULL;
    }
    store->type = LUA_T_EMPTY;
}
