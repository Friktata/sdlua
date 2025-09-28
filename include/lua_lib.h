#ifndef LUA_LIB_H
#define LUA_LIB_H

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#ifndef LUA_ERR_LEN
#define LUA_ERR_LEN     1024
#endif

int         __lua_error_msg             (lua_State *, const char *, ...);
char        *__lua_table_get_boolean    (lua_State *, char *, int, char *, bool*);
char        *__lua_table_get_integer    (lua_State *, char *, int, char *, int *);
const char  *__lua_table_get_string     (lua_State *, char *, int, char *);
char        *__lua_table_get_rgba       (lua_State *, char *, int, SDL_Color *);
char        *__lua_table_get_area       (lua_State *, char *, int, SDL_FRect *);
char        *__lua_table_get_coords     (lua_State *, char *, int, SDL_FRect *);

#endif
