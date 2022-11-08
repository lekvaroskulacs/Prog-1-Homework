#ifndef LOAD_GAME_H_INCLUDED
#define LOAD_GAME_H_INCLUDED

#include "new_game.h"

void save_get(P_adat *palya);

SDL_Rect save_rajzol(SDL_Renderer *renderer, Window wind);

SDL_Rect save_save(SDL_Renderer *renderer, P_adat palya, SDL_Rect savegame, bool isend);

void success_clear(SDL_Renderer *renderer, SDL_Rect save_success);

#endif // LOAD_GAME_H_INCLUDED
