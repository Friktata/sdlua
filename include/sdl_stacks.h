#ifndef SDL_STACK_H
#define SDL_STACK_H

#include "../lib/uthash/include/uthash.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#ifndef SDL_ERR_LEN
#define SDL_ERR_LEN         1024
#endif

typedef struct __sdl_stack {
    SDL_Surface             **surface;
    char                    *id;
    int                     size;
    int                     surfaces;
    int                     position;
    UT_hash_handle          hh;
} SDL_Stack;

typedef struct __sdl_stacks {
    SDL_Stack               **stack;
    int                     size;
    struct __sdl_stack      *hash;
} SDL_Stacks;


SDL_Surface *__sdl_copy_surface (SDL_Surface *, char *);
char        *__sdl_stack_new    (SDL_Stacks *, char *, int);
char        *__sdl_stack_push   (SDL_Stacks *, char *, SDL_Surface *);
char        *__sdl_stack_clear  (SDL_Stacks *, char *);
SDL_Surface *__sdl_stack_prev   (SDL_Stacks *, char *, char *);
SDL_Surface *__sdl_stack_next   (SDL_Stacks *, char *, char *);
char        *__sdl_stack_free   (SDL_Stacks *, char *);

#endif
