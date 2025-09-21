#include "../include/sdl_entity.h"

/**
 *
 */
void entity_free(
    SDL_Entity              *entity
) {

    if (entity->type == SDL_ENTITY_IMAGE) {
        if (entity->data.image.path) {
            free(entity->data.image.path);
            entity->data.image.path = NULL;
        }
    }

    if (entity->type == SDL_ENTITY_FONT) {
        if (entity->data.font.path) {
            free(entity->data.font.path);
            entity->data.font.path = NULL;
        }

        if (entity->data.font.string) {
            free(entity->data.font.string);
            entity->data.font.string = NULL;
        }
    }

    entity->type = SDL_ENTITY_EMPTY;
}

/**
 *
 */
char *entity_set_image(
    SDL_Entity              *entity,
    const char              *image_path
) {
    static char err_msg[SDL_ENTITY_ERR_LEN];

    entity_free(entity);

    if ((entity->data.image.path = malloc(strlen(image_path) + 1)) == NULL) {
        snprintf(err_msg, SDL_ENTITY_ERR_LEN, "malloc() error in entity_set_image(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    snprintf(entity->data.image.path, (strlen(image_path) + 1), "%s", (image_path));
    entity->type = SDL_ENTITY_IMAGE;
    
    return NULL;
}

/**
 *
 */
char *entity_set_text(
    SDL_Entity              *entity,
    const char              *font_path,
    const char              *string,
    const int               font_size,
    SDL_Color               rgba
) {
    static char err_msg[SDL_ENTITY_ERR_LEN];

    entity_free(entity);

    entity->data.font.path = malloc(strlen(font_path) + 1);
    entity->data.font.string = malloc(strlen(string) + 1);

    if (! entity->data.font.path || ! entity->data.font.string) {
        snprintf(err_msg, SDL_ENTITY_ERR_LEN, "malloc() error in entity_set_text(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    snprintf(entity->data.font.path, (strlen(font_path) + 1), "%s", (font_path));
    snprintf(entity->data.font.string, (strlen(string) + 1), "%s", (string));
    
    entity->data.font.rgba = rgba;
    entity->data.font.size = font_size;

    entity->type = SDL_ENTITY_FONT;

    return NULL;
}

char *entity_set_audio(
    SDL_Entity              *entity,
    const char              *audio_path
) {
    static char err_msg[SDL_ENTITY_ERR_LEN];

    entity_free(entity);

    if ((entity->data.audio.path = malloc(strlen(audio_path) + 1)) == NULL) {
        snprintf(err_msg, SDL_ENTITY_ERR_LEN, "malloc() error in entity_set_image(): %s\n", strerror(errno));
        return &err_msg[0];
    }

    snprintf(entity->data.audio.path, (strlen(audio_path) + 1), "%s", (audio_path));
    entity->type = SDL_ENTITY_AUDIO;

    return NULL;
}
