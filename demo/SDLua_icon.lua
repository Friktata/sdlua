-------------------------------------------------------------------------------
--  SDLua_icon.lua
--

print("SDLua_icon STATUS: ", return_status)

if (return_status == nil) then
--  If the return_status is nil then the previous script (SDLua_events.lua)
--  failed.
    return 1
end

icon_width = 64
icon_height = 64

--  This will be called any time the screen is resized, it will centre
--  the icon image in the display.
function set_image_area()
    winfo = SDL_Windowinfo();

    --  We can grab the window dimensions, we can use this info to centre
    --  the icon.
    icon_x = math.floor((winfo["width"] - icon_width) / 2)
    icon_y = math.floor((winfo["height"] - icon_height) / 2)

    SDL_Update({
        id = "my_icon",
        x = icon_x,
        y = icon_y,
        width = icon_width,
        height = icon_height
    })
    
    print("Updated my_icon: x ", icon_x, ", y ", icon_y)
end

set_image_area()

--  Create a new image texture.
--
--  Each SDL entity we create, whether it's some text, an image, or an audio
--  file, requires a uinque id.
--
--  This id is used if we want to reference the entity at a later point in
--  our application.
icon_texture = {
    id = "my_icon",
    image = "./images/Friktata.jpg",
    x = icon_x,
    y = icon_y,
    width = icon_width,
    height = icon_height,
}

sdl_error = SDL_Image(icon_texture)

if (sdl_error ~= "OK") then
    status(sdl_error)
    return 1
end

--  All good, we set the status to "OK" for the next script in the chain.
status("OK")
