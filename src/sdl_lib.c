#include "../include/sdl_lib.h"
#include "../include/sdl_stacks.h"

#include "../include/scripts.h"
#include "../include/app.h"

extern ma_engine audio_engine;

/**
 *
 */
char *__sdl_error_msg(
    const char                  *fmt,
    ...
) {
    static char err_msg[LUA_ERR_LEN] = {0};
    va_list list;

    va_start(list, fmt);
    vsnprintf(err_msg, LUA_ERR_LEN, fmt, list);
    va_end(list);

    return (err_msg[0] == '\0') ? NULL : &err_msg[0];
}

/**
 *
 */
char *__sdl_entity(APP *app, const char *entity_id) {
    static int index = 0;

    SDL_Entity **entity = app->entity;
    int entities = app->entities;

    int entity_index = -1;

    for (int i = 0; i < app->entities; i++) {
        if (app->entity[i]->type == SDL_ENTITY_EMPTY) {
            if (app->entity[i]->surface == NULL && app->entity[i]->texture == NULL) {
                entity_index = i;
                break;
            }
        }
    }

    if (entity_index == -1) {
        if (!entity) {
            entity = malloc(sizeof(SDL_Entity *));
            if (!entity) {
                return __sdl_error_msg("malloc() error in __sdl_entity(): %s", strerror(errno));
            }
            entities = 0;
        }
        else {
            SDL_Entity **new_entities = realloc(entity, sizeof(SDL_Entity *) * (entities + 1));
            if (!new_entities) {
                return __sdl_error_msg("realloc() error in __sdl_entity(): %s", strerror(errno));
            }
            entity = new_entities;
        }
        if (app->log) {
            fprintf(app->log, "Creating new entity %d (%s)\n", entities, entity_id);
        }
    }
    else {
        if (app->log) {
            fprintf(app->log, "Re-using defunct entity %d (%s)\n", entity_index, entity_id);
        }

        entities = entity_index;
    }

    SDL_Entity *e = malloc(sizeof(SDL_Entity));
    if (!e) {
        return __sdl_error_msg("malloc() error in __sdl_entity(): %s", strerror(errno));
    }

    e->id = strdup(entity_id);
    if (!e->id) {
        free(e);
        return __sdl_error_msg("strdup() error in __sdl_entity(): %s", strerror(errno));
    }

    e->type = SDL_ENTITY_EMPTY;
    e->index = index++;
    e->surface = NULL;
    e->texture = NULL;
    e->flags = 0;
    
    memset(&e->area, 0, sizeof(SDL_FRect));
    memset(&e->sound, 0, sizeof(ma_sound));
    memset(&e->decoder, 0, sizeof(ma_decoder));
    memset(&e->data, 0, sizeof(e->data));

    entity[entities] = e;

    app->entity = entity;
    app->entities = entities + 1;

    HASH_ADD_KEYPTR(hh, app->hash, e->id, strlen(e->id), e);

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL_Entity \"%s\"\n", e->id);
    }

    return NULL;
}


/**
 *
 */
char *__sdl_surface_put_pixel(
    SDL_Surface *surface,
    const int x,
    const int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    uint8_t alpha
) {
    static char err_msg[LUA_ERR_LEN];

    if (x < 0 || y < 0 || x >= surface->w || y >= surface->h) {
        return NULL;
    }

    if (SDL_MUSTLOCK(surface)) {
        if (SDL_LockSurface(surface) != 0) {
            return __sdl_error_msg("SDL_Putpixel(): Unable to lock surface: %s\n", SDL_GetError());
        }
    }

    Uint8 *pixels = (Uint8 *) surface->pixels;
    int bpp = SDL_BYTESPERPIXEL(surface->format);
    int pitch = surface->pitch;
    Uint8 *p = pixels + y * pitch + x * bpp;

    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);

    if (!fmt) {
        if (SDL_MUSTLOCK(surface)) {
            SDL_UnlockSurface(surface);
        }
        return __sdl_error_msg("SDL_Putpixel(): SDL_GetPixelFormatDetails failed: %s\n", SDL_GetError());
    }

    Uint32 pixel_value = SDL_MapRGBA(fmt, NULL, red, green, blue, alpha);

    switch (bpp) {
        case 1:
            *p = pixel_value & 0xFF;
            break;
        case 2:
            *(Uint16 *)p = pixel_value & 0xFFFF;
            break;
        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            p[0] = (pixel_value >> 16) & 0xFF;
            p[1] = (pixel_value >> 8) & 0xFF;
            p[2] = pixel_value & 0xFF;
#else
            p[0] = pixel_value & 0xFF;
            p[1] = (pixel_value >> 8) & 0xFF;
            p[2] = (pixel_value >> 16) & 0xFF;
#endif
            break;
        case 4:
            *(Uint32 *)p = pixel_value;
            break;
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }

    return NULL;
}

/**
 *
 */
char *__sdl_surface_get_pixel(
    SDL_Surface *surface,
    int x,
    int y,
    uint8_t *red,
    uint8_t *green,
    uint8_t *blue,
    uint8_t *alpha
) {
    static char err_msg[LUA_ERR_LEN];

    if (x < 0 || y < 0 || x >= surface->w || y >= surface->h) {
        return NULL;
    }

    bool locked = false;
    if (SDL_MUSTLOCK(surface)) {
        if (SDL_LockSurface(surface) != 0) {
            return __sdl_error_msg("SDL_Getpixel(): Unable to lock surface: %s\n", SDL_GetError());
        }
        locked = true;
    }

    Uint8 *pixels = (Uint8 *) surface->pixels;
    int bpp = SDL_BYTESPERPIXEL(surface->format);
    int pitch = surface->pitch;
    Uint8 *p = pixels + y * pitch + x * bpp;

    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);
    if (!fmt) {
        if (locked) {
            SDL_UnlockSurface(surface);
        } 
        return __sdl_error_msg("SDL_Getpixel(): SDL_GetPixelFormatDetails failed: %s\n", SDL_GetError());
    }

    Uint32 pixel_value;

    switch (bpp) {
        case 1:
            pixel_value = *p;
            break;
        case 2:
            pixel_value = *(Uint16 *) p;
            break;
        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            pixel_value = p[0] << 16 | p[1] << 8 | p[2];
#else
            pixel_value = p[0] | p[1] << 8 | p[2] << 16;
#endif
            break;
        case 4:
            pixel_value = *(Uint32 *) p;
            break;
        default:
            if (locked) {
                SDL_UnlockSurface(surface);
            }    
            return NULL;
    }

    SDL_GetRGBA(pixel_value, fmt, NULL, red, green, blue, alpha);

    if (locked) {
        SDL_UnlockSurface(surface);
    }

    return NULL;
}

/**
 *
 */
char *__sdl_surface_fill_circle(
    SDL_Surface *surface,
    const int cx,
    const int cy,
    const int diameter,
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    uint8_t alpha
) {
    static char err_msg[LUA_ERR_LEN] = {0};

    if (!surface) {
        return __sdl_error_msg("__sdl_surface_fill_circle: surface is NULL\n");
    }
    if (diameter <= 0) {
        return __sdl_error_msg("__sdl_surface_fill_circle: diameter must be >0\n");
    }

    int radius = diameter / 2;

    if (SDL_MUSTLOCK(surface)) {
        if (SDL_LockSurface(surface) != 0) {
            return __sdl_error_msg("__sdl_surface_fill_circle: lock failed: %s\n", SDL_GetError());
        }
    }

    for (int dy = -radius; dy <= radius; dy++) {
        int yy = cy + dy;
        double dx_f = sqrt((double)radius * radius - (double)dy * dy);
        int dx = (int)floor(dx_f + 0.5);

        int x_start = cx - dx;
        int x_end   = cx + dx;

        for (int xx = x_start; xx <= x_end; xx++) {
            __sdl_surface_put_pixel(surface, xx, yy, red, green, blue, alpha);
        }
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }

    return NULL;
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
static void __sdl_mouse_wheel_event(
    lua_State                   *state,
    const SDL_MouseWheelEvent   *wheel
) {
    lua_newtable(state);

    lua_pushstring(state, "type");
    lua_pushstring(state, "mouse_wheel");
    lua_settable(state, -3);

    lua_pushstring(state, "x");
    lua_pushnumber(state, wheel->x);
    lua_settable(state, -3);

    lua_pushstring(state, "y");
    lua_pushnumber(state, wheel->y);
    lua_settable(state, -3);

    lua_pushstring(state, "mouse_x");
    lua_pushinteger(state, wheel->mouse_x);
    lua_settable(state, -3);

    lua_pushstring(state, "mouse_y");
    lua_pushinteger(state, wheel->mouse_y);
    lua_settable(state, -3);

    lua_pushstring(state, "direction");
    lua_pushinteger(state, wheel->direction);
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

    if (app->flags & APP_F_SDLINIT) {
        return __lua_error_msg(state, "SDL_Init(): Already initialised\n");
    }

    if (lua_gettop(state) != 2) {
        return __lua_error_msg(state, "SDL_Init(): Exactly 2 parameters expected\n");
    }
    if (! lua_isinteger(state, 1)) {
        return __lua_error_msg(state, "SDL_Init(): Integer expected for first parameter\n");
    }
    if (! lua_istable(state, 2)) {
        return __lua_error_msg(state, "SDL_Init(): Table expected for second parameter\n");
    }

    const uint32_t sdl_flags = (uint32_t) lua_tointeger(state, 1);
    if (! SDL_Init(sdl_flags)) {
        return __lua_error_msg(state, "SDL_Init(): $s\n", SDL_GetError());
    }

    const char *window_title = __lua_table_get_string(state, "SDL_Init()", 2, "title");
    if (! window_title) return 1;

    int window_width, window_height, window_flags;

    if (! __lua_table_get_integer(state, "SDL_Init()", 2, "width", &window_width)) {
        return 1;
    }
    if (! __lua_table_get_integer(state, "SDL_Init()", 2, "height", &window_height)) {
        return 1;
    }
    if (! __lua_table_get_integer(state, "SDL_Init()", 2, "flags", &window_flags)) {
        return 1;
    }

    if ((app->window = SDL_CreateWindow(
        window_title,
        window_width,
        window_height,
        window_flags
    )) == NULL) {
        return __lua_error_msg(state, "SDL_Init(): %s\n", SDL_GetError());
    }

    if ((app->renderer = SDL_CreateRenderer(app->window, NULL)) == NULL) {
        return __lua_error_msg(state, "SDL_Init(): %s\n", SDL_GetError());
    }

    app->flags |= APP_F_SDLINIT;

    if (app->log) {
        fprintf(app->log, ">>> Initialised SDL\n    Window title: %s\n    Window width: %d\n    Window height: %d\n    Window flags: %d\n",
            window_title,
            window_width,
            window_height,
            window_flags
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
        return __lua_error_msg(state, "SDL_Quit(): SDL is not initialised\n");
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
        return __lua_error_msg(state, "SDL_Delay(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Delay(): Exactly 1 parameter expected\n");
    }
    if (! lua_isinteger(state, 1)) {
        return __lua_error_msg(state, "SDL_Delay(): Integer expected for first parameter\n");
    }
    
    SDL_Delay(lua_tointeger(state, 1));
    
    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_event(
    lua_State                   *state
) {
    if (lua_gettop(state) != 2) {
        return __lua_error_msg(state, "SDL_Event(): Exactly 1 parameter expected\n");
    }
    if (lua_type(state, 1) != LUA_TSTRING) {
        return __lua_error_msg(state, "SDL_Event(): String expected for first parameter\n");
    }
    if (lua_type(state, 2) != LUA_TFUNCTION) {
        return __lua_error_msg(state, "SDL_Event(): Function expected for second parameter\n");
    }

    const char *event_name = luaL_checkstring(state, 1);

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
        return __lua_error_msg(state, "SDL_Poll(): SDL not initialised");
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
            case SDL_EVENT_WINDOW_RESIZED:
                event_name = "resize";
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
            case SDL_EVENT_MOUSE_WHEEL:
                event_name = "scroll";
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
            case SDL_EVENT_MOUSE_WHEEL:
                __sdl_mouse_wheel_event(state, &event.wheel);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                __sdl_mouse_motion_event(state, &event.motion);
                break;
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_QUIT:
                lua_newtable(state);
                break;
        }

        if (lua_pcall(state, 1, 0, 0) != LUA_OK) {
            __lua_error_msg(state, "SDL_Poll(): Error in event handler (%s) %s\n", event_name, lua_tostring(state, -1));
            lua_pop(state, 1);
            return 1;
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
        return __lua_error_msg(state, "SDL_Info(): SDL not initialised");
    }

    SDL_Rect bounds;

    if (! SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &bounds) ) {
        return __lua_error_msg(state, "%s", SDL_GetError());
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
        return __lua_error_msg(state, "SDL_Windowinfo(): SDL not initialised");
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

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Drawcolor(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Drawcolor(): Table expected for first parameter\n");
    }
    
    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Drawcolor(): SDL not initialised");
    }

    SDL_Color rgba;
    if (! __lua_table_get_rgba(state, "SDL_Drawcolor()", 1, &rgba)) {
        return 1;
    }
    
    SDL_SetRenderDrawColor(app->renderer, rgba.r, rgba.g, rgba.b, rgba.a);

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
        return __lua_error_msg(state, "SDL_Fullscreen(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Fullscreen(): Exactly 1 parameter expected\n");
    }
    if (! lua_isboolean(state, 1)) {
        return __lua_error_msg(state, "SDL_Fullscreen(): Boolean expected for first parameter\n");
    }

    luaL_checktype(state, 1, LUA_TBOOLEAN);

    const int bln = lua_toboolean(state, 1);

    if (! SDL_SetWindowFullscreen(app->window, bln)) {
        return __lua_error_msg(state, SDL_GetError());
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
SDL_Cursor *__sdl_get_cursor(
    APP                         *app,
    const char                  *cursor_name
) {
    const char *sdl_cursor_names[SDL_CURSORS_MAX] = {
        "arrow",
        "ibeam",
        "wait",
        "crosshair",
        "waitarrow",
        "sizenwse",
        "sizenesw",
        "sizewe",
        "sizens",
        "sizeall",
        "no",
        "hand"
    };

    static SDL_Cursor *sdl_cursors[SDL_CURSORS_MAX] = { NULL };
    static SDL_Cursor *sdl_custom[SDL_CUSTOM_CURSORS] = { NULL };

    for (int cursor = 0; cursor < SDL_CURSORS_MAX; cursor++) {
        if (! sdl_cursors[cursor]) {
            sdl_cursors[cursor] = SDL_CreateSystemCursor(cursor);
            if (app->log) {
                fprintf(app->log, ">>> Created system cursor %s\n", sdl_cursor_names[cursor]);
            }
        }

        if (cursor_name && (strcmp(cursor_name, sdl_cursor_names[cursor]) == 0)) {
            return sdl_cursors[cursor];
        }
    }

    return NULL;
}

/**
 *
 */
int l_sdl_set_cursor(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Setcursor(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Setcursor(): Exactly 1 parameter expected\n");
    }
    if (lua_type(state, 1) != LUA_TSTRING) {
        return __lua_error_msg(state, "SDL_Setcursor(): String expected for first parameter\n");
    }

    char *cursor_name = (char *) lua_tostring(state, 1);

    if (strcmp(cursor_name, "default") == 0) {
        cursor_name = "arrow";
    }

    SDL_Cursor *new_cursor = __sdl_get_cursor(app, cursor_name);

    if (! new_cursor) {
        return __lua_error_msg(state, "SDL_Setcursor(): Unknown cursor reference \"%s\"\n", cursor_name);
    }

    app->cursor_name = cursor_name;
    app->cursor = new_cursor;

    SDL_SetCursor(new_cursor);

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

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Surface(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Surface(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Surface()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    int entity_width, entity_height;
    char *result = __lua_table_get_integer(state, "SDL_Surface()", 1, "width", &entity_width);
    if (! result) {
        return 1;
    }

    result = __lua_table_get_integer(state, "SDL_Surface()", 1, "height", &entity_height);
    if (! result) {
        return 1;
    }

    if ((err = __sdl_entity(app, entity_id)) != NULL) {
        lua_pushstring(state, err);
        return 1;
    }

    SDL_Entity *entity = NULL;

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Surface(): Error creating entity \"%s\"\n", entity_id);
    }

    entity->surface = SDL_CreateSurface(entity_width, entity_height, SDL_PIXELFORMAT_ARGB8888);

    if (! entity->surface) {
        return __lua_error_msg(state, "SDL_Surface(): %s\n", SDL_GetError());
    }

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL surface %s (%dx%d)\n",
            entity_id,
            entity_width,
            entity_height
        );
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_destroy_surface(
    lua_State                   *state
) {
    char err_msg[LUA_ERR_LEN];
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    SDL_Entity *entity = NULL;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Destroy(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Destroy(): Exactly 1 parameter expected\n");
    }
    if (lua_type(state, 1) != LUA_TSTRING) {
        return __lua_error_msg(state, "SDL_Destroy(): String expected for first parameter\n");
    }

    const char *entity_id = lua_tostring(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Destroy(): Entity \"%s\" not found\n", entity_id);
    }

    if (app->log) {
        fprintf(app->log, ">>> Destroyed entity %s\n", entity_id);
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }
    if (entity->surface) {
        SDL_DestroySurface(entity->surface);
        entity->surface = NULL;
    }

    entity_free(entity);

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
        return __lua_error_msg(state, "SDL_Fill(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Fill(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Fill(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Fill()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Fill(): Entity \"%s\" not found\n", entity_id);
    }

    SDL_Color rgba;
    char *result = __lua_table_get_rgba(state, "SDL_Fill()", 1, &rgba);
    if (! result) {
        return 1;
    }

    Uint32 sdl_color = SDL_MapSurfaceRGBA(entity->surface, rgba.r, rgba.g, rgba.b, rgba.a);        
    SDL_FillSurfaceRect(entity->surface, NULL, sdl_color);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_draw_gradient(lua_State *state) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));
    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (!(app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_DrawGradientRect(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_DrawGradientRect(): Exactly 1 parameter expected");
    }

    if (!lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_DrawGradientRect(): Table expected for first parameter");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_DrawGradientRect()", 1, "id");
    if (!entity_id) {
        return 1;
    }

    SDL_FRect area;
    if (!__lua_table_get_area(state, "SDL_DrawGradientRect()", 1, &area)) {
        return 1;
    }

    SDL_Color color_start;
    lua_getfield(state, 1, "color_start");
    if (!lua_istable(state, -1)) {
        return __lua_error_msg(state, "SDL_DrawGradientRect(): color_start must be a table");
    }
    if (!__lua_table_get_rgba(state, "SDL_DrawGradientRect()", -1, &color_start)) {
        return 1;
    }
    lua_pop(state, 1);

    SDL_Color color_end;
    lua_getfield(state, 1, "color_end");
    if (!lua_istable(state, -1)) {
        return __lua_error_msg(state, "SDL_DrawGradientRect(): color_end must be a table");
    }
    if (!__lua_table_get_rgba(state, "SDL_DrawGradientRect()", -1, &color_end)) {
        return 1;
    }
    lua_pop(state, 1);

    const char *direction = __lua_table_get_string(state, "SDL_DrawGradientRect()", 1, "direction");
    if (!direction) {
        direction = "horizontal";
    }

    HASH_FIND_STR(app->hash, entity_id, entity);
    if (!entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (!entity) {
            return __lua_error_msg(state, "SDL_DrawGradientRect(): Entity \"%s\" not found", entity_id);
        }
    }

    if (!entity->surface) {
        return __lua_error_msg(state, "SDL_DrawGradientRect(): Entity surface not found");
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }

    if (SDL_MUSTLOCK(entity->surface)) {
        if (SDL_LockSurface(entity->surface) != 0) {
            return __lua_error_msg(state, "SDL_DrawGradientRect(): %s", SDL_GetError());
        }
    }

    Uint32 *pixels = (Uint32 *) entity->surface->pixels;
    int pitch = entity->surface->pitch / sizeof(Uint32);
    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(entity->surface->format);

    for (int row = 0; row < (int) area.h; row++) {
        for (int col = 0; col < (int) area.w; col++) {
            float t = 0.0f;
            if (strcmp(direction, "vertical") == 0) {
                t = (float) row / ((float) area.h - 1.0f);
            } else {
                t = (float) col / ((float) area.w - 1.0f);
            }

            Uint8 r = (Uint8) ((1.0f - t) * color_start.r + t * color_end.r);
            Uint8 g = (Uint8) ((1.0f - t) * color_start.g + t * color_end.g);
            Uint8 b = (Uint8) ((1.0f - t) * color_start.b + t * color_end.b);
            Uint8 a = (Uint8) ((1.0f - t) * color_start.a + t * color_end.a);

            Uint32 pixel_color = SDL_MapRGBA(fmt, NULL, r, g, b, a);

            int px = col;
            int py = row;
            if ((px >= 0) && (px < entity->surface->w) && (py >= 0) && (py < entity->surface->h)) {
                pixels[py * pitch + px] = pixel_color;
            }
        }
    }

    if (SDL_MUSTLOCK(entity->surface)) {
        SDL_UnlockSurface(entity->surface);
    }

    lua_pushstring(state, "OK");
    return 1;
}
/**
 *
 */
int l_sdl_quad_gradient(lua_State *state) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));
    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (!(app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_QuadGradient(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_QuadGradient(): Exactly 1 parameter expected");
    }

    if (!lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_QuadGradient(): Table expected for first parameter");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_QuadGradient()", 1, "id");
    if (!entity_id) {
        return 1;
    }

    SDL_FRect area;
    if (!__lua_table_get_area(state, "SDL_QuadGradient()", 1, &area)) {
        return 1;
    }

    SDL_Color top_left;
    lua_getfield(state, 1, "top_left");
    if (!lua_istable(state, -1)) {
        return __lua_error_msg(state, "SDL_QuadGradient(): top_left must be a table");
    }
    if (!__lua_table_get_rgba(state, "SDL_QuadGradient()", -1, &top_left)) {
        return 1;
    }
    lua_pop(state, 1);

    SDL_Color top_right;
    lua_getfield(state, 1, "top_right");
    if (!lua_istable(state, -1)) {
        return __lua_error_msg(state, "SDL_QuadGradient(): top_right must be a table");
    }
    if (!__lua_table_get_rgba(state, "SDL_QuadGradient()", -1, &top_right)) {
        return 1;
    }
    lua_pop(state, 1);

    SDL_Color bottom_left;
    lua_getfield(state, 1, "bottom_left");
    if (!lua_istable(state, -1)) {
        return __lua_error_msg(state, "SDL_QuadGradient(): bottom_left must be a table");
    }
    if (!__lua_table_get_rgba(state, "SDL_QuadGradient()", -1, &bottom_left)) {
        return 1;
    }
    lua_pop(state, 1);

    SDL_Color bottom_right;
    lua_getfield(state, 1, "bottom_right");
    if (!lua_istable(state, -1)) {
        return __lua_error_msg(state, "SDL_QuadGradient(): bottom_right must be a table");
    }
    if (!__lua_table_get_rgba(state, "SDL_QuadGradient()", -1, &bottom_right)) {
        return 1;
    }
    lua_pop(state, 1);

    HASH_FIND_STR(app->hash, entity_id, entity);
    if (!entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (!entity) {
            return __lua_error_msg(state, "SDL_QuadGradient(): Entity \"%s\" not found", entity_id);
        }
    }

    if (!entity->surface) {
        return __lua_error_msg(state, "SDL_QuadGradient(): Entity surface not found");
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }

    if (SDL_MUSTLOCK(entity->surface)) {
        if (SDL_LockSurface(entity->surface) != 0) {
            return __lua_error_msg(state, "SDL_QuadGradient(): %s", SDL_GetError());
        }
    }

    Uint32 *pixels = (Uint32 *) entity->surface->pixels;
    int pitch = entity->surface->pitch / sizeof(Uint32);
    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(entity->surface->format);

    for (int row = 0; row < (int) area.h; row++) {
        float v = (float) row / ((float) area.h - 1.0f);

        for (int col = 0; col < (int) area.w; col++) {
            float u = (float) col / ((float) area.w - 1.0f);

            Uint8 r = (Uint8)(
                top_left.r * (1 - u) * (1 - v) +
                top_right.r * u * (1 - v) +
                bottom_left.r * (1 - u) * v +
                bottom_right.r * u * v
            );

            Uint8 g = (Uint8)(
                top_left.g * (1 - u) * (1 - v) +
                top_right.g * u * (1 - v) +
                bottom_left.g * (1 - u) * v +
                bottom_right.g * u * v
            );

            Uint8 b = (Uint8)(
                top_left.b * (1 - u) * (1 - v) +
                top_right.b * u * (1 - v) +
                bottom_left.b * (1 - u) * v +
                bottom_right.b * u * v
            );

            Uint8 a = (Uint8)(
                top_left.a * (1 - u) * (1 - v) +
                top_right.a * u * (1 - v) +
                bottom_left.a * (1 - u) * v +
                bottom_right.a * u * v
            );

            Uint32 pixel_color = SDL_MapRGBA(fmt, NULL, r, g, b, a);

            int px = col;
            int py = row;
            if ((px >= 0) && (px < entity->surface->w) && (py >= 0) && (py < entity->surface->h)) {
                pixels[py * pitch + px] = pixel_color;
            }
        }
    }

    if (SDL_MUSTLOCK(entity->surface)) {
        SDL_UnlockSurface(entity->surface);
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_pushsurface(
    lua_State                   *state
) {
    char err_msg[SDL_ERR_LEN] = "";
    char *err = NULL;

    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (lua_gettop(state) < 2) {
        lua_pushstring(state, __sdl_error_msg("SDL_Pushsurface(): At least 2 parameters expected\n"));
        return 1;
    }

    if (lua_type(state, 1) != LUA_TSTRING) {
        lua_pushstring(state, __sdl_error_msg("SDL_Pushsurface(): String expected for first parameter\n"));
        return 1;
    }
    if (lua_type(state, 2) != LUA_TSTRING) {
        lua_pushstring(state, __sdl_error_msg("SDL_Pushsurface(): String expected for second parameter\n"));
        return 1;
    }

    int stack_size = 100;

    char *stack_id = (char *) lua_tostring(state, 1);
    char *entity_id = (char *) lua_tostring(state, 2);

    SDL_Entity *entity;
    HASH_FIND_STR(app->hash, entity_id, entity);

    SDL_Stack *stack;
    HASH_FIND_STR(app->stacks.hash, stack_id, stack);

    if (! entity) {
        lua_pushstring(state, __sdl_error_msg("SDL_Pushsurface(): Entity \"%s\" doesn't exist\n", entity_id));
        return 1;
    }

    if (lua_gettop(state) > 2) {
        if (lua_type(state, 3) != LUA_TNUMBER) {
            lua_pushstring(state, __sdl_error_msg("SDL_Pushsurface(): Integer expected for third parameter\n"));
            return 1;
        }

        stack_size = lua_tointeger(state, 3);
        if (stack_size < 1) {
            stack_size = 100;
        }
    }

    if (! stack) {
        if ((err = __sdl_stack_new(&app->stacks, stack_id, stack_size)) != NULL) {
            lua_pushstring(state, __sdl_error_msg("SDL_Pushsurface(): %s\n", err));
            return 1;
        }

        HASH_FIND_STR(app->stacks.hash, stack_id, stack);

        if (! stack) {
            lua_pushfstring(state, "SDL_Pushsurface(): Error creating stack \"%s\"\n", stack_id);
            return 1;
        }
    }

    if ((err = __sdl_stack_push(&app->stacks, stack_id, entity->surface)) != NULL) {
        lua_pushstring(state, __sdl_error_msg("SDL_Pushsurface(): %s\n", err));
        return 1;
    }

    if (app->log) {
        fprintf(app->log, ">>> Stacking surface \"%s\" in stack \"%s\"\n", entity_id, stack_id);
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_prevsurface(
    lua_State                   *state
) {
    char err_msg[SDL_ERR_LEN];
    char *err = NULL;

    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (lua_gettop(state) != 2) {
        lua_pushstring(state, __sdl_error_msg("SDL_Prevsurface(): Exactly 2 parameters expected\n"));
        return 1;
    }

    if (lua_type(state, 1) != LUA_TSTRING) {
        lua_pushstring(state, __sdl_error_msg("SDL_Prevsurface(): String expected for first parameter\n"));
        return 1;
    }
    if (lua_type(state, 2) != LUA_TSTRING) {
        lua_pushstring(state, __sdl_error_msg("SDL_Prevsurface(): String expected for second parameter\n"));
        return 1;
    }

    char *stack_id = (char *) lua_tostring(state, 1);
    char *entity_id = (char *) lua_tostring(state, 2);

    SDL_Entity *entity;
    HASH_FIND_STR(app->hash, entity_id, entity);

    SDL_Stack *stack;
    HASH_FIND_STR(app->stacks.hash, stack_id, stack);

    if (! entity) {
        lua_pushstring(state, __sdl_error_msg("SDL_Prevsurface(): Entity \"%s\" doesn't exist\n", entity_id));
        return 1;
    }
    if (! stack) {
        lua_pushstring(state, __sdl_error_msg("SDL_Prevsurface(): Stack \"%s\" doesn't exist\n", stack_id));
        return 1;
    }

    SDL_Surface *surface = __sdl_stack_prev(&app->stacks, stack_id, &err_msg[0]);

    if (! surface) {
        if (err_msg[0] != '\0') {
            lua_pushstring(state, err_msg);
        }
        else {
            lua_pushnil(state);
        }

        return 1;
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }
    if (entity->surface) {
        SDL_DestroySurface(entity->surface);
        entity->surface = NULL;
    }

    entity->surface = __sdl_copy_surface(surface, &err_msg[0]);

    if (! entity->surface) {
        if (err_msg[0] != '\0') {
            lua_pushstring(state, err_msg);
        }
        else {
            lua_pushnil(state);
        }

        return 1;
    }

    if (app->log) {
        fprintf(app->log, ">>> Returning previous surface \"%s\" in stack \"%s\"\n", entity_id, stack_id);
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_nextsurface(
    lua_State                   *state
) {
    char err_msg[SDL_ERR_LEN];
    char *err = NULL;

    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (lua_gettop(state) != 2) {
        lua_pushstring(state, __sdl_error_msg("SDL_Nextsurface(): Exactly 2 parameters expected\n"));
        return 1;
    }

    if (lua_type(state, 1) != LUA_TSTRING) {
        lua_pushstring(state, __sdl_error_msg("SDL_Nextsurface(): String expected for first parameter\n"));
        return 1;
    }
    if (lua_type(state, 2) != LUA_TSTRING) {
        lua_pushstring(state, __sdl_error_msg("SDL_Nextsurface(): String expected for second parameter\n"));
        return 1;
    }

    char *stack_id = (char *) lua_tostring(state, 1);
    char *entity_id = (char *) lua_tostring(state, 2);

    SDL_Entity *entity;
    HASH_FIND_STR(app->hash, entity_id, entity);

    SDL_Stack *stack;
    HASH_FIND_STR(app->stacks.hash, stack_id, stack);

    if (! entity) {
        lua_pushstring(state, __sdl_error_msg("SDL_Nextsurface(): Entity \"%s\" doesn't exist\n", entity_id));
        return 1;
    }
    if (! stack) {
        lua_pushstring(state, __sdl_error_msg("SDL_Nextsurface(): Stack \"%s\" doesn't exist\n", stack_id));
        return 1;
    }

    SDL_Surface *surface = __sdl_stack_next(&app->stacks, stack_id, &err_msg[0]);

    if (! surface) {
        if (err_msg[0] != '\0') {
            lua_pushstring(state, err_msg);
        }
        else {
            lua_pushnil(state);
        }

        return 1;
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }
    if (entity->surface) {
        SDL_DestroySurface(entity->surface);
        entity->surface = NULL;
    }

    entity->surface = __sdl_copy_surface(surface, &err_msg[0]);

    if (! entity->surface) {
        if (err_msg[0] != '\0') {
            lua_pushstring(state, err_msg);
        }
        else {
            lua_pushnil(state);
        }

        return 1;
    }

    if (app->log) {
        fprintf(app->log, ">>> Returning next surface \"%s\" in stack \"%s\"\n", entity_id, stack_id);
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_flushsurfaces(
    lua_State                   *state
) {
    char err_msg[SDL_ERR_LEN];
    char *err = NULL;

    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    if (lua_gettop(state) != 1) {
        lua_pushstring(state, __sdl_error_msg("SDL_Flushsurfaces(): Exactly 2 parameters expected\n"));
        return 1;
    }

    if (lua_type(state, 1) != LUA_TSTRING) {
        lua_pushstring(state, __sdl_error_msg("SDL_Flushsurfaces(): String expected for first parameter\n"));
        return 1;
    }

    char *stack_id = (char *) lua_tostring(state, 1);

    SDL_Stack *stack;
    HASH_FIND_STR(app->stacks.hash, stack_id, stack);

    if (! stack) {
        lua_pushstring(state, __sdl_error_msg("SDL_Flushsurfaces(): Stack \"%s\" doesn't exist\n", stack_id));
        return 1;
    }

    if ((err = __sdl_stack_clear(&app->stacks, stack_id)) != NULL) {
        lua_pushstring(state, err);
        return 1;
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_put_pixel(
    lua_State                   *state
) {
    char err_msg[LUA_ERR_LEN];
    char *err;

    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    SDL_Entity *entity = NULL;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Putpixel(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Putpixel(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Putpixel(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Putpixel", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Putpixel(): Entity \"%s\" not found\n", entity_id);
    }

    int x, y;
    const char *result = __lua_table_get_integer(state, "SDL_Putpixel()", 1, "x", &x);
    if (! result) {
        return 1;
    }
    result = __lua_table_get_integer(state, "SDL_Putpixel()", 1, "y", &y);
    if (! result) {
        return 1;
    }

    SDL_Color rgba;
    if ((result = __lua_table_get_rgba(state, "SDL_Putpixel()", 1, &rgba)) == NULL) {
        return 1;
    }

    if (entity->surface) {
        err = __sdl_surface_put_pixel(entity->surface, x, y, rgba.r, rgba.g, rgba.b, rgba.a);
        if (err) {
            lua_pushstring(state, err);
            return 1;
        }
        fprintf(stdout, "UPDATING SURFACE...\n\n");
    }

    fprintf(stdout, "Putpixel %d,%d,%d,%d\n",
        (int) rgba.r,
        (int) rgba.g,
        (int) rgba.b,
        (int) rgba.a
    );
    fflush(stdout);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_get_pixel(
    lua_State                   *state
) {
    char err_msg[LUA_ERR_LEN];
    char *err;

    APP *app = (APP *) (*(void **) lua_getextraspace(state));
    SDL_Entity *entity = NULL;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Getpixel(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Getpixel(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Getpixel(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Getpixel()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Getpixel(): Entity \"%s\" not found\n", entity_id);
    }

    int x, y;
    char *result = __lua_table_get_integer(state, "SDL_Getpixel()", 1, "x", &x);
    if (! result) {
        return 1;
    }

    result = __lua_table_get_integer(state, "SDL_Getpixel()", 1, "y", &y);
    if (! result) {
        return 1;
    }

    uint8_t red, green, blue, alpha;

    if (entity->surface) {
        err = __sdl_surface_get_pixel(entity->surface, x, y, &red, &green, &blue, &alpha);
        if (err) {
            return __lua_error_msg(state, err);
        }
    }

    lua_newtable(state);

    lua_pushinteger(state, red);
    lua_setfield(state, -2, "red");
    lua_pushinteger(state, green);
    lua_setfield(state, -2, "green");
    lua_pushinteger(state, blue);
    lua_setfield(state, -2, "blue");
    lua_pushinteger(state, alpha);
    lua_setfield(state, -2, "alpha");

    return 1;
}

/**
 *
 */
int l_sdl_drawline(lua_State *state) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));
    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (!(app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_DrawLine(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_DrawLine(): Exactly 1 parameter expected");
    }

    if (!lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_DrawLine(): Table expected for first parameter");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_DrawLine()", 1, "id");
    if (!entity_id) {
        return 1;
    }

    int start_x, start_y, end_x, end_y, weight = 1;
    const char *result;

    result = __lua_table_get_integer(state, "SDL_DrawLine()", 1, "start_x", &start_x);
    if (!result) {
        return 1;
    }
    result = __lua_table_get_integer(state, "SDL_DrawLine()", 1, "start_y", &start_y);
    if (!result) {
        return 1;
    }
    result = __lua_table_get_integer(state, "SDL_DrawLine()", 1, "end_x", &end_x);
    if (!result) {
        return 1;
    }
    result = __lua_table_get_integer(state, "SDL_DrawLine()", 1, "end_y", &end_y);
    if (!result) {
        return 1;
    }

    result = __lua_table_get_integer(state, "SDL_DrawLine()", 1, "weight", &weight);
    if (!result) {
        weight = 1;
    }

    SDL_Color rgba;
    if (!__lua_table_get_rgba(state, "SDL_DrawLine()", 1, &rgba)) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);
    if (!entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (!entity) {
            return __lua_error_msg(state, "SDL_DrawLine(): Entity \"%s\" not found", entity_id);
        }
    }

    if (!entity->surface) {
        return __lua_error_msg(state, "SDL_DrawLine(): Entity surface not found");
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
    }

    if (SDL_MUSTLOCK(entity->surface)) {
        if (SDL_LockSurface(entity->surface) != 0) {
            return __lua_error_msg(state, "SDL_DrawLine(): %s", SDL_GetError());
        }
    }

    Uint32 *pixels = (Uint32 *) entity->surface->pixels;
    int pitch = entity->surface->pitch / sizeof(Uint32);
    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(entity->surface->format);
    Uint32 pixel_color = SDL_MapRGBA(fmt, NULL, rgba.r, rgba.g, rgba.b, rgba.a);

    int dx = abs(end_x - start_x);
    int sx = start_x < end_x ? 1 : -1;
    int dy = -abs(end_y - start_y);
    int sy = start_y < end_y ? 1 : -1;
    int line_err = dx + dy;

    while (1) {
        for (int wx = -weight/2; wx <= weight/2; wx++) {
            for (int wy = -weight/2; wy <= weight/2; wy++) {
                int px = start_x + wx;
                int py = start_y + wy;
                if ((px >= 0) && (px < entity->surface->w) && (py >= 0) && (py < entity->surface->h)) {
                    pixels[py * pitch + px] = pixel_color;
                }
            }
        }

        if (start_x == end_x && start_y == end_y) break;
        int e2 = 2 * line_err;
        if (e2 >= dy) {
            line_err += dy;
            start_x += sx;
        }
        if (e2 <= dx) {
            line_err += dx;
            start_y += sy;
        }
    }

    if (SDL_MUSTLOCK(entity->surface)) {
        SDL_UnlockSurface(entity->surface);
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_rectangle(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Rectangle(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Rectangle(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Rectangle(): Table expected for first parameter\n");
    }

    char *entity_id = (char *) __lua_table_get_string(state, "SDL_Rectangle()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    SDL_FRect area;
    char *result = __lua_table_get_area(state, "SDL_Rectabgle()", 1, &area);

    if (! result) {
        return 1;
    }

    SDL_Color rgba;

    if ((result = __lua_table_get_rgba(state, "SDL_Rectangle()", 1, &rgba)) == NULL) {
        return 1;
    }

    if (strcmp(entity_id, "") != 0) {
        HASH_FIND_STR(app->hash, entity_id, entity);

        if (! entity) {
            snprintf(err_msg, SDL_ERR_LEN, "SDL_Rectangle(): Entity \"%s\" doesn\'t exist\n", entity_id);
            lua_pushstring(state, err_msg);
            return 1;
        }

        const SDL_PixelFormatDetails* formatDetails = SDL_GetPixelFormatDetails(entity->surface->format);
        
        SDL_Rect rect;
        rect.x = (int) area.x;
        rect.y = (int) area.y;
        rect.w = (int) area.w;
        rect.h = (int) area.h;

        Uint32 pixel_color = SDL_MapRGBA(formatDetails, NULL, rgba.r, rgba.g, rgba.b, rgba.a);
        SDL_FillSurfaceRect(entity->surface, &rect, pixel_color);
    }
    else {
        SDL_SetRenderDrawColor(app->renderer, rgba.r, rgba.g, rgba.b, rgba.a);
        SDL_RenderFillRect(app->renderer, &area);
    }

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_circle(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Circle(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Circle(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Circle(): Table expected for first parameter\n");
    }

    char *entity_id = (char *) __lua_table_get_string(state, "SDL_Circle()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        snprintf(err_msg, SDL_ERR_LEN, "SDL_Circle(): Entity \"%s\" doesn\'t exist\n", entity_id);
        lua_pushstring(state, err_msg);
        return 1;
    }

    int x, y, diameter;

    char *result = __lua_table_get_integer(state, "SDL_Circle()", 1, "x", &x);
    if (! result) {
        return 1;
    }
    if ((result = __lua_table_get_integer(state, "SDL_Circle()", 1, "y", &y)) == NULL) {
        return 1;
    }
    if ((result = __lua_table_get_integer(state, "SDL_Circle()", 1, "diameter", &diameter)) == NULL) {
        return 1;
    }

    SDL_Color rgba;

    if ((result = __lua_table_get_rgba(state, "SDL_Circle()", 1, &rgba)) == NULL) {
        return 1;
    }

    __sdl_surface_fill_circle(
        entity->surface,
        x,
        y,
        diameter,
        rgba.r,
        rgba.g,
        rgba.b,
        rgba.a
    );

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
        return __lua_error_msg(state, "SDL_Texture(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Texture(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Texture(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Texture()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    SDL_FRect area;
    char *result = __lua_table_get_area(state, "SDL_Texture()", 1, &area);
    if (! result) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            return __lua_error_msg(state, "Error in SDL_Texture(): Entity \"%s\" not found\n", entity_id);
        }
        entity->surface = SDL_CreateSurface(area.w, area.h, SDL_PIXELFORMAT_ARGB8888);
        if (! entity->surface) {
            return __lua_error_msg(state, "SDL_Texture(): %s\n", SDL_GetError());
        }
    }

    if (entity->texture) {
        SDL_DestroyTexture(entity->texture);
        entity->texture = NULL;
        if (app->log) {
            fprintf(app->log, ">>> Destroyed existing texture %s\n", entity_id);
        }
    }

    if ((entity->texture = SDL_CreateTextureFromSurface(app->renderer, entity->surface)) == NULL) {
        return __lua_error_msg(state, "SDL_Texture(): %s\n", SDL_GetError());
    }

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &area);
    memcpy(&entity->area, &area, (sizeof(SDL_FRect)));

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL texture %s @ %dx%d (%dx%d)\n",
            entity_id,
            (int) area.x,
            (int) area.y,
            (int) area.w,
            (int) area.h
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
        return __lua_error_msg(state, "SDL_Image(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Image(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Image(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Image()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    const char *entity_path = __lua_table_get_string(state, "SDL_Image()", 1, "image");
    if (! entity_path) {
        return 1;
    }

    SDL_FRect area;
    char *result = __lua_table_get_area(state, "SDL_Image()", 1, &area);
    if (! result) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            return __lua_error_msg(state, "SDL_Image(): Entity \"%s\" not found\n", entity_id);
        }
        entity->surface = SDL_CreateSurface(area.w, area.h, SDL_PIXELFORMAT_ARGB8888);
        if (! entity->surface) {
            return __lua_error_msg(state, "SDL_Image(): %s\n", SDL_GetError());
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
        return __lua_error_msg(state, "SDL_Image(): %s\n", SDL_GetError());
    }

    if ((entity->texture = SDL_CreateTextureFromSurface(app->renderer, entity->surface)) == NULL) {
        return __lua_error_msg(state, "SDL_Image(): %s\n", SDL_GetError());
    }

    if (area.w < 1.0f) {
        area.w = (float) entity->texture->w;
    }
    if (area.h < 1.0f) {
        area.h = (float) entity->texture->h;
    }

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &area);
    memcpy(&entity->area, &area, (sizeof(SDL_FRect)));

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL image %s @ %dx%d (%dx%d)\n",
            entity_id,
            area.x,
            area.y,
            area.w,
            area.h
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
        return __lua_error_msg(state, "SDL_Text(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Text(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Text(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Text()", 1, "id");
    if (! entity_id) {
        return 1;
    }
    const char *entity_path = __lua_table_get_string(state, "SDL_Text()", 1, "font");
    if (! entity_path) {
        return 1;
    }
    int entity_size;
    const char *result = __lua_table_get_integer(state, "SDL_Text()", 1, "size", &entity_size);
    if (! result) {
        return 1;
    }
    const char *entity_text = __lua_table_get_string(state, "SDL_Text()", 1, "text");
    if (! entity_text) {
        return 1;
    }
    int wrap;
    result = __lua_table_get_integer(state, "SDL_Text()", 1, "wrap", &wrap);
    if (! result) {
        wrap = 0;
    }

    SDL_FRect area;
    if ((result = __lua_table_get_area(state, "SDL_Text()", 1, &area)) == NULL) {
        return 1;
    }

    SDL_Color rgba;
    if ((result = __lua_table_get_rgba(state, "SDL_Text()", 1, &rgba)) == NULL) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            return __lua_error_msg(state, "SDL_Text(): Entity \"%s\" not found\n", entity_id);
        }
        entity->surface = SDL_CreateSurface(area.w, area.h, SDL_PIXELFORMAT_ARGB8888);
        if (! entity->surface) {
            return __lua_error_msg(state, "SDL_Text(): %s\n", SDL_GetError());
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
            return __lua_error_msg(state, "SDL_Text(): %s\n", SDL_GetError());
        }
        app->flags |= APP_F_TTFINIT;
    }

    TTF_Font *font = TTF_OpenFont(entity_path, entity_size);
    
    if (! font) {
        return __lua_error_msg(state, "%s", SDL_GetError());
    }

    if (wrap > 0) {
        entity->surface = TTF_RenderText_Blended_Wrapped(font, entity_text, strlen(entity_text), rgba, wrap);
    }
    else {
        entity->surface = TTF_RenderText_Blended(font, entity_text, strlen(entity_text), rgba);
    }

    if (! entity->surface) {
        return __lua_error_msg(state, "%s", SDL_GetError());
    }

    entity->texture = SDL_CreateTextureFromSurface(app->renderer, entity->surface);

    if (! entity->texture) {
        return __lua_error_msg(state, "%s", SDL_GetError());
    }

    if (area.w < 1.0f) {
        area.w = (float) entity->texture->w;
    }
    if (area.h < 1.0f) {
        area.h = (float) entity->texture->h;
    }

    SDL_Rect rect;

    rect.x = (int) area.x;
    rect.y = (int) area.y;
    rect.w = (int) area.w;
    rect.h = (int) area.h;

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &area);

    memcpy(&entity->area, &area, (sizeof(SDL_FRect)));

    if (app->log) {
        fprintf(app->log, ">>> Created new SDL text %s @ %dx%d (%dx%d), font=\"%s\", size=%d, text=\"%s\"\n",
            entity_id,
            (int) area.x,
            (int) area.y,
            (int) area.w,
            (int) area.h,
            entity_path,
            entity_size,
            entity_text
        );
    }

    if ((err = entity_set_text(entity, entity_path, entity_text, entity_size, rgba)) != NULL) {
        return __lua_error_msg(state, err);
    }

    TTF_CloseFont(font);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_cliprect(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    SDL_Rect rect;
    SDL_Rect *clip_rect = &rect;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Cliprect(): SDL not initialised");
    }

    if (lua_gettop(state) < 1) {
        clip_rect = NULL;
    }
    else {
        if (! lua_istable(state, 1)) {
            return __lua_error_msg(state, "SDL_Cliprect(): Table expected for first parameter\n");
        }
    }

    SDL_FRect area;
    char *result;

    if (clip_rect) {
        if ((result = __lua_table_get_area(state, "SDL_Cliprect()", 1, &area)) == NULL) {
            return 1;
        }

        clip_rect->x = (int) area.x;
        clip_rect->y = (int) area.y;
        clip_rect->w = (int) area.w;
        clip_rect->h = (int) area.h;
    }

    if (app->log) {
        if (clip_rect) {
            fprintf(stdout, ">>> Enabling clipping area @ x=%d, y=%d (width=%d, height=%d)\n", rect.x, rect.y, rect.w, rect.h);
        }
        else {
            fprintf(stdout, ">>> Dsabling clipping area\n");
        }
    }

    SDL_SetRenderClipRect(app->renderer, clip_rect);

    lua_pushstring(state, "OK");
    return 1;
}

/**
 *
 */
int l_sdl_textarea(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Textarea(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Textarea(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Textarea(): Table expected for first parameter\n");
    }

    const char *entity_path = __lua_table_get_string(state, "SDL_Textarea()", 1, "font");
    if (! entity_path) {
        return 1;
    }
    int entity_size;
    const char *result = __lua_table_get_integer(state, "SDL_Textarea()", 1, "size", &entity_size);
    if (! result) {
        return 1;
    }
    const char *entity_text = __lua_table_get_string(state, "SDL_Textarea()", 1, "text");
    if (! entity_text) {
        return 1;
    }

    if (! (app->flags & APP_F_TTFINIT)) {
        if (app->log) {
            fprintf(app->log, ">>> Initialising TTF subsystem\n");
        }
        if (! TTF_Init()) {
            return __lua_error_msg(state, "SDL_Textarea(): %s\n", SDL_GetError());
        }
        app->flags |= APP_F_TTFINIT;
    }

    TTF_Font *font = TTF_OpenFont(entity_path, entity_size);

    if (! font) {
        return __lua_error_msg(state, "SDL_Textarea(): %s", SDL_GetError());
    }

    int text_width, text_height;

    if (! TTF_GetStringSize(font, entity_text, 0, &text_width, &text_height)) {
        TTF_CloseFont(font);
        return __lua_error_msg(state, "%s", SDL_GetError());
    }

    TTF_CloseFont(font);

    lua_newtable(state);
    lua_pushstring(state, "width");
    lua_pushinteger(state, text_width);

    lua_settable(state, -3);
    lua_pushstring(state, "height");
    lua_pushinteger(state, text_height);

    lua_settable(state, -3);

    return 1;
}

/**
 *
 */
int l_sdl_surface_info(
    lua_State                   *state
) {
    APP *app = (APP *) (*(void **) lua_getextraspace(state));

    char *err;
    char err_msg[LUA_ERR_LEN];

    SDL_Entity *entity;

    if (! (app->flags & APP_F_SDLINIT)) {
        return __lua_error_msg(state, "SDL_Surfaceinfo(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Surfaceinfo(): Exactly 1 parameter expected\n");
    }
    if (! lua_isstring(state, 1)) {
        return __lua_error_msg(state, "SDL_Surfaceinfo(): String expected for first parameter\n");
    }

    const char *entity_id = lua_tostring(state, -1);
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Surfaceinfo(): Entity \"%s\" not found\n", entity_id);
    }

    if (entity->type != SDL_ENTITY_IMAGE && entity->type != SDL_ENTITY_FONT) {
        return __lua_error_msg(state, "SDL_Surfaceinfo(): Entity \"%s\" is not a valid surface or texture\n", entity_id);
    }

    int entity_x, entity_y, entity_width, entity_height;

    entity_x = (int) entity->area.x;
    entity_y = (int) entity->area.y;
    entity_width = (int) entity->area.w;
    entity_height = (int) entity->area.h;

    lua_newtable(state);

    lua_pushnumber(state, entity_x);
    lua_setfield(state, -2, "x");

    lua_pushnumber(state, entity_y);
    lua_setfield(state, -2, "y");

    lua_pushnumber(state, entity_width);
    lua_setfield(state, -2, "width");

    lua_pushnumber(state, entity_height);
    lua_setfield(state, -2, "height");

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
        return __lua_error_msg(state, "SDL_Update(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Update(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Update(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Update()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Update(): Entity \"%s\" not found\n", entity_id);
    }

    SDL_FRect area;
    char *result = __lua_table_get_area(state, "SDL_Update()", 1, &area);
    if (! result) {
        return 1;
    }

    memcpy(&entity->area, &area, (sizeof(SDL_FRect)));

    if (app->log) {
        fprintf(app->log, ">>> Update to %s @ %dx%d (%dx%d): red=%d, green=%d, blue=%d, alpha=%d\n",
            entity_id,
            (int) area.x,
            (int) area.y,
            (int) area.w,
            (int) area.h,
            255, 255, 255, 255
        );
    }

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
        return __lua_error_msg(state, "SDL_Audio(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Audio(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Audio(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Audio()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    const char *entity_path = __lua_table_get_string(state, "SDL_Audio()", 1, "audio");
    if (! entity_path) {
        return 1;
    }

    bool autoplay;
    const char *result = __lua_table_get_boolean(state, "SDL_Audio()", 1, "autoplay", &autoplay);
    if (! result) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            return __lua_error_msg(state, "SDL_Audio(): Entity \"%s\" not found\n", entity_id);
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
            return __lua_error_msg(state, "SDL_Audio(): Error initialising audio engine\n");
        }
        if (app->log) { 
            fprintf(app->log, ">>> Initialised audio engine\n");
        }
        app->flags |= APP_F_AUDIOINIT;
    }

    entity->decoder = (ma_decoder){0};

    if (ma_decoder_init_file(entity_path, NULL, &entity->decoder) != MA_SUCCESS) {
        return __lua_error_msg(state, "SDL_Audio(): Failed to init decoder: %s\n", entity_path);
    }

    if (autoplay) {
        if (ma_sound_init_from_data_source(&app->engine, &entity->decoder, 0, NULL, &entity->sound) != MA_SUCCESS) {
            return __lua_error_msg(state, "SDL_Audio(): Failed to init sound from decoder: %s\n", entity_path);
        }

        if (ma_engine_start(&app->engine) != MA_SUCCESS) {
            return __lua_error_msg(state, "SDL_Audio(): Failed to start audio engine\n");
        }

        ma_sound_set_volume(&entity->sound, 1.0f);
        ma_engine_set_volume(&app->engine, 1.0f);

        ma_sound_start(&entity->sound);
    }

    if ((err = entity_set_audio(entity, entity_path)) != NULL) {
        return __lua_error_msg(state, err);
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
        return __lua_error_msg(state, "SDL_Play(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Play(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Play(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Play()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            return __lua_error_msg(state, "SDL_Play(): Entity \"%s\" not found\n", entity_id);
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
        return __lua_error_msg(state, "SDL_Play(): %s\n", ma_result_description(result));
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
        return __lua_error_msg(state, "SDL_Audioinfo(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Audioinfo(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Audioinfo(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "ASL_Audioinfo()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            return __lua_error_msg(state, err);
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            return __lua_error_msg(state, "SDL_Pause(): Entity \"%s\" not found\n", entity_id);
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
        return __lua_error_msg(state, "SDL_Pause(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Pause(): Exactly 1 parameter expected\n");
    }
    if (! lua_istable(state, 1)) {
        return __lua_error_msg(state, "SDL_Pause(): Table expected for first parameter\n");
    }

    const char *entity_id = __lua_table_get_string(state, "SDL_Pause()", 1, "id");
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        if ((err = __sdl_entity(app, entity_id)) != NULL) {
            lua_pushstring(state, err);
            return 1;
        }
        HASH_FIND_STR(app->hash, entity_id, entity);
        if (! entity) {
            return __lua_error_msg(state, "SDL_Pause(): Entity \"%s\" not found\n", entity_id);
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
        return __lua_error_msg(state, "SDL_Pause(): %s\n", ma_result_description(result));
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
        return __lua_error_msg(state, "SDL_Render(): SDL not initialised");
    }

    if (lua_gettop(state) != 1) {
        return __lua_error_msg(state, "SDL_Render(): Exactly 1 parameter expected\n");
    }
    if (lua_type(state, 1) != LUA_TSTRING) {
        return __lua_error_msg(state, "SDL_Render(): String expected for first parameter\n");
    }

    const char *entity_id = lua_tostring(state, 1);
    if (! entity_id) {
        return 1;
    }

    HASH_FIND_STR(app->hash, entity_id, entity);

    if (! entity) {
        return __lua_error_msg(state, "SDL_Texture(): Entity \"%s\" not found\n", entity_id);
    }

    if (app->log) {
        fprintf(app->log, "Rendering texture %s @ %dx%d (%dx%d)\n",
            entity_id,
            (int) entity->area.x,
            (int) entity->area.y,
            (int) entity->area.w,
            (int) entity->area.h
        );
    }

    SDL_RenderTexture(app->renderer, entity->texture, NULL, &entity->area);
    SDL_FlushRenderer(app->renderer);

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
        return __lua_error_msg(state, "SDL_Clear(): SDL not initialised");
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
        return __lua_error_msg(state, "SDL_Present(): SDL not initialised");
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

/**
 *
 */
void reg_sdl_cursor_types(
    lua_State                   *state
) {
    lua_pushinteger(state, SDL_SYSTEM_CURSOR_DEFAULT);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_DEFAULT");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_TEXT);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_TEXT");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_WAIT);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_WAIT");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_CROSSHAIR);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_CROSSHAIR");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_PROGRESS);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_PROGRESS");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_NWSE_RESIZE);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_NWSE_RESIZE");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_NESW_RESIZE);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_NESW_RESIZE");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_EW_RESIZE);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_EW_RESIZE");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_NS_RESIZE);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_NS_RESIZE");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_MOVE);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_MOVE");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_NOT_ALLOWED);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_NOT_ALLOWED");

    lua_pushinteger(state, SDL_SYSTEM_CURSOR_POINTER);
    lua_setglobal(state, "SDL_SYSTEM_CURSOR_POINTER");
}

void reg_sdl_keycodes(
    lua_State                   *state
) {
    lua_pushinteger(state, SDLK_RETURN);
    lua_setglobal(state, "SDLK_RETURN");

    lua_pushinteger(state, SDLK_ESCAPE);
    lua_setglobal(state, "SDLK_ESCAPE");

    lua_pushinteger(state, SDLK_BACKSPACE);
    lua_setglobal(state, "SDLK_BACKSPACE");

    lua_pushinteger(state, SDLK_TAB);
    lua_setglobal(state, "SDLK_TAB");

    lua_pushinteger(state, SDLK_SPACE);
    lua_setglobal(state, "SDLK_SPACE");

    lua_pushinteger(state, SDLK_DELETE);
    lua_setglobal(state, "SDLK_DELETE");

    lua_pushinteger(state, SDLK_INSERT);
    lua_setglobal(state, "SDLK_INSERT");

    lua_pushinteger(state, SDLK_HOME);
    lua_setglobal(state, "SDLK_HOME");

    lua_pushinteger(state, SDLK_END);
    lua_setglobal(state, "SDLK_END");

    lua_pushinteger(state, SDLK_PAGEUP);
    lua_setglobal(state, "SDLK_PAGEUP");

    lua_pushinteger(state, SDLK_PAGEDOWN);
    lua_setglobal(state, "SDLK_PAGEDOWN");

    // Arrow keys
    lua_pushinteger(state, SDLK_LEFT);
    lua_setglobal(state, "SDLK_LEFT");

    lua_pushinteger(state, SDLK_RIGHT);
    lua_setglobal(state, "SDLK_RIGHT");

    lua_pushinteger(state, SDLK_UP);
    lua_setglobal(state, "SDLK_UP");

    lua_pushinteger(state, SDLK_DOWN);
    lua_setglobal(state, "SDLK_DOWN");

    // Function keys
    lua_pushinteger(state, SDLK_F1);
    lua_setglobal(state, "SDLK_F1");

    lua_pushinteger(state, SDLK_F2);
    lua_setglobal(state, "SDLK_F2");

    lua_pushinteger(state, SDLK_F3);
    lua_setglobal(state, "SDLK_F3");

    lua_pushinteger(state, SDLK_F4);
    lua_setglobal(state, "SDLK_F4");

    lua_pushinteger(state, SDLK_F5);
    lua_setglobal(state, "SDLK_F5");

    lua_pushinteger(state, SDLK_F6);
    lua_setglobal(state, "SDLK_F6");

    lua_pushinteger(state, SDLK_F7);
    lua_setglobal(state, "SDLK_F7");

    lua_pushinteger(state, SDLK_F8);
    lua_setglobal(state, "SDLK_F8");

    lua_pushinteger(state, SDLK_F9);
    lua_setglobal(state, "SDLK_F9");

    lua_pushinteger(state, SDLK_F10);
    lua_setglobal(state, "SDLK_F10");

    lua_pushinteger(state, SDLK_F11);
    lua_setglobal(state, "SDLK_F11");

    lua_pushinteger(state, SDLK_F12);
    lua_setglobal(state, "SDLK_F12");

    // Modifier keys (optional)
    lua_pushinteger(state, SDLK_LCTRL);
    lua_setglobal(state, "SDLK_LCTRL");

    lua_pushinteger(state, SDLK_RCTRL);
    lua_setglobal(state, "SDLK_RCTRL");

    lua_pushinteger(state, SDLK_LSHIFT);
    lua_setglobal(state, "SDLK_LSHIFT");

    lua_pushinteger(state, SDLK_RSHIFT);
    lua_setglobal(state, "SDLK_RSHIFT");

    lua_pushinteger(state, SDLK_LALT);
    lua_setglobal(state, "SDLK_LALT");

    lua_pushinteger(state, SDLK_RALT);
    lua_setglobal(state, "SDLK_RALT");

    lua_pushinteger(state, SDLK_LGUI);
    lua_setglobal(state, "SDLK_LGUI");

    lua_pushinteger(state, SDLK_RGUI);
    lua_setglobal(state, "SDLK_RGUI");

    // Alphabet/digits 0-9
    lua_pushinteger(state, SDLK_A);
    lua_setglobal(state, "SDLK_A");

    lua_pushinteger(state, SDLK_B);
    lua_setglobal(state, "SDLK_B");

    lua_pushinteger(state, SDLK_C);
    lua_setglobal(state, "SDLK_C");

    lua_pushinteger(state, SDLK_D);
    lua_setglobal(state, "SDLK_D");

    lua_pushinteger(state, SDLK_E);
    lua_setglobal(state, "SDLK_E");

    lua_pushinteger(state, SDLK_F);
    lua_setglobal(state, "SDLK_F");

    lua_pushinteger(state, SDLK_G);
    lua_setglobal(state, "SDLK_G");

    lua_pushinteger(state, SDLK_H);
    lua_setglobal(state, "SDLK_H");

    lua_pushinteger(state, SDLK_I);
    lua_setglobal(state, "SDLK_I");

    lua_pushinteger(state, SDLK_J);
    lua_setglobal(state, "SDLK_J");

    lua_pushinteger(state, SDLK_K);
    lua_setglobal(state, "SDLK_K");

    lua_pushinteger(state, SDLK_L);
    lua_setglobal(state, "SDLK_L");

    lua_pushinteger(state, SDLK_M);
    lua_setglobal(state, "SDLK_M");

    lua_pushinteger(state, SDLK_N);
    lua_setglobal(state, "SDLK_N");

    lua_pushinteger(state, SDLK_O);
    lua_setglobal(state, "SDLK_O");

    lua_pushinteger(state, SDLK_P);
    lua_setglobal(state, "SDLK_P");

    lua_pushinteger(state, SDLK_Q);
    lua_setglobal(state, "SDLK_Q");

    lua_pushinteger(state, SDLK_R);
    lua_setglobal(state, "SDLK_R");

    lua_pushinteger(state, SDLK_S);
    lua_setglobal(state, "SDLK_S");

    lua_pushinteger(state, SDLK_T);
    lua_setglobal(state, "SDLK_T");

    lua_pushinteger(state, SDLK_U);
    lua_setglobal(state, "SDLK_U");

    lua_pushinteger(state, SDLK_V);
    lua_setglobal(state, "SDLK_V");

    lua_pushinteger(state, SDLK_W);
    lua_setglobal(state, "SDLK_W");

    lua_pushinteger(state, SDLK_X);
    lua_setglobal(state, "SDLK_X");

    lua_pushinteger(state, SDLK_Y);
    lua_setglobal(state, "SDLK_Y");

    lua_pushinteger(state, SDLK_Z);
    lua_setglobal(state, "SDLK_Z");

    lua_pushinteger(state, SDLK_0);
    lua_setglobal(state, "SDLK_0");

    lua_pushinteger(state, SDLK_1);
    lua_setglobal(state, "SDLK_1");

    lua_pushinteger(state, SDLK_2);
    lua_setglobal(state, "SDLK_2");

    lua_pushinteger(state, SDLK_3);
    lua_setglobal(state, "SDLK_3");

    lua_pushinteger(state, SDLK_4);
    lua_setglobal(state, "SDLK_4");

    lua_pushinteger(state, SDLK_5);
    lua_setglobal(state, "SDLK_5");

    lua_pushinteger(state, SDLK_6);
    lua_setglobal(state, "SDLK_6");

    lua_pushinteger(state, SDLK_7);
    lua_setglobal(state, "SDLK_7");

    lua_pushinteger(state, SDLK_8);
    lua_setglobal(state, "SDLK_8");

    lua_pushinteger(state, SDLK_9);
    lua_setglobal(state, "SDLK_9");

}
