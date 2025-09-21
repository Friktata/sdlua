#include "../include/err.h"
#include "../include/scripts.h"

/**
 *
 */
int scriptenv_find(
    const SCRIPTS           *scripts,
    const char              *id
) {
    for (int index = 0; index < scripts->size; index++) {
        if (strcmp(id, scripts->id[index]) == 0) {
            return index;
        }
    }

    return -1;
}

/**
 *
 */
char *scriptenv_new(
    SCRIPTS                 *scripts,
    const char              *id,
    unsigned char           flags
) {
    static char err_msg[SCRIPTS_ERR_LEN];

    if (! scripts) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_new(): The SCRIPTS pointer is NULL\n");
        return &err_msg[0];
    }
    if (! id) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_new(): The id pointer is NULL\n");
        return &err_msg[0];
    }

    int index = scriptenv_find(scripts, id);

    if (index >= 0) {
        return NULL;
    }

    index = 0;

    if (scripts->scriptenv) {
        SCRIPTENV **scriptenv = realloc(scripts->scriptenv, (sizeof(SCRIPTENV *) * (scripts->size + 1)));
        char **id = realloc(scripts->id, (sizeof(char *) * (scripts->size + 1)));

        if (! scriptenv || ! id) {
            snprintf(err_msg, SCRIPTS_ERR_LEN, "realloc() error in scriptenv_new(): %s\n", strerror(errno));
            return &err_msg[0];
        }

        scripts->scriptenv = scriptenv;
        index = scripts->size++;
    }
    else {
        scripts->scriptenv = malloc(sizeof(SCRIPTENV *));
        scripts->id = malloc(sizeof(char *));
        scripts->size = 1;

        if (! scripts->scriptenv || ! scripts->id) {
            snprintf(err_msg, SCRIPTS_ERR_LEN, "malloc() error in scriptenv_new(): %s\n", strerror(errno));
            return &err_msg[0];
        }
    }

    scripts->scriptenv[index] = malloc(sizeof(SCRIPTENV));
    scripts->id[index] = malloc(strlen(id) + 1);

    if (! scripts->scriptenv[index] || ! scripts->id[index]) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "malloc() error in scriptenv_new(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    snprintf(scripts->id[index], (strlen(id) + 1), "%s", id);

    scripts->scriptenv[index]->script = NULL;
    scripts->scriptenv[index]->size = 0;
    scripts->scriptenv[index]->state = luaL_newstate();
    scripts->scriptenv[index]->flags = flags;

    return NULL;
}

/**
 *
 */
char *scriptenv_add(
    SCRIPTS                 *scripts,
    const char              *id,
    const char              *path,
    unsigned char           flags
) {
    static char err_msg[SCRIPTS_ERR_LEN];

    if (! scripts) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_new(): The SCRIPTS pointer is NULL\n");
        return &err_msg[0];
    }
    if (! id) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_new(): The id pointer is NULL\n");
        return &err_msg[0];
    }
    if (! path) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_new(): The path pointer is NULL\n");
        return &err_msg[0];
    }

    int index = scriptenv_find(scripts, id);

    if (index < 0) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_new(): Environment \"%s\" doesn\'t exist\n", id);
        return &err_msg[0];
    }

    for (int script = 0; script < scripts->scriptenv[index]->size; script++) {
        if (strcmp(scripts->scriptenv[index]->script[script], path) == 0) {
            snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_new(): Script \"%s\" included in environment \"%s\" more than once\n", path, id);
            return &err_msg[0];
        }
    }

    int script = 0;

    if (scripts->scriptenv[index]->script) {
        script = scripts->scriptenv[index]->size;
        char **new_scripts = realloc(scripts->scriptenv[index]->script, (sizeof(char *) * (script + 1)));
        if (! new_scripts) {
            snprintf(err_msg, SCRIPTS_ERR_LEN, "realloc() error in scriptenv_new(): %s", strerror(errno));
            return &err_msg[0];
        }
        scripts->scriptenv[index]->script = new_scripts;
        scripts->scriptenv[index]->size++;
    }
    else {
        if ((scripts->scriptenv[index]->script = malloc(sizeof(char *))) == NULL) {
            snprintf(err_msg, SCRIPTS_ERR_LEN, "malloc() error in scriptenv_new(): %s", strerror(errno));
            return &err_msg[0];
        }
        scripts->scriptenv[index]->size = 1;
    }
    
    if ((scripts->scriptenv[index]->script[script] = malloc(strlen(path) + 1)) == NULL) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "malloc() error in scriptenv_new(): %s", strerror(errno));
        return &err_msg[0];
    }

    snprintf(scripts->scriptenv[index]->script[script], (strlen(path) + 1), "%s", path);
    scripts->scriptenv[index]->flags = flags;
    scripts->scripts++;

    return NULL;
}

/**
 *
 */
char *scriptenv_free(
    SCRIPTS                 *scripts,
    const char              *id
) {
    static char err_msg[SCRIPTS_ERR_LEN];

    if (! scripts || ! id) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_free(): The SCRIPTS pointer is NULL\n");
        return &err_msg[0];
    }

    int index = scriptenv_find(scripts, id);

    if (index < 0) {
        snprintf(err_msg, SCRIPTS_ERR_LEN, "Error in scriptenv_free(): Environment \"%s\" doesn\'t exist\n", id);
        return &err_msg[0];
    }

    for (int script = 0; script < scripts->scriptenv[index]->size; script++) {
        if (scripts->scriptenv[index]->script[script]) {
            free(scripts->scriptenv[index]->script[script]);
            scripts->scriptenv[index]->script[script] = NULL;
        }
    }

    scripts->scripts -= scripts->scriptenv[index]->size;

    free(scripts->scriptenv[index]->script);
    lua_close(scripts->scriptenv[index]->state);

    free(scripts->scriptenv[index]);
    scripts->scriptenv[index] = NULL;

    free(scripts->id[index]);
    scripts->id[index] = NULL;

    for (index++; index < scripts->size; index++) {
        scripts->scriptenv[(index - 1)] = scripts->scriptenv[index];
        scripts->id[(index - 1)] = scripts->id[index];
    }

    scripts->size--;

    if (! scripts->size) {
        free(scripts->scriptenv);
        free(scripts->id);
        scripts->scriptenv = NULL;
        scripts->id = NULL;
    }
    else {
        SCRIPTENV **scriptenv = realloc(scripts->scriptenv, (sizeof(SCRIPTENV *) * scripts->size));
        char **id = realloc(scripts->id, (sizeof(char *) * scripts->size));

        if (! scriptenv || ! id) {
            snprintf(err_msg, SCRIPTS_ERR_LEN, "malloc() error in scriptenv_free(): %s\n", strerror(errno));
            return &err_msg[0];
        }

        scripts->scriptenv = scriptenv;
        scripts->id = id;
    }

    return NULL;
}

/**
 *
 */
SCRIPTS scripts_new(
    unsigned char           flags
) {
    SCRIPTS scripts = {
        NULL,
        NULL,
        0,
        0,
        flags
    };

    return scripts;
}

/*
 *
 */
void scripts_free(
    SCRIPTS                 *scripts
) {
    for (int i = 0; i < scripts->size; ) {
        const char *env_id = scripts->id[i];
        scriptenv_free(scripts, env_id);
    }
}
