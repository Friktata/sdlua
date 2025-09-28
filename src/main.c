
#define _POSIX_C_SOURCE 200809L

#include "../include/app.h"
#include "../include/args.h"
#include "../include/sdl_stacks.h"

#include "../lib/miniaudio/dr_mp3.h"

#define MINIAUDIO_IMPLEMENTATION

#define MA_ENABLE_MP3
#define DR_MP3_IMPLEMENTATION

#include "../lib/miniaudio/dr_mp3.h"
#include "../lib/miniaudio/miniaudio.h"

/**
 *
 */
int main(
    int                 argc,
    char                **argv
) {
    int exit_status = EXIT_SUCCESS;
    
    APP app;

    app.log = NULL;
    app.err = stderr;
    app.err_msg[0] = '\0';
    app.scripts = scripts_new(0);
    app.argv = NULL;
    app.argc = 0;
    app.entity = NULL;
    app.entities = 0;
    app.stacks.stack = NULL;
    app.stacks.size = 0;
    app.stacks.hash = NULL;
    app.flags = 0;
    app.window = NULL;
    app.renderer = NULL;
    app.cursor_name = "default";
    app.hash = NULL;

    char *err = sort_args(&app, argc, argv);

    if (err) {
        goto __lbl_main_error;
    }

    goto __lbl_main_cleanup;

__lbl_main_error:

    if (err) {
        fprintf(app.err, "%s", err);
    }

    if (app.err_msg[0] != '\0') {
        fprintf(app.err, "%s", app.err_msg);
    }

    exit_status = EXIT_FAILURE;

__lbl_main_cleanup:

    scripts_free(&app.scripts);

    if (app.argv) {
        free(app.argv);
        app.argv = NULL;
        app.argc = 0;
    }

    app_cleanup(&app);

    if (app.log && (app.log != stdout && app.log != stderr)) {
        fclose(app.log);
        app.log = NULL;
    }

    if (app.err && (app.err != stdout && app.err != stderr)) {
        fclose(app.err);
        app.err = NULL;
    }

    return exit_status;
}
