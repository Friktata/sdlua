#ifndef SDL_LIB_H
#define SDL_LIB_H

#include <stdarg.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#define LUA_ERR_LEN         1024

int     l_sdl_init          (lua_State *);
int     l_sdl_quit          (lua_State *);
int     l_sdl_event         (lua_State *);
int     l_sdl_poll          (lua_State *);
int     l_sdl_delay         (lua_State *);
int     l_sdl_screen_info   (lua_State *);
int     l_sdl_window_info   (lua_State *);
int     l_sdl_fullscreen    (lua_State *);
int     l_sdl_drawcolor     (lua_State *);
int     l_sdl_surface       (lua_State *);
int     l_sdl_put_pixel     (lua_State *);
int     l_sdl_get_pixel     (lua_State *);
int     l_sdl_fill_surface  (lua_State *);
int     l_sdl_texture       (lua_State *);
int     l_sdl_image         (lua_State *);
int     l_sdl_text          (lua_State *);
int     l_sdl_update        (lua_State *);
int     l_sdl_audio         (lua_State *);
int     l_sdl_audio_info    (lua_State *);
int     l_sdl_play          (lua_State *);
int     l_sdl_pause         (lua_State *);
int     l_sdl_render        (lua_State *);
int     l_sdl_clear         (lua_State *);
int     l_sdl_present       (lua_State *);

static const struct luaL_Reg sdl_lib[] = {
    { "SDL_Init",           l_sdl_init },
    { "SDL_Quit",           l_sdl_quit },
    { "SDL_Event",          l_sdl_event },
    { "SDL_Poll",           l_sdl_poll },
    { "SDL_Delay",          l_sdl_delay },
    { "SDL_Info",           l_sdl_screen_info },
    { "SDL_Windowinfo",     l_sdl_window_info },
    { "SDL_Fullscreen",     l_sdl_fullscreen },
    { "SDL_Drawcolor",      l_sdl_drawcolor },
    { "SDL_Surface",        l_sdl_surface },
    { "SDL_Putpixel",       l_sdl_put_pixel },
    { "SDL_Getpixel",       l_sdl_get_pixel },
    { "SDL_Fill",           l_sdl_fill_surface },
    { "SDL_Texture",        l_sdl_texture },
    { "SDL_Image",          l_sdl_image },
    { "SDL_Text",           l_sdl_text },
    { "SDL_Update",         l_sdl_update },
    { "SDL_Audio",          l_sdl_audio },
    { "SDL_Audioinfo",      l_sdl_audio_info },
    { "SDL_Play",           l_sdl_play },
    { "SDL_Pause",          l_sdl_pause },
    { "SDL_Render",         l_sdl_render },
    { "SDL_Clear",          l_sdl_clear },
    { "SDL_Present",        l_sdl_present },
    { NULL,                 NULL }
};

void    reg_sdl_init_flags  (lua_State *);
void    reg_sdl_win_flags   (lua_State *);

#endif
