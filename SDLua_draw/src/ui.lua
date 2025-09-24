-------------------------------------------------------------------------------
--  SDLua_draw/src/ui.lua
--

-------------------------------------------------------------------------------
--  Menu
--
    local M = {}

    function M.init_menu()
        local menu_option_left = 0

        SDL_Error = SDL_Surface(app_menu)
        if (SDL_Error ~= "OK") then
            app["error"] = SDL_Error
            return 1
        end

        SDL_Error = SDL_Fill(app_menu)
        if (SDL_Error ~= "OK") then
            app["error"] = SDL_Error
            return 1
        end

        SDL_Error = SDL_Texture(app_menu)
        if (SDL_Error ~= "OK") then
            app["error"] = SDL_Error
            return 1
        end

        for key in pairs(app_menu_options) do
            SDL_Error = SDL_Text(app_menu_options[key])
            if (SDL_Error ~= "OK") then
                app["error"] = SDL_Error
                return 1
            end

    --  Because the text area isn't being specified (both width and,
    --  height are 0, see ../config/menu.lua) we're allowing the font
    --  size to dictate the texture area. We need to query the surface
    --  to grab the default dimensions and update app_menu_options[key]
    --  for the call to SDL_Render().
            text_area = SDL_Surfaceinfo(app_menu_options[key]["id"])
            if (type(text_area) ~= "table") then
                print("init_menu(): text_area is not a table: " .. text_area)
                app["error"] = text_area;
                return 1
            end

            app_menu_options[key]["width"] = text_area["width"]
            app_menu_options[key]["height"] = text_area["height"]
            
    --  Now we can figure out the top and left positions for this
    --  element. Menu text is centered vertically, the horizontal
    --  position is the position + width of the last item + 6 pixels.
            local text_top = math.floor((app_menu["height"] - app_menu_options[key]["height"]) / 2)
            local text_left = math.floor(app_menu_options[key]["x"] + 6 + menu_option_left)

            app_menu_options[key]["y"] = text_top
            app_menu_options[key]["x"] = text_left

    --  Update the texture with the new area/dimensions.
            SDL_Error = SDL_Update(app_menu_options[key])
            if (SDL_Error ~= "OK") then
                app["error"] = SDL_Error
                return 1
            end

            menu_option_left = (app_menu_options[key]["x"] + text_area["width"] + 6)
        end
    end

    function M.render_menu_text()
        for key in pairs(app_menu_options) do
            SDL_Error = SDL_Render(app_menu_options[key]["id"])
            
            if (SDL_Error ~= "OK") then
                app["error"] = SDL_Error
                return 1
            end
        end
    end

    function M.find_menu_option(x, y)
        for key in pairs(app_menu_options) do
            option_x = app_menu_options[key]["x"]
            option_y = app_menu_options[key]["y"]
            bound_x = (option_x + app_menu_options[key]["width"])
            bound_y = (option_y + app_menu_options[key]["height"])

            if (x >= option_x and x < bound_x) then
                if (y >= option_y and y < bound_y) then
                    return app_menu_options[key]
                end
            end
        end

        return nil
    end

    return M
