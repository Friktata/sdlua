#ifndef SDL_ENTITY_H
#define SDL_ENTITY_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../lib/uthash/include/uthash.h"
#include "../lib/miniaudio/miniaudio.h"

#define SDL_ENTITY_ERR_LEN  1024

typedef enum {
    SDL_ENTITY_EMPTY,
    SDL_ENTITY_IMAGE,
    SDL_ENTITY_FONT,
    SDL_ENTITY_AUDIO
} SDL_EntityT;

typedef struct __sdl_image {
    char                    *path;
} SDL_EntityImage;

typedef struct __sdl_font {
    char                    *path;
    char                    *string;
    int                     size;
    SDL_Color               rgba;
} SDL_EntityFont;

typedef struct __sdl_audio {
    char                    *path;
} SDL_EntityAudio;

typedef struct __sdl_entity {
    SDL_EntityT             type;
    int                     index;
    char                    *id;
    SDL_FRect               area;
    SDL_Surface             *surface;
    SDL_Texture             *texture;
    union {
        SDL_EntityImage     image;
        SDL_EntityFont      font;
        SDL_EntityAudio     audio;
    } data;
    UT_hash_handle          hh;
    unsigned char           flags;
    ma_sound                sound;
    ma_decoder              decoder;
#define SDL_E_INVISIBLE     0x01
} SDL_Entity;

void    entity_free         (SDL_Entity *);
char    *entity_set_image   (SDL_Entity *, const char *);
char    *entity_set_text    (SDL_Entity *, const char *, const char *, const int, const SDL_Color);
char    *entity_set_audio   (SDL_Entity *, const char *);

#endif
