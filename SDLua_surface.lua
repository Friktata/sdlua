-------------------------------------------------------------------------------
--  SDLua_surface.lua
--

print("SDLua_surface STATUS: ", return_status)

if (return_status == nil) then
--  If the return_status is nil then the previous script (SDLua_icon.lua)
--  failed.
    return 1
end

--  We can create a surface of any size...
surface_width = 128
surface_height = 128

sdl_surface = {
    id = "my_surface",
    width = surface_width,
    height = surface_height
}

sdl_error = SDL_Surface(sdl_surface)

if (sdl_error ~= "OK") then
    print(sdl_error)
    return 1
end

--  Fill it with a color...
SDL_Fill({
    id = sdl_surface["id"],
    red = 255,
    green = 255,
    blue = 255,
    alpha = 255
})

--  This will be called any time the screen is resized, it will centre
--  the surface in the display.
function set_surface_area()
    winfo = SDL_Windowinfo()

--  window_info was set in SDLua_icon.lua...
    surface_x = math.floor((winfo["width"] - surface_width) / 2)
    surface_y = math.floor((winfo["height"] - surface_height) / 2)

--  Now we use the surface to create a texture which we can render.
    my_texture = {
        id = sdl_surface["id"],
        x = surface_x,
        y = surface_y,
        width = surface_width,
        height = surface_height
    }
    SDL_Update(my_texture)
    print("Updated my_surface: x ", surface_x, ", y ", surface_y)
end

set_surface_area()

sdl_error = SDL_Texture(my_texture)

if (sdl_error ~= "OK") then
    print(sdl_error)
    return 1
end

status("OK")
