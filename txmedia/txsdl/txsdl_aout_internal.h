
#ifndef TXSDL__TXSDL_AOUT_INTERNAL_H
#define TXSDL__TXSDL_AOUT_INTERNAL_H

#include "txsdl_mutex.h"
#include "txsdl_aout.h"

inline static SDL_Aout *SDL_Aout_CreateInternal(size_t opaque_size)
{
    SDL_Aout *aout = (SDL_Aout*) mallocz(sizeof(SDL_Aout));
    if (!aout)
        return NULL;

    aout->opaque = mallocz(opaque_size);
    if (!aout->opaque) {
        free(aout);
        return NULL;
    }

    aout->mutex = SDL_CreateMutex();
    if (aout->mutex == NULL) {
        free(aout->opaque);
        free(aout);
        return NULL;
    }

    return aout;
}

inline static void SDL_Aout_FreeInternal(SDL_Aout *aout)
{
    if (!aout)
        return;

    if (aout->mutex) {
        SDL_DestroyMutex(aout->mutex);
    }

    free(aout->opaque);
    memset(aout, 0, sizeof(SDL_Aout));
    free(aout);
}

#endif
