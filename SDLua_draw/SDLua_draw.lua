#!/usr/bin/env -S sdlua --log --stdout

-------------------------------------------------------------------------------
--  SDLua_draw/SDLua_draw.lua
--
--  Configs are stored in the config/ directory, these are scripts
--  that reveal tables describing UI components.
--
--  Controllers are stored in the src/ directory, these are scripts that
--  provide the logic and control the application.
--
    package.path = package.path .. ";./config/?.lua;./src/?.lua"


-------------------------------------------------------------------------------
--  Initialise SDL and create the main window.
--
    require("app")
    
    SDL_Error = SDL_Init(app["flags"], app["window"])
    if (SDL_Error ~= "OK") then
        print(SDL_Error)
        return 1
    end

    SDL_Error = SDL_Drawcolor(app["window_color"])
    if (SDL_Error ~= "OK") then
        SDL_Quit()
        print(SDL_Error)
        return 1
    end


-------------------------------------------------------------------------------
--  Import UI configs.
--
    require("menu")


-------------------------------------------------------------------------------
--  Import src/ controller scripts.
--
    ui = require("ui")
    require("events")


    SDL_Error = ui.init_menu()

    if (app["error"] ~= nil) then
        app["state"]["running"] = false
    end

-------------------------------------------------------------------------------
--  Begin the main loop.
--
    while (app["state"]["running"]) do

        SDL_Clear()

        SDL_Render(app_menu["id"])

        ui.render_menu_text()

        if (app["error"] ~= nil) then
            app["state"]["running"] = false
        end

        SDL_Present()

        SDL_Poll()

        SDL_Delay(app["timeslice"])

    end


-------------------------------------------------------------------------------
--  Done.
--
    SDL_Quit()

    if (app["error"] ~= nil) then
        print(app["error"])
        return 1
    end
