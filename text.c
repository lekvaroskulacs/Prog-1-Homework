#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "text.h"
#include "debugmalloc.h"

//létrehoz egy szöveget a megadott fonttal és színnel a megadott koordinátákban.
//visszatér a szövegdobozzal, amibõl késõbb tudhatjuk, hol van a szöveg
SDL_Rect text(SDL_Renderer *renderer, SDL_Color color, TTF_Font *font, char *text, int x, int y){
    SDL_Surface *felirat;
    SDL_Texture *felirat_t;
    SDL_Rect hova = {0, 0, 0, 0};

    felirat = TTF_RenderUTF8_Blended(font, text, color);
    felirat_t = SDL_CreateTextureFromSurface(renderer, felirat);
    hova.x = x - felirat->w/2;
    hova.y = y - felirat->h/2;
    hova.w = felirat->w;
    hova.h = felirat->h;
    SDL_RenderCopy(renderer, felirat_t, NULL, &hova);
    SDL_FreeSurface(felirat);
    SDL_DestroyTexture(felirat_t);
    return hova;
}
//egy paraméterben kapott számot sztringként ad vissza (dinamikus tömbben!)
char *num_to_str(int x){
    int digit = 0;
    int temp = x;
    if (x == 0)
        digit = 1;
    else
        while (temp!=0){
            temp/=10;
            ++digit;
        }
    char *str = (char*) malloc((digit+1) * sizeof(char));
    str[digit] = 0;
    for (int i=digit-1; i >= 0; --i){
        str[i] = '0' + (x) % 10;
        x/=10;
    }
    return str;
}
