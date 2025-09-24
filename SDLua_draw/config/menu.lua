-------------------------------------------------------------------------------
--  SDLua_draw/config/menu.lua
--

    app_menu            = {

        id              = "app_menu",
        
        x               = 0,
        y               = 0,
        width           = 640,
        height          = 32,

        red             = 220,
        green           = 220,
        blue            = 220,
        alpha           = 220

    }

    app_menu_options    = {

        menu_quit       = {
            id          = "menu_quit",
            font        = app["font"],
            text        = "(Q)uit",
            size        = 14,
            x           = 0,
            y           = 0,
            width       = 0,
            height      = 0,
            red         = 0,
            green       = 0,
            blue        = 0,
            alpha       = 255
        },

        menu_save       = {
            id          = "menu_save",
            font        = app["font"],
            text        = "(S)ave",
            size        = 14,
            x           = 0,
            y           = 0,
            width       = 0,
            height      = 0,
            red         = 0,
            green       = 0,
            blue        = 0,
            alpha       = 255
        },

        menu_help       = {
            id          = "menu_help",
            font        = app["font"],
            text        = "(H)elp",
            size        = 14,
            x           = 0,
            y           = 0,
            width       = 0,
            height      = 0,
            red         = 0,
            green       = 0,
            blue        = 0,
            alpha       = 255
        }

    }
