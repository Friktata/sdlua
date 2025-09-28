-------------------------------------------------------------------------------
--  SDLua_loop.lua
--

print("SDLua_loop STATUS: ", return_status)

if (return_status == nil) then
--  If the return_status is nil then the previous script (SDLua_surface.lua)
--  failed.
    return 1
end

while (app_run) do
--  We call SDL_Clear() to clear the display.
    SDL_Clear()

--  Render our surfaces/textures here.
    SDL_Render("my_surface")
    SDL_Render("my_icon")
    SDL_Render("my_font")

--  Animate the SDLua text.
    update_font_position()

--  SDL_Present() to refresh the display.
    SDL_Present()

--  This will poll for any events - the handlers were defined in the
--  SDLua_events.lua script.
    SDL_Poll()

    SDL_Delay(app_timeslice)
end
