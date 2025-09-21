-------------------------------------------------------------------------------
--  SDLua_events.lua
--

print("SDLua_events STATUS: ", return_status)

window_info = 0

if (return_status == nil) then
--  If the return_status is nil then the previous script (SDLua_audio.lua)
--  failed.
    return 1
end

--  We can set up event handlers using the SDL_Event() method, the following
--  events are currently supported:
--
--      fullscreen          - Triggered when the app switches to fullscreen
--      windowed            - When the app switches to windowed mode
--      resize              - Triggers for both windowed and fullscreen
--      quit                - When the x (close) button is clicked
--      keydown             - A key was pressed
--      keyup               - A key was released
--      mousedown           - Mouse button was pressed
--      mouseup             - Mouse button was released
--      mousemove           - Mouse movement
--

--  The quit event is triggered when the window close button is clicked.
SDL_Event("quit", function(event)
--  The main loop (SDLua_loop.lua) will terminate when app_run is set to
--  false.
    app_run = false
end)

--  Let's handle some keyboard events.
SDL_Event("keydown", function(event)
--  App will also terminate if we press the q key.
    if (event.char == "q" or event.char == "Q") then
        app_run = false
    end

--  Pressing the f key will toggle between fullscreen and windowed mode.
    if (event.char == "f" or event.char == "F") then
        app_fullscreen = not app_fullscreen
        SDL_Fullscreen(app_fullscreen)
    end

--  Pressing the p key will pause/play the audio track.
    if (event.char == "p" or event.char == "P") then
        print("PLAY")
--  SDL_Audioinfo() gives us information about the track that is currently
--  loaded.
        audio_info = SDL_Audioinfo(audio_track)
        print("Track duration: ", audio_info["duration"])
        print("Track position: ", audio_info["position"])
        print("Track volume: ", audio_info["volume"])
        print("Track playing: ", audio_info["playing"])
        if (audio_info["playing"]) then
            SDL_Pause(audio_track)
        else
            SDL_Play(audio_track)
        end
    end
end)

--  Mouse events.
mouse_buttons = {
    "Left",
    "Middle",
    "Right"
}

SDL_Event("mousedown", function(event)
    print(mouse_buttons[event.button], " button down at ", event.x, ", ", event.y)
end)
SDL_Event("mouseup", function(event)
    print(mouse_buttons[event.button], " button up at ", event.x, ", ", event.y)
end)
SDL_Event("mousemove", function(event)
    print("Mouse move to ", event.x, ", ", event.y)
end)

--  The fullscreen event fires when the app switches from windowed
--  to fullscreen mode.
SDL_Event("fullscreen", function(event)
--  Need to update the icon and surface positions
    set_surface_area()
    set_image_area()
    
    print("Going fullscreen!")
end)

--  The fullscreen event fires when the app switches from fullscreen
--  to windowed mode.
SDL_Event("windowed", function(event)
--  Need to update the icon and surface positions
    set_surface_area()
    set_image_area()
    
    print("Going windowed!")
end)

--  Resize fires for both fullscreen and windowed events.
SDL_Event("resize", function(event)
    print("RESIZE EVENT TRIGGERED!")
end)

status("OK")
