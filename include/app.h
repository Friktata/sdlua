#ifndef SDLUA_H
#define SDLUA_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "scripts.h"
#include "sdl_entity.h"

#include "../lib/miniaudio/miniaudio.h"

#define APP_NAME            "SDLua"
#define APP_ERRMSG_LEN      1024

typedef struct __app        {
    FILE                    *log;
    FILE                    *err;
    char                    err_msg[APP_ERRMSG_LEN];
    SCRIPTS                 scripts;
    SDL_Entity              **entity;
    int                     entities;
    unsigned char           flags;
#define APP_F_SDLINIT       0x01
#define APP_F_TTFINIT       0x02
#define APP_F_AUDIOINIT     0x04
    SDL_Window              *window;
    SDL_Renderer            *renderer;
    ma_engine               engine;
    SDL_Entity              *hash;
} APP;

void        app_cleanup     (APP *);

#endif
