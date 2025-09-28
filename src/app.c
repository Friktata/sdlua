#include "../include/app.h"

ma_engine audio_engine;

/** 
 *
 */
void app_cleanup(
    APP                 *app
) {
    SDL_Entity *current_entry, *tmp;

    if (app->hash)
        HASH_CLEAR(hh, app->hash);

    if (app->entity) {
        for (int entity = 0; entity < app->entities; entity++) {
            if (app->entity[entity]->type == SDL_ENTITY_FONT) {
                if (app->entity[entity]->data.font.path) {
                    free(app->entity[entity]->data.font.path);
                    app->entity[entity]->data.font.path = NULL;
                }
                if (app->entity[entity]->data.font.string) {
                    free(app->entity[entity]->data.font.string);
                    app->entity[entity]->data.font.string = NULL;
                }
            }

            if (app->entity[entity]->type == SDL_ENTITY_IMAGE) {
                if (app->entity[entity]->data.image.path) {
                    free(app->entity[entity]->data.image.path);
                    app->entity[entity]->data.image.path = NULL;
                }
            }

            if (app->entity[entity]->type == SDL_ENTITY_AUDIO) {
                if (app->entity[entity]->data.audio.path) {
                    free(app->entity[entity]->data.audio.path);
                    app->entity[entity]->data.audio.path = NULL;
                }
                ma_sound_uninit(&app->entity[entity]->sound);
            }

            if (app->entity[entity]->id) {
                free(app->entity[entity]->id);
                app->entity[entity]->id = NULL;
            }
            if (app->entity[entity]->surface) {
                SDL_DestroySurface(app->entity[entity]->surface);
                app->entity[entity]->surface = NULL;
            }
            if (app->entity[entity]->texture) {
                SDL_DestroyTexture(app->entity[entity]->texture);
                app->entity[entity]->texture = NULL;
            }

            free(app->entity[entity]);
            app->entity[entity] = NULL;     
        }

        free(app->entity);
        app->entity = NULL;
        app->entities = 0;
    }

    if (app->renderer) {
        SDL_DestroyRenderer(app->renderer);
        app->renderer = NULL;
    }

    if (app->window) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }

    if (app->flags & APP_F_AUDIOINIT) {
        app->flags &= ~APP_F_AUDIOINIT;
        ma_engine_uninit(&audio_engine);
    }
    if (app->flags & APP_F_TTFINIT) {
        app->flags &= ~APP_F_TTFINIT;
        TTF_Quit();
    }
    if (app->flags & APP_F_SDLINIT) {
        app->flags &= ~APP_F_SDLINIT;
        SDL_Quit();
    }
}
