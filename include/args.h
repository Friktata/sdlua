#ifndef ARGS_H
#define ARGS_H

#define MINIAUDIO_MP3

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "app.h"
#include "scripts.h"

char        *sort_args       (APP *, int, char **);

#endif
