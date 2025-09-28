-------------------------------------------------------------------------------
--  SDLua_demo.lua
--

app = {
--  Flags for SDL_Init()
    sdl_flags = (SDL_INIT_VIDEO | SDL_INIT_AUDIO),

--  Window parameters
    sdl_window = {
        title = "SDLua Demo",
        width = 640,
        height = 480,
        flags = 0
    },

--  Window color
    sdl_window_rgba = {
        red = 32,
        green = 127,
        blue = 255,
        alpha = 255
    }
}

print("Initialising SDL...")

sdl_error = SDL_Init(app["sdl_flags"], app["sdl_window"])

--  If we return without setting a status() then the following script
--  in the chain will receive a return_status of nil.
if (sdl_error ~= "OK") then
    print(sdl_error)
    return 1
end

--  If we get to here then SDL was successfully initialised,
--  set the status to "OK" for the next script.
status("OK")

print("SDL initialised successfully!")

--  We can set the window color using the SDL_Drawcolor() method.
SDL_Drawcolor(app["sdl_window_rgba"])

--  Let's set some global values for the app, all scripts in the chain
--  will be able to access these.
app_run = true
app_timeslice = 16
app_fullscreen = false
