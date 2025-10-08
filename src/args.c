#include "../include/err.h"
#include "../include/scripts.h"
#include "../include/args.h"
#include "../include/exec.h"

/**
 *
 */
char *__printenv(
    APP                 *app,
    const char          *id
) {
    static char err_msg[SCRIPTS_ERR_LEN];

    if (! app->log) {
        return NULL;
    }

    if (id) {
        fprintf(app->log, "Dumping environment %s\n", id);
    }
    else {
        fprintf(app->log, "Dumping %d script environments\n", app->scripts.size);
    }

    for (int scriptenv = 0; scriptenv < app->scripts.size; scriptenv++) {
        if (id) {
            if (strcmp(id, app->scripts.id[scriptenv]) != 0) {
                continue;
            }
        }

        fprintf(app->log, "\tEnv %d: %s (%d scripts):\n", scriptenv, app->scripts.id[scriptenv], app->scripts.scriptenv[scriptenv]->size);
        
        for (int script = 0; script < app->scripts.scriptenv[scriptenv]->size; script++) {
            fprintf(app->log, "\t\tScript %d (%s)\n", script, app->scripts.scriptenv[scriptenv]->script[script]);
        }

        if (id) {
            return NULL;
        }
    }

    if (id) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in sort_args(): Environment \"%s\" doesn\'t exist\n", id);
        return &err_msg[0];
    }

    return NULL;
}

/**
 *
 */
char *sort_args(
    APP                 *app,
    int                 argc,
    char                **argv
) {
    static char err_msg[SCRIPTS_ERR_LEN];
    char *err;

    char *current_env = "env";
    unsigned char current_flags = (SCRIPTS_F_LUALIBS | SCRIPTS_F_EXTLIBS | SCRIPTS_F_SDLLIBS);

    lua_State *state = NULL;
    LuaStore shared_status = { LUA_T_EMPTY };

    app->argv = malloc(sizeof(char *) * argc);

    for (int arg = 1; arg < argc; arg++) {
        if (strcmp(argv[arg], "--log") == 0) {
            if (++arg >= argc) {
                return "Error in sort_args(): The --log option requires a parameter\n";
            }

            if (app->log && (app->log != stdout && app->log != stderr)) {
                fclose(app->log);
            }

            if (strcmp(argv[arg], "--stdout") == 0) {
                app->log = stdout;
            }
            else if (strcmp(argv[arg], "--") == 0) {
                app->log = NULL;
            }
            else {
                if ((app->log = fopen(argv[arg], "a")) == NULL) {
                    snprintf(err_msg, SCRIPTS_ERR_LEN, "fopen() error in sort_args(): Error opening file %s\n%s\n", argv[arg], strerror(errno));
                    return &err_msg[0];
                }
            }

            continue;
        }

        if (strcmp(argv[arg], "--err") == 0) {
            if (++arg >= argc) {
                return "Error in sort_args(): The --err option requires a parameter\n";
            }

            if (app->err && (app->err != stdout && app->err != stderr)) {
                fclose(app->err);
            }

            if (strcmp(argv[arg], "--stderr") == 0) {
                app->err = stderr;
            }
            else {
                if ((app->err = fopen(argv[arg], "a")) == NULL) {
                    app->err = stderr;
                    snprintf(err_msg, SCRIPTS_ERR_LEN, "fopen() error in sort_args(): Error opening file %s\n%s\n", argv[arg], strerror(errno));
                    return &err_msg[0];
                }
            }

            continue;
        }
    }

    for (int arg = 1; arg < argc; arg++) {
        if ((strcmp(argv[arg], "--log") == 0) || (strcmp(argv[arg], "--err") == 0)) {
            arg++;
            continue;
        }
        
        if (strcmp(argv[arg], "--env") == 0) {
            if (++arg >= argc) {
                return "Error in sort_args(): The --env option requires a parameter\n";
            }

            current_env = argv[arg];
            current_flags &= ~SCRIPTS_F_NOEXEC;

            continue;
        }

        if (strcmp(argv[arg], "--env-noexec") == 0) {
            if (++arg >= argc) {
                return "Error in sort_args(): The --env-noexec option requires a parameter\n";
            }

            current_env = argv[arg];
            current_flags |= SCRIPTS_F_NOEXEC;

            continue;
        }

        if (strcmp(argv[arg], "--printenv") == 0) {
            if (++arg >= argc) {
                return "Error in sort_args(): The --printenv option requires a parameter\n";
            }

            if (strcmp(argv[arg], "--") == 0) {
                __printenv(app, NULL);
            }
            else {
                if ((err = __printenv(app, argv[arg])) != NULL) {
                    return err;
                }
            }

            continue;
        }

        if (strcmp(argv[arg], "--arg") == 0) {
            if (++arg >= argc) {
                return "Error in sort_args(): The --arg option requires a parameter\n";
            }
            
            app->argv[app->argc] = argv[arg];
            app->argc++;

            continue;
        }

        if (strcmp(argv[arg], "--reset") == 0) {
            scripts_free(&app->scripts);
            if (app->log) {
                fprintf(app->log, ">>> Resetting - all environments were destroyed\n");
            }

            current_env = "env";
            current_flags = 0;

            continue;
        }

        if ((strcmp(argv[arg], "-run") == 0) || (strcmp(argv[arg], "-runclear") == 0)) {
            unsigned char __run_flags = 0;

            if (strcmp(argv[arg], "-runclear") == 0) {
                __run_flags |= SCRIPTS_F_NOEXEC;
            }

            if (! app->scripts.scripts) {
                return "Error in sort_args(): Nothing to run\n";
            }

            if (state) {
                state = NULL;
            }

            if ((err = exec_all(app->log, &app->scripts, &state, __run_flags, &shared_status)) != NULL) {
                return err;
            }

            current_env = "env";
            current_flags = (SCRIPTS_F_LUALIBS | SCRIPTS_F_EXTLIBS | SCRIPTS_F_SDLLIBS);

            continue;
        }

        if (strcmp(argv[arg], "-exitifnil") == 0) {
            if (! state) {
                continue;
            }

            lua_getfield(state, LUA_REGISTRYINDEX, "__return_status");

            if (lua_isnil(state, -1)) {
                break;
            }

            continue;
        }

        if (strcmp(argv[arg], "-exitifnotnil") == 0) {
            if (! state) {
                continue;
            }

            lua_getfield(state, LUA_REGISTRYINDEX, "__return_status");

            if (! lua_isnil(state, -1)) {
                break;
            }

            continue;
        }

        if (scriptenv_find(&app->scripts, current_env) < 0) {
            if ((err = scriptenv_new(&app->scripts, current_env, current_flags)) != NULL) {
                return err;
            }

            int env_index = scriptenv_find(&app->scripts, (const char *) current_env);
            *(void **) lua_getextraspace(app->scripts.scriptenv[env_index]->state) = app;

            if (app->log) {
                fprintf(app->log, ">>> Created new environment (%d): \"%s\"\n", app->scripts.size, current_env);
            }
        }

        if ((err = scriptenv_add(&app->scripts, current_env, argv[arg], current_flags)) != NULL) {
            return err;
        }
    }

    if (app->scripts.scripts) {
        if (state) {
            state = NULL;
        }
        
        if ((err = exec_all(app->log, &app->scripts, &state, 0, &shared_status)) != NULL) {
            return err;
        }
    }

    return NULL;
}
