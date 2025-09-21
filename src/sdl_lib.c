#include "../include/sdl_lib.h"

#include "../include/scripts.h"
#include "../include/app.h"

extern ma_engine audio_engine;

/**
 *
 */
char *__sdl_entity(
    APP                         *app,
    const char                  *entity_id
) {
    static int index = 0;
    static char err_msg[LUA_ERR_LEN];
    
    char *err = NULL;

    SDL_Entity **entity = app->entity;
    int entities = app->entities;

    if (! entity) {
        if ((entity = malloc(sizeof(SDL_Entity *))) == NULL) {
            snprintf(err_msg, LUA_ERR_LEN, "malloc() error in __new_sql_entity(): %s\n", strerror(errno));
            return &err_msg[0];
        }
        entities = 0;
    }
    else {
        SDL_Entity **new_entities = realloc(entity, (sizeof(SDL_Entity *) * (entities + 1)));

        if (! new_entities) {
            snprintf(err_msg, LUA_ERR_LEN, "realloc() error in __new_sql_entity(): %s\n", strerror(errno));
            return &err_msg[0];
        }

        entity = new_entities;
    }

    if ((entity[entities] = malloc(sizeof(SDL_Entity))) == NULL) {
        free(entity);
        entity = NULL;
        snprintf(err_msg, LUA_ERR_LEN, "malloc() error in __new_sql_entity(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    entity[entities]->type = SDL_ENTITY_EMPTY;
    entity[entities]->index = index++;
    entity[entities]->surface = NULL;
    entity[entities]->texture = NULL;

    if ((entity[entities]->id = malloc(strlen(entity_id) + 1)) == NULL) {
        free(entity);
        entity = NULL;
        snprintf(err_msg, LUA_ERR_LEN, "malloc() error in __new_sql_entity(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    snprintf(entity[entities]->id, (strlen(entity_id) + 1), "%s", entity_id);

    app->entity = entity;
    app->entities = (entities + 1);

    HASH_ADD_KEYPTR(
        hh, app->hash,
        app->entity[entities]->id,
        strlen(app->entity[entities]->id),
        app->entity[entities]
    );

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL_Entity \"%s\"\n", entity_id);
    }

    return err;
}

/**
 *
 */
static void __sdl_keyboard_event(
    lua_State                   *state,
    const SDL_KeyboardEvent     *key
) {
    lua_newtable(state);

    lua_pushstring(state, "key");
    lua_pushinteger(state, key->key);
    lua_settable(state, -3);

    lua_pushstring(state, "char");
    lua_pushstring(state, SDL_GetKeyName(key->key));
    lua_settable(state, -3);

    lua_pushstring(state, "scancode");
    lua_pushinteger(state, key->scancode);
    lua_settable(state, -3);

    lua_pushstring(state, "mod");
    lua_pushinteger(state, key->mod);
    lua_settable(state, -3);

    lua_pushstring(state, "down");
    lua_pushboolean(state, key->down);
    lua_settable(state, -3);

    lua_pushstring(state, "repeat");
    lua_pushboolean(state, key->repeat);
    lua_settable(state, -3);
}

/**
 *
 */
static void __sdl_mouse_button_event(
    lua_State                   *state,
    const SDL_MouseButtonEvent  *btn
) {
    lua_newtable(state);

    lua_pushstring(state, "button");
    lua_pushinteger(state, btn->button);
    lua_settable(state, -3);

    lua_pushstring(state, "x");
    lua_pushinteger(state, btn->x);
    lua_settable(state, -3);

    lua_pushstring(state, "y");
    lua_pushinteger(state, btn->y);
    lua_settable(state, -3);
}

/**
 *
 */
static void __sdl_mouse_motion_event(
    lua_State                   *state,
    const SDL_MouseMotionEvent  *motion
) {
    lua_newtable(state);

    lua_pushstring(state, "x");
    lua_pushinteger(state, motion->x);
    lua_settable(state, -3);

    lua_pushstring(state, "y");
    lua_pushinteger(state, motion->y);
    lua_settable(state, -3);

    lua_pushstring(state, "xrel");
    lua_pushinteger(state, motion->xrel);
    lua_settable(state, -3);

    lua_pushstring(state, "yrel");
    lua_pushinteger(state, motion->yrel);
    lua_settable(state, -3);
}

/**
 *
 */
int l_sdl_init(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));
    char err_msg[LUA_ERR_LEN];

    if (app->flags & APP_F_SDLINIT) {
        lua_pushstring(state, "SDL_Init(): Already initialised\n");
        return 1;
    }

    const uint32_t sdl_flags = (uint32_t) lua_tointeger(state, 1);
    
    if (! SDL_Init(sdl_flags)) {
        snprintf(err_msg, LUA_ERR_LEN, "Error initialising SDL: %s\n", SDL_GetError());
        lua_pushstring(state, err_msg);
        return 1;
    }

    lua_getfield(state, 2, "title");
    const char *window_title = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 2, "width");
    const int window_width = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 2, "height");
    const int window_height = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 2, "flags");
    const int window_flags = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    if ((app->window = SDL_CreateWindow(
        window_title,
        window_width,
        window_height,
        window_flags
    )) == NULL) {
        snprintf(err_msg, LUA_ERR_LEN, "Error creating SDL window: %s\n", SDL_GetError());
        lua_pushstring(state, err_msg);
        return 1;
    }

    if ((app->renderer = SDL_CreateRenderer(app->window, NULL)) == NULL) {
        snprintf(err_msg, LUA_ERR_LEN, "Error creating SDL renderer: %s\n", SDL_GetError());
        lua_pushstring(state, err_msg);
        return 1;
    }

    app->flags |= APP_F_SDLINIT;

    if (app->log) {
        fprintf(app->log, ">>> Initialised SDL\n    Window title: %s\n    Window width: %d\n    Window height: %d\n    Window flags: %d\n",
            window_title,
            (int) window_width,
            (int) window_height,
            (int) window_flags
        );
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_quit(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Quit(): SDL is not initialised\n");
        return 1;
    }

    app_cleanup(app);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_delay(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Delay(): SDL not initialised");
        return 1;
    }

    int ms = luaL_checkinteger(state, 1);
    SDL_Delay(ms);
    
    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_event(
    lua_State                   *state
) {
    const char *event_name = luaL_checkstring(state, 1);
    luaL_checktype(state, 2, LUA_TFUNCTION);

    lua_pushstring(state, event_name);
    lua_pushvalue(state, 2);
    lua_settable(state, LUA_REGISTRYINDEX);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_poll(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Poll(): SDL not initialised");
        return 1;
    }

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        const char *event_name = NULL;

        switch (event.type) {
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
                event_name = "fullscreen";
                break;
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
                event_name = "windowed";
                break;
            case SDL_EVENT_QUIT:
                event_name = "quit";
                break;
            case SDL_EVENT_KEY_DOWN:
                event_name = "keydown";
                break;
            case SDL_EVENT_KEY_UP:
                event_name = "keyup";
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                event_name = "mousedown";
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                event_name = "mouseup";
                break;
            case SDL_EVENT_MOUSE_MOTION:
                event_name = "mousemove";
                break;
            default:
                continue;
        }

__lbl_add_event:

        lua_pushstring(state, event_name);
        lua_gettable(state, LUA_REGISTRYINDEX);

        if (!lua_isfunction(state, -1)) {
            lua_pop(state, 1);
            continue;
        }

        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                __sdl_keyboard_event(state, &event.key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                __sdl_mouse_button_event(state, &event.button);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                __sdl_mouse_motion_event(state, &event.motion);
                break;
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_QUIT:
                lua_newtable(state);
                break;
        }

        if (lua_pcall(state, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "Lua error in event handler: %s\n", lua_tostring(state, -1));
            lua_pop(state, 1);
        }

        if ((strcmp(event_name, "fullscreen") == 0) || (strcmp(event_name, "windowed") == 0)) {
            event_name = "resize";
            goto __lbl_add_event;
        }
    }

    return 0;
}

/**
 *
 */
int l_sdl_screen_info(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Info(): SDL not initialised");
        return 1;
    }

    SDL_Rect bounds;

    if (! SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &bounds) ) {
        lua_pushstring(state, SDL_GetError());
        return 1;
    }

    const int max_width = bounds.w;
    const int max_height = bounds.h;

    lua_newtable(state);
    
    lua_pushstring(state, "width");
    lua_pushinteger(state, max_width);
    lua_settable(state, -3);

    lua_pushstring(state, "height");
    lua_pushinteger(state, max_height);
    lua_settable(state, -3);

    return 1;
}

/**
 *
 */
int l_sdl_window_info(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Info(): SDL not initialised");
        return 1;
    }

    uint32_t window_width;
    uint32_t window_height;

    SDL_GetWindowSize(app->window, &window_width, &window_height);

    lua_newtable(state);
    
    lua_pushstring(state, "width");
    lua_pushinteger(state, window_width);
    lua_settable(state, -3);

    lua_pushstring(state, "height");
    lua_pushinteger(state, window_height);
    lua_settable(state, -3);

    return 1;
}

/**
 *
 */
int l_sdl_drawcolor(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Drawcolor(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "red");
    const uint8_t red = luaL_checkinteger(state, -1);
    lua_pop(state, 1);
    
    lua_getfield(state, 1, "green");
    const uint8_t green = luaL_checkinteger(state, -1);
    lua_pop(state, 1);
    
    lua_getfield(state, 1, "blue");
    const uint8_t blue = luaL_checkinteger(state, -1);
    lua_pop(state, 1);
    
    lua_getfield(state, 1, "alpha");
    const uint8_t alpha = luaL_checkinteger(state, -1);
    lua_pop(state, 1);
    
    SDL_SetRenderDrawColor(app->renderer, red, green, blue, alpha);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_fullscreen(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Fullscreen(): SDL not initialised");
        return 1;
    }

    luaL_checktype(state, 1, LUA_TBOOLEAN);

    const int bln = lua_toboolean(state, 1);

    if (!SDL_SetWindowFullscreen(app->window, bln)) {
        lua_pushstring(state, SDL_GetError());
        return 1;
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_surface(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));
    
    char err_msg[LUA_ERR_LEN];
    char *err = NULL;

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "width");
    const int entity_width = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "height");
    const int entity_height = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    if ((err = __sdl_entity(app, entity_id)) != NULL) {
        lua_pushstring(state, err);
        return 1;
    }

    SDL_Entity *entity = NULL;

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Surface(): Error creating entity \"%s\"\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    entity->surface = SDL_CreateSurface(entity_width, entity_height, SDL_PIXELFORMAT_ARGB8888);

    if (! entity->surface) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Surface(): %s\n", SDL_GetError());
        lua_pushstring(state, err_msg);
        return 1;
    }

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL surface %s (%dx%d)\n",
            entity_id,
            (int) entity_width,
            (int) entity_height
        );
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_fill_surface(
    lua_State                   *state
) {
    char err_msg[LUA_ERR_LEN];
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    SDL_Entity *entity = NULL;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Fill(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Fill(): Entity \"%s\" not found\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    lua_getfield(state, 1, "red");
    const uint8_t red = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "green");
    const uint8_t green = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "blue");
    const uint8_t blue = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "alpha");
    const uint8_t alpha = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    Uint32 rgba = SDL_MapSurfaceRGBA(entity->surface, red, green, blue, alpha);        
    SDL_FillSurfaceRect(entity->surface, NULL, rgba);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_texture(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Texture(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "x");
    const int entity_x = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "y");
    const int entity_y = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "width");
    const int entity_width = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "height");
    const int entity_height = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Texture(): Entity \"%s\" not found\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }
        entity->surface = SDL_CreateSurface(entity_width, entity_height, SDL_PIXELFORMAT_ARGB8888);
        if (! entity->surface) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Texture(): %s\n", SDL_GetError());
            lua_pushstring(state, err_msg);
            return 1;
        }
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }
    if ((entity->texture = SDL_CreateTextureFromSurface(app->renderer, entity->surface)) == NULL) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Texture(): %s\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    SDL_FRect area = { entity_x, entity_y, entity_width, entity_height };

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &area);
    memcpy(&entity->area, &area, (sizeof(SDL_FRect)));

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL texture %s @ %dx%d (%dx%d)\n",
            entity_id,
            (int) entity_x,
            (int) entity_y,
            (int) entity_width,
            (int) entity_height
        );
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_image(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Image(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "image");
    const char *entity_path = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "x");
    const int entity_x = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "y");
    const int entity_y = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "width");
    const int entity_width = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "height");
    const int entity_height = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Image(): Entity \"%s\" not found\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }
        entity->surface = SDL_CreateSurface(entity_width, entity_height, SDL_PIXELFORMAT_ARGB8888);
        if (! entity->surface) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Image(): %s\n", SDL_GetError());
            lua_pushstring(state, err_msg);
            return 1;
        }
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }
    if (entity->surface) {
        SDL_DestroySurface(entity->surface);
        entity->surface = NULL;
    }

    if ((entity->surface = IMG_Load(entity_path)) == NULL) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Image(): %s\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    if ((entity->texture = SDL_CreateTextureFromSurface(app->renderer, entity->surface)) == NULL) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Image(): %s\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    SDL_FRect area = { entity_x, entity_y, entity_width, entity_height };

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &area);
    memcpy(&entity->area, &area, (sizeof(SDL_FRect)));

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL image %s @ %dx%d (%dx%d)\n",
            entity_id,
            (int) entity_x,
            (int) entity_y,
            (int) entity_width,
            (int) entity_height
        );
    }

    if ((err = entity_set_image(entity, entity_path)) != NULL) {
        lua_pushstring(state, err);
        return 1;
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_text(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Text(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "font");
    const char *entity_path = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "size");
    const int entity_size = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "text");
    const char *entity_text = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "x");
    const int entity_x = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "y");
    const int entity_y = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "width");
    const int entity_width = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "height");
    const int entity_height = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "red");
    const uint8_t red = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "green");
    const uint8_t green = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "blue");
    const uint8_t blue = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "alpha");
    const uint8_t alpha = (uint8_t) luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Text(): Entity \"%s\" not found\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }
        entity->surface = SDL_CreateSurface(entity_width, entity_height, SDL_PIXELFORMAT_ARGB8888);
        if (! entity->surface) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Text(): %s\n", SDL_GetError());
            lua_pushstring(state, err_msg);
            return 1;
        }
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }
    if (entity->surface) {
        SDL_DestroySurface(entity->surface);
        entity->surface = NULL;
    }

    if (! (app->flags & APP_F_TTFINIT)) {
        if (app->log) {
            fprintf(app->log, ">>> Initialising TTF subsystem\n");
        }
        if (! TTF_Init()) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Text(): %s\n", SDL_GetError());
            lua_pushstring(state, err_msg);
            return 1;
        }
        app->flags |= APP_F_TTFINIT;
    }

    TTF_Font *font = TTF_OpenFont(entity_path, entity_size);
    
    if (! font) {
        snprintf(err_msg, LUA_ERR_LEN, "%s", SDL_GetError());
        lua_pushstring(state, err_msg);
        return 1;
    }

    SDL_FRect area = { entity_x, entity_y, entity_width, entity_height };
    SDL_Color rgba = { red, green, blue, alpha };

    entity->surface = TTF_RenderText_Blended(font, entity_text, strlen(entity_text), rgba);

    if (! entity->surface) {
        snprintf(err_msg, LUA_ERR_LEN, "%s", SDL_GetError());
        lua_pushstring(state, err_msg);
        return 1;
    }

    entity->texture = SDL_CreateTextureFromSurface(app->renderer, entity->surface);

    if (! entity->texture) {
        snprintf(err_msg, LUA_ERR_LEN, "%s", SDL_GetError());
        lua_pushstring(state, err_msg);
        return 1;
    }

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &area);
    memcpy(&entity->area, &area, (sizeof(SDL_FRect)));

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL text %s @ %dx%d (%dx%d), font=\"%s\", size=%d, text=\"%s\"\n",
            entity_id,
            entity_x,
            entity_y,
            entity_width,
            entity_height,
            entity_path,
            entity_size,
            entity_text
        );
    }

    if ((err = entity_set_text(entity, entity_path, entity_text, entity_size, rgba)) != NULL) {
        lua_pushstring(state, err);
        return 1;
    }

    TTF_CloseFont(font);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_update(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Audio(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Audio(): Entity \"%s\" not found\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    lua_getfield(state, 1, "x");
    const int entity_x = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "y");
    const int entity_y = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "width");
    const int entity_width = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "height");
    const int entity_height = luaL_checkinteger(state, -1);
    lua_pop(state, 1);

    entity->area.x = entity_x;
    entity->area.y = entity_y;
    entity->area.w = entity_width;
    entity->area.h = entity_height;

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_audio(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Audio(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "audio");
    const char *entity_path = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    lua_getfield(state, 1, "autoplay");
    const bool autoplay = lua_toboolean(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Audio(): Entity \"%s\" not found\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }
    if (entity->surface) {
        SDL_DestroySurface(entity->surface);
        entity->surface = NULL;
    }

    if (! (app->flags & APP_F_AUDIOINIT)) {
        if (ma_engine_init(NULL, &app->engine) != MA_SUCCESS) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Audio(): Error initialising audio engine\n");
            lua_pushstring(state, err_msg);
            return 1;
        }
        if (app->log) { 
            fprintf(app->log, ">>> Initialised audio engine\n");
        }
        app->flags |= APP_F_AUDIOINIT;
    }

    entity->decoder = (ma_decoder){0};

    if (ma_decoder_init_file(entity_path, NULL, &entity->decoder) != MA_SUCCESS) {
        printf("Failed to init decoder: %s\n", entity_path);
        return 1;
    }

    if (autoplay) {
        if (ma_sound_init_from_data_source(&app->engine, &entity->decoder, 0, NULL, &entity->sound) != MA_SUCCESS) {
            printf("Failed to init sound from decoder: %s\n", entity_path);
            return 1;
        }

        if (ma_engine_start(&app->engine) != MA_SUCCESS) {
            printf("Failed to start audio engine\n");
            return 1;
        }

        ma_sound_set_volume(&entity->sound, 1.0f);
        ma_engine_set_volume(&app->engine, 1.0f);

        ma_sound_start(&entity->sound);
    }


    if ((err = entity_set_audio(entity, entity_path)) != NULL) {
        lua_pushstring(state, err);
        return 1;
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_play(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Play(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Play(): Entity \"%s\" not found\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }
    }

    ma_result result;

    if (! ma_sound_is_playing(&entity->sound)) {
        result = ma_sound_start(&entity->sound);
        if (app->log) {
            fprintf(app->log, ">>> Playing audio entity %s\n", entity_id);
        }
    }

    if (result != MA_SUCCESS) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Play(): %s\n", ma_result_description(result));
        lua_pushstring(state, err_msg);
        return 1;
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_audio_info(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Pause(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Pause(): Entity \"%s\" not found\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }
    }

    ma_uint64 cursorFrames = 0;
    ma_uint64 totalFrames = 0;
    float position = 0.0f;
    float duration = 0.0f;
    float volume = 0.0f;
    ma_bool32 isPlaying = MA_FALSE;

    ma_sound_get_cursor_in_pcm_frames(&entity->sound, &cursorFrames);
    totalFrames = ma_decoder_get_length_in_pcm_frames(&entity->decoder, &totalFrames);

    if (entity->decoder.outputSampleRate > 0) {
        position = (float)cursorFrames / entity->decoder.outputSampleRate;
        duration = (float)totalFrames / entity->decoder.outputSampleRate;
    }

    volume = ma_sound_get_volume(&entity->sound);
    isPlaying = ma_sound_is_playing(&entity->sound);

    lua_newtable(state);

    lua_pushnumber(state, duration);
    lua_setfield(state, -2, "duration");

    lua_pushnumber(state, position);
    lua_setfield(state, -2, "position");

    lua_pushnumber(state, volume);
    lua_setfield(state, -2, "volume");

    lua_pushboolean(state, isPlaying);
    lua_setfield(state, -2, "playing");

    return 1;
}

/**
 *
 */
int l_sdl_pause(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Pause(): SDL not initialised");
        return 1;
    }

    lua_getfield(state, 1, "id");
    const char *entity_id = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Pause(): Entity \"%s\" not found\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }
    }

    ma_result result;

    if (ma_sound_is_playing(&entity->sound)) {
        result = ma_sound_stop(&entity->sound);
        if (app->log) {
            fprintf(app->log, ">>> Paused audio entity %s\n", entity_id);
        }
    }

    if (result != MA_SUCCESS) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Pause(): %s\n", ma_result_description(result));
        lua_pushstring(state, err_msg);
        return 1;
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_render(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Texture(): SDL not initialised");
        return 1;
    }

    const char *entity_id = luaL_checkstring(state, -1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        snprintf(err_msg, LUA_ERR_LEN, "Error in SDL_Texture(): Entity \"%s\" not found\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &entity->area);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_clear(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Clear(): SDL not initialised");
        return 1;
    }

    SDL_RenderClear(app->renderer);
    return 0;
}

/**
 *
 */
int l_sdl_present(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        lua_pushstring(state, "Error in SDL_Present(): SDL not initialised");
        return 1;
    }

    SDL_RenderPresent(app->renderer);
    return 0;
}

/**
 *
 */
void reg_sdl_init_flags(
    lua_State                   *state
) {
    lua_pushinteger(state, SDL_INIT_AUDIO);
    lua_setglobal(state, "SDL_INIT_AUDIO");

    lua_pushinteger(state, SDL_INIT_VIDEO);
    lua_setglobal(state, "SDL_INIT_VIDEO");

    lua_pushinteger(state, SDL_INIT_EVENTS);
    lua_setglobal(state, "SDL_INIT_EVENTS");

    lua_pushinteger(state, SDL_INIT_JOYSTICK);
    lua_setglobal(state, "SDL_INIT_JOYSTICK");
    
    lua_pushinteger(state, SDL_INIT_HAPTIC);
    lua_setglobal(state, "SDL_INIT_HAPTIC");
    
    lua_pushinteger(state, SDL_INIT_GAMEPAD);
    lua_setglobal(state, "SDL_INIT_GAMEPAD");
    
    lua_pushinteger(state, SDL_INIT_SENSOR);
    lua_setglobal(state, "SDL_INIT_SENSOR");
}

/**
 *
 */
void reg_sdl_win_flags(
    lua_State                   *state
) {
    lua_pushinteger(state, SDL_WINDOW_FULLSCREEN);
    lua_setglobal(state, "SDL_WINDOW_FULLSCREEN");
    
    lua_pushinteger(state, SDL_WINDOW_OPENGL);
    lua_setglobal(state, "SDL_WINDOW_OPENGL");
    
    lua_pushinteger(state, SDL_WINDOW_HIDDEN);
    lua_setglobal(state, "SDL_WINDOW_HIDDEN");
    
    lua_pushinteger(state, SDL_WINDOW_BORDERLESS);
    lua_setglobal(state, "SDL_WINDOW_BORDERLESS");
    
    lua_pushinteger(state, SDL_WINDOW_RESIZABLE);
    lua_setglobal(state, "SDL_WINDOW_RESIZABLE");
    
    lua_pushinteger(state, SDL_WINDOW_MINIMIZED);
    lua_setglobal(state, "SDL_WINDOW_MINIMIZED");
    
    lua_pushinteger(state, SDL_WINDOW_MAXIMIZED);
    lua_setglobal(state, "SDL_WINDOW_MAXIMIZED");
    
    lua_pushinteger(state, SDL_WINDOW_MOUSE_GRABBED);
    lua_setglobal(state, "SDL_WINDOW_MOUSE_GRABBED");
    
    lua_pushinteger(state, SDL_WINDOW_INPUT_FOCUS);
    lua_setglobal(state, "SDL_WINDOW_INPUT_FOCUS");
    
    lua_pushinteger(state, SDL_WINDOW_MOUSE_FOCUS);
    lua_setglobal(state, "SDL_WINDOW_MOUSE_FOCUS");
    
    lua_pushinteger(state, SDL_WINDOW_ALWAYS_ON_TOP);
    lua_setglobal(state, "SDL_WINDOW_ALWAYS_ON_TOP");
    
    lua_pushinteger(state, SDL_WINDOW_UTILITY);
    lua_setglobal(state, "SDL_WINDOW_UTILITY");
    
    lua_pushinteger(state, SDL_WINDOW_TOOLTIP);
    lua_setglobal(state, "SDL_WINDOW_TOOLTIP");
    
    lua_pushinteger(state, SDL_WINDOW_POPUP_MENU);
    lua_setglobal(state, "SDL_WINDOW_POPUP_MENU");
    
    lua_pushinteger(state, SDL_WINDOW_KEYBOARD_GRABBED);
    lua_setglobal(state, "SDL_WINDOW_KEYBOARD_GRABBED");
    
    lua_pushinteger(state, SDL_WINDOW_VULKAN);
    lua_setglobal(state, "SDL_WINDOW_VULKAN");
    
    lua_pushinteger(state, SDL_WINDOW_METAL);
    lua_setglobal(state, "SDL_WINDOW_METAL");
}
