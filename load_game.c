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

//a c�m szerint �tvett p�lya strukt�r�ba �rja a gamesave.txt nev� f�jl tartalm�t
//(ennek l�teznie kell a save_save f�ggv�nnyel megegyez� form�tumban)
void save_get(P_adat *palya){
    FILE *save = fopen("gamesave.txt", "r");
    //ha nem siker�l megnyitni
    if (save == NULL){
        perror("Nem sikerult megnyitni a mentes fajlt.\nVan mar mentesed?");
        exit(1);
    }
    //m�ret olvas�sa
    fscanf(save, "%d\n%d\n", &palya->xmeret, &palya->ymeret);
    //board_state inicializ�l�sa
    init_board(palya);
    //annak felt�lt�se
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
    //a j�t�kos m�d �s jelenlegi j�t�kos bet�lt�se
    fscanf(save, "%c\n%c", &palya->players, &palya->current);
    fclose(save);
}

//a j�t�k (new game) fut�sa k�zben a p�lya mell� rajzol egy SAVE GAME gombot, aminek helyzet�vel visszat�r
SDL_Rect save_rajzol(SDL_Renderer *renderer, Window wind){
    SDL_Color black = {0, 0, 0, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 50);
    SDL_Rect savegame = text(renderer, black, font, "SAVE GAME", 3*wind.w/4, wind.h/4);
    boxRGBA(renderer, savegame.x-20, savegame.y-20, savegame.x+savegame.w+20, savegame.y+savegame.h+20, 100, 100, 100, 100);
    rectangleRGBA(renderer, savegame.x-20, savegame.y-20, savegame.x+savegame.w+20, savegame.y+savegame.h+20, 0, 0, 0, 255);
    TTF_CloseFont(font);
    return savegame;
}

//elmenti az aktu�lis �llapot�t a j�t�knak egy gamesave.txt f�jlba
//param�terk�nt megkapja a SAVE GAME feliratot, hogy el tudja helyezni al� a visszajelz�st
SDL_Rect save_save(SDL_Renderer *renderer, P_adat palya, SDL_Rect savegame, bool isend){
    if (!isend){
        FILE *save = fopen("gamesave.txt", "w");
        //ha nem siker�l megnyitni
        if (save == NULL){
            perror("Nem sikerult megnyitni a file-t");
            exit(1);
        }
        //p�lya m�ret�nek be�r�sa
        fprintf(save, "%d\n%d\n", palya.xmeret, palya.ymeret);
        //p�lya helyzet�nek ment�se
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
        //j�t�kos m�d �s jelenlegi j�t�kos ment�se
        fprintf(save, "%c\n%c", palya.players, palya.current);
        fclose(save);
    }
    //visszajelz�s a ment�s sikeress�g�r�l
    SDL_Color zold = {0, 255, 0, 255}, piros = {220, 20, 0, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 20);
    SDL_Rect save_success = text(renderer, isend ? piros : zold, font, isend ? "Save Unsuccesful" : "Save Successful", savegame.x+savegame.w/2, savegame.y+savegame.h+50);
    SDL_RenderPresent(renderer);
    TTF_CloseFont(font);
    return save_success;
}

//t�rli az el�z� f�ggv�ny visszajelz�s�t
void success_clear(SDL_Renderer *renderer, SDL_Rect save_success){
    boxRGBA(renderer, save_success.x, save_success.y, save_success.x+save_success.w, save_success.y+save_success.h, 20, 130, 20, 255);
    SDL_RenderPresent(renderer);
}
