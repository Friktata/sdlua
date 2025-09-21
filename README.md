# SDLua

SDLua is a fun little scripting engine that provides a handful of useful SDL3 bindings accessible from Lua scripts.

You can create SDL "entity" objects representing surfaces, textures, and audio, referenced by ID. It uses the [uthash](https://troydhanson.github.io/uthash/) library for super-fast ID lookups, making it very responsive.

With SDLua, you can quickly build simple apps with a single Lua script — no compiling needed — just fast, raw access to some useful SDL3 goodies!

Miniaudio is used instead of SDL3_mixer, since mixer has platform compatibility issues. SDLua runs on Arch Linux and Windows 10 (with some effort), with miniaudio providing a more reliable and portable audio solution for now.

Both uthash and miniaudio source files are included in the `lib/` directory.

---

## Installation

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

All it does is render and display a surface and an image - these are responsive, if you
press the `f` key the app toggles between fullscreen and windowed modes.

We can use the `p` key to pause/resume the track, or if the track ends we can press
`p` to restart the track.

We can also use the `q` key to exit the application.

Any mouse events - clicks or movements are displayed in the terminal, the demo essentially
shows you all you need to know to build a simple game.

---

## Future Plans

Over the coming weeks, I plan to add more functionality for manipulating and working with surfaces and textures.

This project started as a fun way to learn SDL3, having used SDL many years ago. SDL3 has changed a lot, and this has been a great learning experience.

I hadn't coded in C for quite a while (mostly JavaScript and PHP lately), so this has been a refreshing return to C development.

Also, learning how to build a C app embedding Lua has been educational — still a lot to do, and many improvements ahead, but it's functional enough to build neat little apps with a couple of scripts.

---

Thanks for checking out SDLua!
