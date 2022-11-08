#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "new_game.h"
#include "load_game.h"
#include "text.h"
#include "debugmalloc.h"

typedef struct Settings {
    int xsize, ysize;
    char jatekosok;
} Settings;

void sdl_init(char const *felirat, int szeles, int magas, SDL_Window **pwindow, SDL_Renderer **prenderer);

void fomenu_rajzol(SDL_Renderer *renderer, Window wind);

bool new_game(SDL_Renderer *renderer, Settings set, Window wind);

bool load_game(SDL_Renderer *renderer, Window wind);

bool options(SDL_Renderer *renderer, Settings *set, Window wind);

void fomenu_kattintas(SDL_Renderer *renderer, Window window);

int main(int argc, char *argv[]) {
    Window wind;
    wind.w = 1280, wind.h = 720;
    SDL_Window *window;
    SDL_Renderer *renderer;
    sdl_init("Reversi", wind.w, wind.h, &window, &renderer);
    TTF_Init();

    fomenu_rajzol(renderer, wind);
    fomenu_kattintas(renderer, wind);

    SDL_Quit();

    return 0;
}

void sdl_init(char const *felirat, int szeles, int magas, SDL_Window **pwindow, SDL_Renderer **prenderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Nem indithato az SDL: %s", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow(felirat, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, szeles, magas, 0);
    if (window == NULL) {
        SDL_Log("Nem hozhato letre az ablak: %s", SDL_GetError());
        exit(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Nem hozhato letre a megjelenito: %s", SDL_GetError());
        exit(1);
    }
    SDL_RenderClear(renderer);

    *pwindow = window;
    *prenderer = renderer;
}



//kirajzolja a fõmenüt, a renderer és az ablak méretei segítségével
void fomenu_rajzol(SDL_Renderer *renderer, Window wind){
    //ablak háttere
    boxRGBA(renderer, 0, 0, wind.w, wind.h, 60, 60, 60, 255);
    //egy betûtípus inicializálása
    TTF_Font *font = TTF_OpenFont("cour.ttf",50);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    //színek amire szükség lesz
    SDL_Color white = {255, 255, 255, 255}, black = {0, 0, 0, 255};
    //a fehér színû "Reversi" felirat
    SDL_Rect title = text(renderer, white, font, "Reversi", wind.w/2+20, wind.h/15);
    //a Reversi felirat melletti logo (korongok)
    filledCircleRGBA(renderer, title.x-19, 56, 10, 255, 255, 255, 255);
    filledCircleRGBA(renderer, title.x-19, 34, 10, 0, 0, 0, 255);
    filledCircleRGBA(renderer, title.x-41, 34, 10, 255, 255, 255, 255);
    filledCircleRGBA(renderer, title.x-41, 56, 10, 0, 0, 0, 255);
    //a betûtípus megmarad, csak lekicsinyítve
    font = TTF_OpenFont("cour.ttf", 30);
    //A menü opciók dobozai és feliratai
    for (int i=0; i<3; ++i){
        boxRGBA(renderer, wind.w/2-wind.w/8, wind.h/4+i*(wind.h/12), wind.w/2+wind.w/8, (wind.h/4+40)+i*(wind.h/12), 230, 230, 230, 255);
        text(renderer, black, font, i==0 ? "NEW GAME" : i==1 ? "LOAD GAME" : "OPTIONS", wind.w/2, (wind.h/4+20)+i*(wind.h/12));
    }
    //a "QUIT" opciónak külön; ez lejjebb van mint a többi
    boxRGBA(renderer, wind.w/2-wind.w/8, wind.h-wind.h/5, wind.w/2+wind.w/8,wind.h-wind.h/5+40, 230, 230, 230, 255);
    text(renderer, black, font, "QUIT", wind.w/2, wind.h-wind.h/5+20);
    //megjelenítés
    SDL_RenderPresent(renderer);
    TTF_CloseFont(font);
}

//a new game modul vezérlése, megkapja a renderert, a kiválasztott beállításokat, és az ablak méreteit
bool new_game(SDL_Renderer *renderer, Settings set, Window wind){
    //pálya méretei
    P_adat palya;
    palya.players = set.jatekosok;
    palya.current = 'b';
    palya.xmeret = set.xsize, palya.ymeret = set.ysize;
    //létrehozzuk a pálya helyzetét tároló tömböt
    init_board(&palya);
    //létrehozzuk a tábla négyzeteinek koordinátáit megjegyzõ tömböt
    palya.kpont_x = (int*) malloc(palya.xmeret*sizeof(int));
    palya.kpont_y = (int*) malloc(palya.ymeret*sizeof(int));
    palya_kpont(&palya, wind.h);
    //pálya kirajzolása
    palya_rajzol(renderer, palya, wind);
    //alaphelyzet beállítása
    alaphelyzet(renderer, &palya);
    //a save game gomb rajzolása
    save_rajzol(renderer, wind);
    SDL_RenderPresent(renderer);
    //a játék eseményciklusa
    bool quit = game_eventcycle(renderer, &palya, wind);
    free_board(palya.board_state);
    free(palya.kpont_x);
    free(palya.kpont_y);
    return quit;
}


//betölti a gamesave.txt fájlba mentett állást, megkapja a renderert, az ablak méreteit
bool load_game(SDL_Renderer *renderer, Window wind){
    //pálya inicializálása
    P_adat palya;
    save_get(&palya);
    palya.kpont_x = (int*) malloc(palya.xmeret*sizeof(int));
    palya.kpont_y = (int*) malloc(palya.ymeret*sizeof(int));
    palya_kpont(&palya, wind.h);
    //pálya kirajzolása
    palya_rajzol(renderer, palya, wind);
    //megjelenítés
    for (int x=0; x<palya.xmeret; ++x)
        for (int y=0; y<palya.ymeret; ++y)
            if (palya.board_state[y][x]!=ures)
                kugli_rajzol(palya.board_state[y][x]==fekete ? fekete : feher, renderer, palya, x, y);
    //a save game gomb rajzolása
    save_rajzol(renderer, wind);
    SDL_RenderPresent(renderer);
    //a játék eseményciklusa
    bool quit = game_eventcycle(renderer, &palya, wind);
    free_board(palya.board_state);
    free(palya.kpont_x);
    free(palya.kpont_y);
    return quit;
}

//az options-ben használatos, megváltoztatja a pálya struktúrájában
//a pálya méretét, és ki is jelzi az aktuális értéket
//paraméterek:  renderer és font a megjelenítéshez
//              a size(fel vagy le) gomb tájékozódáshoz
//              fel_le, hogy növelünk vagy csökkentünk, lehet +2, -2 vagy 0 (nem változik)
//              és x_y, hogy a pálya melyik méretét változtatjuk, lehet 'x' vagy 'y'
void size_changer(SDL_Renderer *renderer, SDL_Rect size, Settings *set, int fel_le, char x_y){
    SDL_Color white = {255, 255, 255, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 30);
    if (x_y == 'x')
        set->xsize += fel_le;
    else
        set->ysize += fel_le;
    boxRGBA(renderer, size.x-175, size.y + (x_y == 'y' ? 50 : 0), size.x-25, size.y+size.h + (x_y == 'y' ? 50 : 0), 60, 60, 60, 255);
    char *szam = num_to_str(x_y == 'x' ? set->xsize : set->ysize);
    text(renderer, white, font, szam, size.x-100, size.y+size.h/2 + (x_y == 'y' ? 50 : 0));
    free(szam);
    TTF_CloseFont(font);
}


//megjelenítést megegyszerűsítő struktúrák
typedef struct BsizeDisp {
    SDL_Rect bsize, size_up, size_down;
} BsizeDisp;

typedef struct PlayersDisp {
    SDL_Rect players, p_down, p_up;
} PlayersDisp;


//főként az options kinézetét megjelenítő függvény, ami paraméterként kapja cím szerint az előbbi struktúrákat,
//aminek az értékeit vissza kell adnia a hívónak. A renderer, font, wind a megjelenítéshez
void opt_setup(SDL_Renderer *renderer, BsizeDisp *b, PlayersDisp *p, Settings *set, Window wind){
    SDL_Color white = {255, 255, 255, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 30);
    //a Board Size felirat, size_down és size_up gomb helyét tároló SDL_Rect-ek létrehozása
    b->bsize = text(renderer, white, font, "Board Size:", wind.w/2-wind.w/4, 3*wind.h/8);
    b->size_down.x = b->bsize.x + b->bsize.w + 100;
    b->size_down.y = b->size_up.y = b->bsize.y;
    b->size_down.w = b->size_down.h = b->size_up.w = b->size_up.h = b->bsize.h;
    b->size_up.x = b->size_down.x + b->size_down.w + 200;
    //size_down, size_up gombok megjelenítése
    boxRGBA(renderer, b->size_down.x, b->size_down.y, b->size_down.x + b->size_down.w, b->size_down.y + b->size_down.h, 180, 180, 180, 255);
    boxRGBA(renderer, b->size_up.x, b->size_up.y, b->size_up.x + b->size_up.w, b->size_up.y + b->size_up.h, 180, 180, 180, 255);
    boxRGBA(renderer, b->size_down.x, b->size_down.y+50, b->size_down.x + b->size_down.w, b->size_down.y + b->size_down.h + 50, 180, 180, 180, 255);
    boxRGBA(renderer, b->size_up.x, b->size_up.y + 50, b->size_up.x + b->size_up.w, b->size_up.y + b->size_up.h + 50, 180, 180, 180, 255);
    //jelenlegi pályaméretek szöveges megjelenítése, nem változtatunk
    size_changer(renderer, b->size_up, set, 0, 'x');
    size_changer(renderer, b->size_up, set, 0, 'y');

    //Players felirat és Single vagy Multi módot váltó gombok helyét tároló SDL_Rectek
    p->players = text(renderer, white, font, "Players:", wind.w/2-wind.w/4, 5*wind.h/8);
    p->p_down.x = b->bsize.x + b->bsize.w + 100;
    p->p_down.y = p->p_up.y = p->players.y;
    p->p_down.w = p->p_down.h = p->p_up.w = p->p_up.h = p->players.h;
    p->p_up.x = p->p_down.x + p->p_down.w + 200;
    //gombok megjelenítése
    boxRGBA(renderer, p->p_down.x, p->p_down.y, p->p_down.x + p->p_down.w, p->p_down.y + p->p_down.h, 180, 180, 180, 255);
    boxRGBA(renderer, p->p_up.x, p->p_up.y, p->p_up.x + p->p_up.w, p->p_up.y + p->p_up.h, 180, 180, 180, 255);
    //gombokban a nyilak megjelenítése
    for (int i = 0; i<3; ++i){
        if (i<2){
            filledTrigonRGBA(renderer, b->size_down.x+b->size_down.w/10, b->size_down.y+b->size_down.h/2+i*50,
                                 b->size_down.x+b->size_down.w*9/10, b->size_down.y+b->size_down.h/10+i*50,
                                 b->size_down.x+b->size_down.w*9/10, b->size_down.y+b->size_down.h*9/10+i*50,
                                 60, 60, 60, 255);
            filledTrigonRGBA(renderer, b->size_up.x+b->size_up.w*9/10, b->size_up.y+b->size_up.h/2+i*50,
                                 b->size_up.x+b->size_up.w/10, b->size_up.y+b->size_up.h/10+i*50,
                                 b->size_up.x+b->size_up.w/10, b->size_up.y+b->size_up.h*9/10+i*50,
                                 60, 60, 60, 255);
        }
        else {
            filledTrigonRGBA(renderer, p->p_down.x+p->p_down.w/10, p->p_down.y+p->p_down.h/2,
                                 p->p_down.x+p->p_down.w*9/10, p->p_down.y+p->p_down.h/10,
                                 p->p_down.x+p->p_down.w*9/10, p->p_down.y+p->p_down.h*9/10,
                                 60, 60, 60, 255);
            filledTrigonRGBA(renderer, p->p_up.x+p->p_up.w*9/10, p->p_up.y+p->p_up.h/2,
                                 p->p_up.x+p->p_up.w/10, p->p_up.y+p->p_up.h/10,
                                 p->p_up.x+p->p_up.w/10, p->p_up.y+p->p_up.h*9/10,
                                 60, 60, 60, 255);
        }
    }
    //jelenlegi játékos mód kiírása
    text(renderer, white, font, set->jatekosok == 's' ? "Single" : "Multi", p->p_up.x-100, p->p_up.y+p->p_up.h/2);
    TTF_CloseFont(font);

}

//Az options menüpontban vezérli a kattintásokat, eseményeket.
//paraméterei a renderer és wind a megjelenítéshez, és egy Settings struktúra, amit cím szerint kap
bool options(SDL_Renderer *renderer, Settings *set, Window wind){
    //Options felirat
    SDL_Color white = {255, 255, 255, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 60);
    boxRGBA(renderer, 0, 0, wind.w, wind.h, 60, 60, 60, 255);
    text(renderer, white, font, "OPTIONS", wind.w/2, wind.h/8);
    //Back felirat
    font = TTF_OpenFont("cour.ttf", 30);
    SDL_Rect back = text(renderer, white, font, "Back", wind.w-wind.w/8, wind.h-wind.h/8);
    //a beállítások képernyőn lévő helyeit tároló tömbök létrehozása
    BsizeDisp b; PlayersDisp p;
    //függvény hívásával ezeket feltöltjük, megjelenítjük őket
    opt_setup(renderer, &b, &p, set, wind);

    while (true){
        SDL_Event event;
        //várjuk a kattintást
        SDL_WaitEvent(&event);
        switch (event.type){
            case SDL_MOUSEBUTTONDOWN:
                //ha a back-re kattintunk vissza a főmenübe
                if (event.button.x < back.x+back.w && event.button.x > back.x && event.button.y < back.y+back.h && event.button.y > back.y)
                    return false;

                if (event.button.x < b.size_down.x+b.size_down.w && event.button.x > b.size_down.x){
                    //x lefelé
                    if (event.button.y < b.size_down.y + b.size_down.h && event.button.y > b.size_down.y && set->xsize > 4){
                        size_changer(renderer, b.size_up, set, -2, 'x');
                    }
                    //y lefelé
                    if (event.button.y < b.size_down.y + b.size_down.h + 50 && event.button.y > b.size_down.y +50 && set->ysize > 4){
                        size_changer(renderer, b.size_up, set, -2, 'y');
                    }
                }
                if (event.button.x < b.size_up.x+b.size_up.w && event.button.x > b.size_up.x){
                    //x felfelé
                    if (event.button.y < b.size_up.y+b.size_up.h && event.button.y > b.size_up.y && set->xsize < 50){
                        size_changer(renderer, b.size_up, set, 2, 'x');
                    }
                    //y felfelé
                    if (event.button.y < b.size_up.y+b.size_up.h +50 && event.button.y > b.size_up.y + 50 && set->ysize < 50){
                        size_changer(renderer, b.size_up, set, 2, 'y');
                    }
                }
                //player "le" vagy "fel" (ugyanaz történik)
                if ((event.button.x < p.p_down.x+p.p_down.w && event.button.x > p.p_down.x && event.button.y < p.p_down.y+p.p_down.h && event.button.y > p.p_down.y) ||
                    (event.button.x < p.p_up.x+p.p_up.w && event.button.x > p.p_up.x && event.button.y < p.p_up.y+p.p_up.h && event.button.y > p.p_up.y)){
                    set->jatekosok = set->jatekosok == 's' ? 'm' : 's';
                    boxRGBA(renderer, p.p_up.x-175, p.p_up.y, p.p_up.x-25, p.p_up.y+p.p_up.h, 60, 60, 60, 255);
                    text(renderer, white, font, set->jatekosok == 's' ? "Single" : "Multi", p.p_up.x-100, p.p_up.y+p.p_up.h/2);
                }
            break;
            case SDL_QUIT:
                return true;
        }
        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(font);
}



//a menü vezérlõegysége, a kattintás vizsgálatához tudnia kell az ablak méreteit
void fomenu_kattintas(SDL_Renderer *renderer, Window wind){
    Settings set;
    set.xsize = set.ysize = 8;
    set.jatekosok = 's';
    bool quit = false;
    while (!quit){
        SDL_Event event;
        //várjuk a kattintást
        SDL_WaitEvent(&event);

        switch (event.type){
            case SDL_MOUSEBUTTONDOWN:
                //megvizsgáljuk, hogy melyik menü opció dobozába kattintottunk
                //amikor visszatérünk a meghívott függvényekből, újra menü rajzolás
                if(event.button.x < wind.w/2+wind.w/8 && event.button.x > wind.w/2-wind.w/8){
                    if (event.button.y < (wind.h/4+40) && event.button.y > wind.h/4){
                        quit = new_game(renderer, set, wind);
                        fomenu_rajzol(renderer, wind);
                    }
                    if (event.button.y < (wind.h/4+40)+(wind.h/12) && event.button.y > wind.h/4+(wind.h/12)){
                        quit = load_game(renderer, wind);
                        fomenu_rajzol(renderer, wind);
                    }
                    if (event.button.y < (wind.h/4+40)+2*(wind.h/12) && event.button.y > wind.h/4+2*(wind.h/12)){
                        quit = options(renderer, &set, wind);
                        fomenu_rajzol(renderer, wind);
                    }
                    if (event.button.y < wind.h-wind.h/5+40 && event.button.y > wind.h-wind.h/5)
                        quit=true;
                }
                break;
            case SDL_QUIT:
                quit=true;
        }
        SDL_RenderPresent(renderer);
    }
    //a bezárás kezelése a main dolga, visszatérünk
    return;
}
