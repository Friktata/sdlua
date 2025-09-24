-------------------------------------------------------------------------------
--  SDLua_draw/src/events.lua
--
    -- function find_menu_option(x, y)
    --     for key in pairs(app_menu_options) do
    --         option_x = app_menu_options[key]["x"]
    --         option_y = app_menu_options[key]["y"]
    --         bound_x = (option_x + app_menu_options[key]["width"])
    --         bound_y = (option_y + app_menu_options[key]["height"])

    --         if (x >= option_x and x < bound_x) then
    --             if (y >= option_y and y < bound_y) then
    --                 return app_menu_options[key]
    --             end
    --         end
    --     end

    --     return nil
    -- end

    menu = require("ui")

    SDL_Event("quit", function(event)
        app["state"]["running"] = false
    end)

    SDL_Event("keydown", function(event)

        if (event.char == 'q' or event.char == 'Q') then
            app["state"]["running"] = false
        end

    end)

    SDL_Event("mousedown", function(event)

        if (event.y < app_menu["height"]) then
    --  Some click event occurred in the menu
            menu_result = menu.find_menu_option(event.x, event.y)
            
            if (type(menu_result) == "table") then
                print("Clicked on " .. menu_result["id"])

                if (menu_result["id"] == app_menu_options["menu_quit"]["id"]) then
                    app["state"]["running"] = false
                end
            end
        end

    end)

    SDL_Event("mousemove", function(event)

        if (event.y < app_menu["height"]) then
    --  Some click event occurred in the menu
            menu_result = menu.find_menu_option(event.x, event.y)
            
            if (type(menu_result) == "table") then
                SDL_Setcursor("hand")
            else
                SDL_Setcursor("default")
            end
        end

    end)
    