-------------------------------------------------------------------------------
--  SDLua_draw/config/app.lua
--

    app                     = {

        flags               = (SDL_INIT_VIDEO),
        timeslice           = 16,
        error               = nil,

        state               = {
            running         = true,
            initialised     = false,
            fullscreen      = false
        },

        window              = {
            title           = "SDLua Draw",
            width           = 640,
            height          = 480,
            flags           = (SDL_WINDOW_RESIZABLE)
        },

        window_color        = {
            red             = 127,
            green           = 127,
            blue            = 220,
            alpha           = 255
        },

        font                = "/usr/share/fonts/TTF/Hack-Regular.ttf"

    }
