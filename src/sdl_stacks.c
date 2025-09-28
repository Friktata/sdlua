#include "../include/sdl_stacks.h"

/**
 *
 */
SDL_Surface *__sdl_copy_surface(
    SDL_Surface                 *surface,
    char                        *err_msg
) {
    if (!surface) {
        return NULL;
    }

    // Prefer built-in duplicate if available
    SDL_Surface *dup = SDL_DuplicateSurface(surface);
    if (dup) {
        return dup;
    }

    // Fallback: manual create + blit

    // width and height are public fields in SDL_Surface in your build
    int width = surface->w;
    int height = surface->h;

    // The `format` member is now an enum representing SDL_PixelFormat
    SDL_PixelFormat format_enum = surface->format;

    SDL_Surface *copy_surface = SDL_CreateSurface(width, height, format_enum);
    if (!copy_surface) {
        snprintf(err_msg, SDL_ERR_LEN, "__sdl_copy_surface: %s", SDL_GetError());
        return NULL;
    }

    // Note: SDL_BlitSurface returns bool in SDL3 — true = success, false = failure. :contentReference[oaicite:3]{index=3}
    if (!SDL_BlitSurface(surface, NULL, copy_surface, NULL)) {
        // blit failed
        snprintf(err_msg, SDL_ERR_LEN, "__sdl_copy_surface: %s", SDL_GetError());
        SDL_DestroySurface(copy_surface);
        return NULL;
    }

    return copy_surface;
}

/**
 *
 */
char *__sdl_stack_new(
    SDL_Stacks                  *stacks,
    char                        *id,
    int                         size
) {
    static char err_msg[SDL_ERR_LEN] = {0};

    SDL_Stack *stack;
    HASH_FIND_STR(stacks->hash, id, stack);

    if (stack) {
        snprintf(err_msg, SDL_ERR_LEN, "Error in __sdl_stack_new(): A stack with id \"%s\" already exists\n", id);
        return &err_msg[0];
    }

    int current_stack = -1;

    for (int index = 0; index < stacks->size; index++) {
        if (! stacks->stack[index]) {
            current_stack = index;
            break;
        }
    }

    if (current_stack < 0) {
        if (stacks->stack) {
            current_stack = stacks->size++;
            SDL_Stack **newstacks = realloc(stacks->stack, (sizeof(SDL_Stack *) * stacks->size));
            if (! newstacks) {
                snprintf(err_msg, SDL_ERR_LEN, "realloc() error in __sdl_stack_new(): %s\n", strerror(errno));
                return &err_msg[0];
            }
            stacks->stack = newstacks;
        }
        else {
            if ((stacks->stack = malloc(sizeof(SDL_Stack *))) == NULL) {
                snprintf(err_msg, SDL_ERR_LEN, "malloc() error in __sdl_stack_new(): %s\n", strerror(errno));
                return &err_msg[0];
            }
            current_stack = 0;
            stacks->size = 1;
        }
    }

    if ((stacks->stack[current_stack] = malloc(sizeof(SDL_Stack))) == NULL) {
        snprintf(err_msg, SDL_ERR_LEN, "malloc() error in __sdl_stack_new(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    SDL_Surface **surfaces = malloc(sizeof(SDL_Surface *) * size);
    char *stack_id = malloc(strlen(id) + 1);

    if (! surfaces || ! stack_id) {
        snprintf(err_msg, SDL_ERR_LEN, "malloc() error in __sdl_stack_new(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    for (int surface = 0; surface < size; surface++) {
        surfaces[surface] = NULL;
    }

    snprintf(stack_id, (strlen(id) + 1), "%s", id);

    stacks->stack[current_stack]->surface = surfaces;
    stacks->stack[current_stack]->id = stack_id;
    stacks->stack[current_stack]->size = size;
    stacks->stack[current_stack]->surfaces = 0;
    stacks->stack[current_stack]->position = 0;

    HASH_ADD_KEYPTR(
        hh,
        stacks->hash,
        stacks->stack[current_stack]->id,
        strlen(stacks->stack[current_stack]->id),
        stacks->stack[current_stack]
    );

    return NULL;
}

/**
 *
 */
char *__sdl_stack_push(
    SDL_Stacks                  *stacks,
    char                        *id,
    SDL_Surface                 *surface
) {
    static char err_msg[SDL_ERR_LEN] = {0};

    SDL_Stack *stack;
    HASH_FIND_STR(stacks->hash, id, stack);

    if (!stack) {
        snprintf(err_msg, SDL_ERR_LEN, "Error in __sdl_stack_push(): Stack with id \"%s\" doesn’t exist\n", id);
        return &err_msg[0];
    }

    if (stack->surfaces >= stack->size) {
        if (stack->surface[0]) {
            SDL_DestroySurface(stack->surface[0]);
            stack->surface[0] = NULL;
        }

        for (int i = 1; i < stack->size; ++i) {
            stack->surface[i - 1] = stack->surface[i];
        }

        stack->position = stack->size - 2;  // New item will go at position+1
        stack->surfaces = stack->size - 1;
    }

    SDL_Surface *copy_surface = __sdl_copy_surface(surface, err_msg);
    if (!copy_surface) {
        return err_msg;
    }

    int push_index = stack->surfaces;
    stack->surface[push_index] = copy_surface;

    stack->surfaces++;
    stack->position = push_index;

    fprintf(stdout, "Pushed to stack %s (surfaces=%d, position=%d)\n", id, stack->surfaces, stack->position);

    return NULL;
}

/**
 *
 */
char *__sdl_stack_clear(
    SDL_Stacks                  *stacks,
    char                        *id
) {
    static char err_msg[SDL_ERR_LEN] = "";

    SDL_Stack *stack;
    HASH_FIND_STR(stacks->hash, id, stack);

    if (! stack) {
        snprintf(err_msg, SDL_ERR_LEN, "Error in __sdl_clear(): Stack with id \"%s\" doesn’t exist\n", id);
        return NULL;
    }

    for (int surface = (stack->position + 1); surface < stack->surfaces; surface++) {
        if (stack->surface[surface]) {
            SDL_DestroySurface(stack->surface[surface]);
            stack->surface[surface] = NULL;
        }
    }

    stack->surfaces = (stack->position + 1);

    return NULL;
}

/**
 *
 */
SDL_Surface *__sdl_stack_prev(
    SDL_Stacks                  *stacks,
    char                        *id,
    char                        *err_msg
) {
    SDL_Stack *stack;
    HASH_FIND_STR(stacks->hash, id, stack);

    if (! stack) {
        snprintf(err_msg, SDL_ERR_LEN, "Error in __sdl_stack_prev(): Stack with id \"%s\" doesn’t exist\n", id);
        return NULL;
    }

    if (stack->position < 1) {
        *err_msg = '\0';
        return NULL;
    }

    fprintf(stdout, "Retuning previous surface from stack %s (surfaces=%d, position=%d)\n", id, stack->surfaces, stack->position);

    stack->position--;
    return stack->surface[stack->position];
}

/**
 *
 */
SDL_Surface *__sdl_stack_next(
    SDL_Stacks                  *stacks,
    char                        *id,
    char                        *err_msg
) {
    SDL_Stack *stack;
    HASH_FIND_STR(stacks->hash, id, stack);

    if (!stack) {
        snprintf(err_msg, SDL_ERR_LEN, "Error in __sdl_stack_next(): Stack with id \"%s\" doesn’t exist\n", id);
        return NULL;
    }

    if ((stack->position + 1) >= stack->surfaces) {
        *err_msg = '\0';
        return NULL;
    }

    return stack->surface[++stack->position];
}

/**
 *
 */
char *__sdl_stack_free(
    SDL_Stacks                  *stacks,
    char                        *id
) {
    static char err_msg[SDL_ERR_LEN] = {0};

    SDL_Stack *stack;
    HASH_FIND_STR(stacks->hash, id, stack);

    if (! stack) {
        snprintf(err_msg, SDL_ERR_LEN, "Error in stack_free(): Stack with id \"%s\" doesn\'t exist\n", id);
        return &err_msg[0];
    }

    for (int surface = 0; surface < stack->surfaces; surface++) {
        if (stack->surface[surface]) {
            SDL_DestroySurface(stack->surface[surface]);
            stack->surface[surface] = NULL;
        }
    }

    free(stack->surface);
    
    if (stack->id) {
        free(stack->id);
        stack->id = NULL;
    }

    stack->size = 0;
    stack->surfaces = 0;
    stack->position = 0;

    return NULL;
}
