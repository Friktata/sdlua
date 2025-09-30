### Updates

**update** 28/09/2025

**SDLK_**

I added some SDL keyboard key codes - see:

    src/sdl_lib.h

You'll find them along with various other SDL specific values/flags that
are exposed in the Lua environment when our scripts execute.

You can handle keyboard events easily using the ```SDL_Event``` and ```SDL_Poll```
functions. Here's a simple app with a main loop that will exit if either the **Q**
or **Escape** keys are pressed.

```
    local app_running = true

    -- Define a callback for the keydown event.
    SDL_Event("keydown", function(event)
        if (event.key == SDLK_Q or event.key == SDLK_ESCAPE) then
            app_running = false
        end

        -- Alternatively you can use event.char...
        --
        -- if (event.char == 'q' or event.key == SDLK_ESCAPE) then
        --     app_running = false
        -- end
    end)

    -- Main loop.
    while (app_running) do

        SDL_Clear()                     -- Clear the window
        SDL_Render(my_surface["id"])    -- Render something
        SDL_Present()                   -- Redraw the display

        SDL_Poll()                      -- Poll for events

        SDL_Delay(32)                   -- Delay in milliseconds

    end
```

I also moved the SDLua_demo.lua script and related scripts into:

    demo/

Just to tidy things up, this makes running the script a bit of a pain, here's
the command if you want to just copy and paste to test it:

    ./sdlua \
        demo/SDLua_demo.lua \
        demo/SDLua_audio.lua \
        demo/SDLua_icon.lua \
        demo/SDLua_surface.lua \
        demo/SDLua_font.lua \
        demo/SDLua_events.lua \
        demo/SDLua_loop.lua \
        demo/SDLua_quit.lua 

**new stuff**

Added a few new features. I've been building an image editor (**SDLua_draw**)
which is basically a simply **MS Paint** clone. We can use it to create and edit
various formats.

I added a few new functions:

```
    SDL_Circle()
    SDL_Pushsurface()
    SDL_Prevsurface()
    SDL_Nextsurface()
    SDL_Flushsurfaces()
```

**SDL_Circle()**

This function is fairly self-explanitory, it lets you draw a circle - here's
an example:

```
    SDL_Circle({
        id = "entity_id",
        x = 10,
        y = 10,
        diameter = 5,
        red = 255,
        green = 0,
        blue = 0,
        alpha = 255
    })
```

This draws a circle at 10x10 with a 5 pixel diameter. Note the x,y co-ordinates
specifiy the center of the circle to be drawn, not the upper left as you might
expect - the reason for this is I created the SDL_Circle() function to create the
pencil tool for my editor so this made most sense.

**Surface stacks**

The other functions:

```
    SDL_Pushsurface()
    SDL_Prevsurface()
    SDL_Nextsurface()
    SDL_Flushsurfaces()
```

First - you have to understand how SDLua manages surfaces and textures. When we
create a surface:

```
    my_surface = {
        id = "surface_id",
        x = 10,
        y = 20,
        height = 100,
        width = 100
    }

    SDL_Error = SDL_Surface(my_Surface)

    if (SDL_Error ~= "OK") then
        print("Error: " .. SDL_Error)
    end
```

The SDL_Surface function returns a string, "OK" on success or an error message
on failure. It does not return a surface, those are stored internally within the
**sdlua** environment. Instead we give the surface an ID which we use to reference
the surface in our Lua scripts.

Actually, we're creating an SDL_Entity type native to sdlua, this can be anything
from a basic surface to an audio track, image, or font. These are all entities,
they're maintained internally within sdlua, we use functions/wrappers to tell
sdlua what we want to do and with what entity but we don't manage the asset
directly.

The ```SDL_Stack``` data type allows us to create a surface stack, which is useful
if we're editing surfaces...like an image editor.

Like entities, stacks have a unique ID, we can create a stack easily by simply
pushing a surface to it, if the stack doesn't exist it'll be created:

```
    SDL_Pushsurface("stack_id", "surface_id")
```

This will create a stack with the default size of 100 surfaces. We can specify a
size:

```
    SDL_Pushsurface("stack_id", "surface_id", 500)
```

And if the stack doesn't exist it'll be created with 500 surfaces.

What this means, internally, is that whatever surface entity **surface_id** points
to will be pushed to index 0 of the stack named **stack_id**, the SDL_Stack structure
will keep track of our **position** in the stack, since it's the first element our
position will be 0.

As we push more elements onto the stack, the stack grows as does our position.

We can use the ```SDL_Prevsurface()``` and ```SDL_Nextsurface()``` functions to shift
the position in the stack to point to any stored surface, which we can restore:

```
    SDL_Prevsurface("stack_id", "surface_id")
    SDL_Texture(my_surface)
    SDL_Render(my_surface["id"])
```

```SDL_Prevsurface``` tells sdlua to shift the position back (assuming position > 0)
and then restore that surface back to the surface with an entity ID of **surface_id**,
which we can then use to create and render a texture.

Naturally, the ```SDL_Nextsurface``` funtion will increase the position to the right
assuming position doesn't already point to the top of the stack.

Lastly, the ```SDL_Flushsurfaces``` function will clear all surfaces after the current
position - i.e it clears everything to the right.

I was able to use these features to build **undo/redo** functionality into my little
image editor which I'll be uploading as a little demo in the next day or so.

---

**update** 26/09/2025

Fixed some bugs and other small issues.

Added the **scroll** mouse event, we can now do:

```
    SDL_Event("scroll", function(event)
        --  event.y will be 1 (scroll up) or -1 (scroll down)
        --
        --  We can also get the mouse position with event.mouse_x
        --  and event.mouse_y
    end)
```

I also added a way to pass command line arguments to scripts, it's a
bit cumbersome right now but you can use `--arg <value>` to set a
parameter for the script environment, these are indexed starting at 0:

```
    sdlua script_name --arg my_file.txt
```

This will make "my_file.txt" available in the environment, we can use
the `arg()` function to manage arguments.

```
    --  arg() with no parameters returns the argument count
    arg_count = arg()
    
    --  Or we can specify the index of an argument we want to reference,
    --  indexes in this case are 0-based.
    argv_0 = arg(0)
    argv_1 = arg(1)
    ...etc...
```

---

**update** 24/09/2025

Fixed a bug that was causing a seg-fault when `sdlua` was being invoked via a shebang,
some path related issues and a weird bug with the `--log --stdout` option.

I also added a couple of new functions:

    SDL_Surfaceinfo()
    SDL_Setcursor()

The ```SDL_Surfaceinfo()``` function lets us grab information about a surface (**width**,
**height**, etc).

This is useful because when we create an image or font texture we might not
necessarily know the area it consumes, we can create an image like this:

```
    img = {
        id = "entity_id",
        image = "path/to/image",
        x = 100,
        y = 100,
        width = 0,
        height = 0
    }
```

Which will load the image at its default dimensions, if we do:

```
    img = {
        id = "entity_id",
        image = "path/to/image",
        x = 100,
        y = 100,
        width = 100,
        height = 100
    }
```

Then the image will be scaled to 100x100 irrespective of what size the
image actually is.

Problem is we don't know the image size, so we use ```SDL_Surfaceinfo()```
to get a table describing the surface/texture area:

```
    img_info = SDL_Surfaceinfo("entity_id")

--  Either returns a table on success or a string containing an
--  error message on failure.
    if (type(img_info) ~= "table") then
        print(img_info)
        return 1
    end

--  Update the image dimensions.
    img["width"] = img_info["width"]
    img["height"] = img_info["height"]

--  We have to update the entity with the new values, this
--  means we don't need to destroy and create new entities,
--  we can just call SDL_Render("entity_id") and it will
--  reflect the changes. 
    SDL_Update(img)
```

This also works with ```SDL_Text()``` if you want a font to scale to a width and
height based on the font size.

We can use ```SDL_Setcursor()``` to set the cursor:

```
    SDL_Setcursor("hand")
```

Fairly self explanitory, you will find the full list of **SDL_INIT_, SDL_WINDOW_** and
**SDL_SYSTEM_CURSOR_** flags towards the bottom of the **src/sdl_lib.c** file.

---

**update** 23/09/2025

Added some new stuff - firstly, more error checking to validate input, functions will
check to ensure the correct parameters are passed and handle the logging.

Also added a few new functions, specifically:

    SDL_Putpixel()
    SDL_Getpixel()

See:

    include/sdl_lib.h

Simple enough, we can call:

```
    SDL_Setpixel({
        id = "surface",
        x = 10,
        y = 10,
        red = 123,
        green = 231,
        blue = 132,
        alpha = 255
    })
```

To set a pixel at an **x, y** position on a surface, we can then use
something like:

```
    pixel_info = SDL_Getpixel({
        id = "surface",
        x = 10,
        y = 10
    })

    print("red=" .. pixel_info["red"])
    print("green=" .. pixel_info["green"])
    print("blue=" .. pixel_info["blue"])
    print("alpha=" .. pixel_info["alpha"])
```

For a list of available functions.

---

# SDLua

SDLua is a fun little scripting engine that provides a handful of useful SDL3 bindings accessible from Lua scripts.

You can create SDL "entity" objects representing surfaces, textures, and audio, referenced by ID. It uses the [uthash](https://troydhanson.github.io/uthash/) library for super-fast ID lookups, making it very responsive.

With SDLua, you can quickly build simple apps with a single Lua script, no compilation needed! just fast, raw access to some useful SDL3 goodies!

Miniaudio is used instead of SDL3_mixer, since mixer has platform compatibility issues. SDLua runs on Arch Linux and Windows 10 (with some effort), with miniaudio providing a more reliable and portable audio solution for now.

Both uthash and miniaudio source files are included in the `lib/` directory.

---

## Clone and compile

Make sure SDL3, SDL3_image, and SDL3_ttf subsystems are installed on your target machine.

Clone the repo:

```bash
git clone https://github.com/Friktata/sdlua.git
```

Build the project:

```bash
cd sdlua
make
```

The provided `Makefile` compiles sources and links the required libraries.

Run the application:

```bash
./sdlua
```

Clean up build files:

```bash
make clean
```

---

## Command Line Options

### Running Scripts with Shared Environment

The default environment (`env`) allows multiple scripts to share data:

```bash
./sdlua one two three
```

This runs three scripts sequentially, all sharing the same environment.

---

### Creating Separate Environments with `--env`

You can create multiple environments using the `--env` option:

```bash
./sdlua one --env env_two two --env env_three three
```

This runs each script in its own environment:

| Environment ID | Script  |
|----------------|---------|
| `env`          | one     |
| `env_two`      | two     |
| `env_three`    | three   |

You can add more scripts to existing environments in any order:

```bash
./sdlua one --env env_two two --env env_three three --env env additional
```

Resulting environments:

| Environment ID | Script     |
|----------------|------------|
| `env`          | one        |
| `env`          | additional |
| `env_two`      | two        |
| `env_three`    | three      |

The first created environment (`env`) always executes first, running attached scripts in order.

---

### Printing Environment Info with `--printenv`

Print all environments and their scripts:

```bash
./sdlua one --env env_two two --env env_three three --env env additional --printenv
```

Print a list of scripts belonging a specific environment:

```bash
./sdlua --printenv env
```

---

### Running Environments with `-run`

Example:

```bash
./sdlua one --env env_two two -run three
```

- Attach script `one` to the default `env`.
- Create environment `env_two` and attach `two`.
- Run both environments (`env` and `env_two`), then clear/reset data.
- Attach script `three` to a fresh `env` (recreated after `-run`).

---

### Experimental Options

Some other options are experimental and not fully tested. See `src/args.c`, specifically the `sort_args()` function.

---

## Features and Demo

SDLua currently supports:

- Creating windows
- Creating surfaces
- Creating textures
- Importing and displaying images
- Importing and playing audio (formats: `.wav`, `.mp3`)
- Using TrueType fonts to display text
- Handling input events (mouse, keyboard, etc.)

I've created a chain of 8 demo scripts illustrating these features:

- `SDLua_demo.lua`
- `SDLua_audio.lua`
- `SDLua_events.lua`
- `SDLua_icon.lua`
- `SDLua_font.lua`
- `SDLua_surface.lua`
- `SDLua_loop.lua`
- `SDLua_quit.lua`

Run the demo with verbose logging:

```bash
./sdlua --log --stdout SDLua_demo.lua SDLua_audio.lua SDLua_events.lua SDLua_icon.lua SDLua_surface.lua SDLua_font.lua SDLua_loop.lua SDLua_quit.lua
```

The `--log` option enables verbose output to standard output; omit it to disable logs.

Each script is small, well-commented, and designed to showcase specific features.

All it does is render and display a surface and an image and makes responsive. If you
press the `f` key the app toggles between fullscreen and windowed modes.

We can use the `p` key to pause/resume the track, or if the track ends we can press
`p` to restart the track.

We can also use the `q` key to exit the application.

Any mouse events - clicks or movements are displayed in the terminal, the demo essentially
shows you all you need to know to build a simple game.

---

## Future Plans

Over the coming weeks I plan to add more functionality for manipulating and working with surfaces and textures.

This project started as a fun way to learn SDL3, having used SDL many years ago. SDL3 has changed a lot, and this has been a great learning experience.

I hadn't coded in C for quite a while (mostly JavaScript and PHP lately), so this has been a refreshing return to C development.

Also, learning how to build a C app embedding Lua has been educational — still a lot to do, and many improvements ahead, but it's functional enough to build neat little apps with a couple of scripts.

---

