-------------------------------------------------------------------------------
--  SDLua_draw/src/main.lua
--

    while (app["state"]["running"]) do

        SDL_Clear()

        SDL_Render(app_menu["id"])

        print(ui.render_menu_text())
        
        SDL_Error = ui.render_menu_text()
        if (app["Error"] ~= "OK") then
            app["state"]["running"] = false
        end

        SDL_Present()

        SDL_Poll()

        SDL_Delay(app["timeslice"])

    end

