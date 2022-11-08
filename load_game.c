#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "load_game.h"
#include "new_game.h"
#include "text.h"
#include "debugmalloc.h"

//a cím szerint átvett pálya struktúrába írja a gamesave.txt nevû fájl tartalmát
//(ennek léteznie kell a save_save függvénnyel megegyezõ formátumban)
void save_get(P_adat *palya){
    FILE *save = fopen("gamesave.txt", "r");
    //ha nem sikerül megnyitni
    if (save == NULL){
        perror("Nem sikerult megnyitni a mentes fajlt.\nVan mar mentesed?");
        exit(1);
    }
    //méret olvasása
    fscanf(save, "%d\n%d\n", &palya->xmeret, &palya->ymeret);
    //board_state inicializálása
    init_board(palya);
    //annak feltöltése
    for (int x = 0; x<palya->xmeret; ++x){
        for (int y = 0; y<palya->ymeret; ++y){
            int cur=0;
            fscanf(save, "%d ", &cur);
            if (cur == 1)
                palya->board_state[y][x] = fekete;
            if (cur == 2)
                palya->board_state[y][x] = feher;
        }
        fscanf(save, "\n");
    }
    //a játékos mód és jelenlegi játékos betöltése
    fscanf(save, "%c\n%c", &palya->players, &palya->current);
    fclose(save);
}

//a játék (new game) futása közben a pálya mellé rajzol egy SAVE GAME gombot, aminek helyzetével visszatér
SDL_Rect save_rajzol(SDL_Renderer *renderer, Window wind){
    SDL_Color black = {0, 0, 0, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 50);
    SDL_Rect savegame = text(renderer, black, font, "SAVE GAME", 3*wind.w/4, wind.h/4);
    boxRGBA(renderer, savegame.x-20, savegame.y-20, savegame.x+savegame.w+20, savegame.y+savegame.h+20, 100, 100, 100, 100);
    rectangleRGBA(renderer, savegame.x-20, savegame.y-20, savegame.x+savegame.w+20, savegame.y+savegame.h+20, 0, 0, 0, 255);
    TTF_CloseFont(font);
    return savegame;
}

//elmenti az aktuális állapotát a játéknak egy gamesave.txt fájlba
//paraméterként megkapja a SAVE GAME feliratot, hogy el tudja helyezni alá a visszajelzést
SDL_Rect save_save(SDL_Renderer *renderer, P_adat palya, SDL_Rect savegame, bool isend){
    if (!isend){
        FILE *save = fopen("gamesave.txt", "w");
        //ha nem sikerül megnyitni
        if (save == NULL){
            perror("Nem sikerult megnyitni a file-t");
            exit(1);
        }
        //pálya méretének beírása
        fprintf(save, "%d\n%d\n", palya.xmeret, palya.ymeret);
        //pálya helyzetének mentése
        for (int x=0; x<palya.xmeret; ++x){
            for (int y=0; y<palya.ymeret; ++y){
                int cur=0;
                if (palya.board_state[y][x]==fekete)
                    cur = 1;
                if (palya.board_state[y][x]==feher)
                    cur = 2;
                fprintf(save, "%d ", cur);
            }
        fprintf(save, "\n");
        }
        //játékos mód és jelenlegi játékos mentése
        fprintf(save, "%c\n%c", palya.players, palya.current);
        fclose(save);
    }
    //visszajelzés a mentés sikerességérõl
    SDL_Color zold = {0, 255, 0, 255}, piros = {220, 20, 0, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 20);
    SDL_Rect save_success = text(renderer, isend ? piros : zold, font, isend ? "Save Unsuccesful" : "Save Successful", savegame.x+savegame.w/2, savegame.y+savegame.h+50);
    SDL_RenderPresent(renderer);
    TTF_CloseFont(font);
    return save_success;
}

//törli az elõzõ függvény visszajelzését
void success_clear(SDL_Renderer *renderer, SDL_Rect save_success){
    boxRGBA(renderer, save_success.x, save_success.y, save_success.x+save_success.w, save_success.y+save_success.h, 20, 130, 20, 255);
    SDL_RenderPresent(renderer);
}
