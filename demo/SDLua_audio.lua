-------------------------------------------------------------------------------
--  SDLua_audio.lua
--

print("SDLua_audio STATUS: ", return_status)

--  If the return status is nil then the previous script failed.
if (return_status == nil) then
--  If the return_status is nil then the previous script (SDLua_demo.lua)
--  failed.
    return 1
end

--  Self-explanitory.
audio_track = {
    id = "my_audio_track",
    audio = "./audio/tune.mp3",
    autoplay = true
}

--  This will automatically load and start playing the track.
sdl_error = SDL_Audio(audio_track)

if (sdl_error ~= "OK") then
    print(sdl_error)
    return 1
end

--  ...th...that's pretty much it!

status("OK")
