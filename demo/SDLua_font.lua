-------------------------------------------------------------------------------
--  SDLua_font.lua
--

if (return_status == nil) then
    return 1
end

font_width = 150
font_height = 32

--  All fairly self-explanitory.
my_text = {
    id = "my_font",
    font = "/usr/share/fonts/TTF/Hack-Regular.ttf",
    size = 32,
    text = "SDLua",
    x = 32,
    y = 32,
    width = font_width,
    height = font_height,
    red = 0,
    green = 0,
    blue = 0,
    alpha = 255
}

SDL_Text(my_text)

--  We will animate the text in the main loop (see SDLua_demo.lua)
font_x_animation = 2
font_y_animation = -2

--  The SDLua text is animated, this function is called from the main
--  loop in the SDLua_loop.lua script.
function update_font_position()
    window_info = SDL_Windowinfo()

    if (font_x_animation < 0 and my_text["x"] <= 0) then
        font_x_animation = 2
    elseif (font_x_animation > 0 and (my_text["x"] + font_width) >= window_info["width"]) then
        font_x_animation = -2
    end

    if (font_y_animation < 0 and my_text["y"] <= 0) then
        font_y_animation = 2
    elseif (font_y_animation > 0 and (my_text["y"] + font_height) >= window_info["height"]) then
        font_y_animation = -2
    end

    my_text["x"] = (my_text["x"] + font_x_animation)
    my_text["y"] = (my_text["y"] + font_y_animation)

--  We can update the font texture using the SDL_Update() method.
    SDL_Update(my_text)
end

status("OK")
