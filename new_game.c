#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "new_game.h"
#include "load_game.h"
#include "text.h"
#include "debugmalloc.h"

//kirajzolja az adott x és y TÁBLA (nem pixel) koordinátában a mode-ban megadott színû korongot (kuglit)
void kugli_rajzol(Mezo mode, SDL_Renderer *renderer, P_adat palya, int x, int y){
    int const negyzet = palya.xmeret>=palya.ymeret ? palya.kpont_x[1]-palya.kpont_x[0] : palya.kpont_y[1] - palya.kpont_y[0];
    if(mode == fekete)
        filledCircleRGBA(renderer, palya.kpont_x[x], palya.kpont_y[y],
                         negyzet/2 - 2, //beleférjen a négyzetbe
                         0, 0, 0, 255);
    if(mode == feher){
        filledCircleRGBA(renderer, palya.kpont_x[x], palya.kpont_y[y],
                         negyzet/2 - 2,
                         255, 255, 255, 255);
        circleRGBA(renderer, palya.kpont_x[x], palya.kpont_y[y],
                   negyzet/2 - 2,
                   0, 0, 0, 255);
    }
}

//meghatározza, hogy a kapott eventben az egér koordináták az x és y ban megadott négyzetben van e a táblán
//igazzal tér vissza ha igen
bool negyzetben(int x, int y, P_adat palya, SDL_Event event){
    int const felnegyzet = palya.xmeret>=palya.ymeret ? (palya.kpont_x[1]-palya.kpont_x[0])/2 : (palya.kpont_y[1] - palya.kpont_y[0])/2;
    return event.button.x < palya.kpont_x[x]+felnegyzet && event.button.x >= palya.kpont_x[x]-felnegyzet
        && event.button.y < palya.kpont_y[y]+felnegyzet && event.button.y >= palya.kpont_y[y]-felnegyzet;
}

//a függvény megvizsgálja, helyes-e a lépés, a paraméterekben kapott x és y TÁBLA(nem pixel) koordinátákban.
//a lépés helyességét a mode-ban kapott játékos szemszögébõl vizsgálja.
//visszatérési értéke igaz vagy hamis, a lépés helyességének megfelelõen.
bool legalis_lepes(int x, int y, P_adat palya, Mezo mode){
    //üres egyáltalán?
    if(palya.board_state[y][x]!=ures)
        return false;
    //az ellentétes korong
    Mezo vizsgalt_ellentet = mode==fekete ? feher : fekete;
    //lépés körüli négyzetek vizsgálata
    for (int x_ellen=x-1; x_ellen<=x+1; ++x_ellen)
        for (int y_ellen=y-1; y_ellen<=y+1; ++y_ellen){
            //pályán kívüli kockát ne vizsgáljunk
            bool palyan_kivul = false;
            if (x_ellen < 0 || x_ellen >= palya.xmeret || y_ellen < 0 || y_ellen >= palya.ymeret)
                palyan_kivul = true;
            //ha ellentétes korongot találunk:
            if (!palyan_kivul && palya.board_state[y_ellen][x_ellen] == vizsgalt_ellentet){
                //megnézzük hogy az ellentétesek irányában van e saját korong
                int x_irany=x_ellen-x, y_irany=y_ellen-y;
                int x_azon=x_ellen+x_irany, y_azon=y_ellen+y_irany;
                //ha van közben üres Mezo, akkor helytelen
                bool talalt_urest = false;
                //ciklus amíg találunk azonost, vagy helytelen a lépés
                while (x_azon>=0 && x_azon<palya.xmeret && y_azon>=0 && y_azon<palya.ymeret && !talalt_urest){
                    //van közben üres Mezo?
                    if(palya.board_state[y_azon][x_azon]==ures)
                        talalt_urest = true;
                    //tehát csak akkor legális, ha van mellette ellentétes színû, ennek az irányában van azonos színû, de nincs köztük üres
                    if(palya.board_state[y_azon][x_azon]==mode && !talalt_urest)
                        return true;
                    //lépünk egyet az irányban
                    x_azon+=x_irany;
                    y_azon+=y_irany;
                }
            }
        }
    //nincs ellentétes színû körülötte
    return false;
}

//az átfordítandó korongokat átfordítja, az adott x y tábla koordinátába lépést követõen, a mode-ban kapott játékos szerint.
//(azaz csak legális lépés után hívjuk meg, aminek a koordinátáit meg kell adni paraméterként), ez a palya értékeit megváltoztathatja
void flip(int x, int y, Mezo mode, SDL_Renderer *renderer, P_adat *palya){
    //az ellentétes korong
    Mezo vizsgalt_ellentet = mode==fekete ? feher : fekete;
    //lépés körüli négyzetek vizsgálata
    for (int x_ellen=x-1; x_ellen<=x+1; ++x_ellen)
        for (int y_ellen=y-1; y_ellen<=y+1; ++y_ellen){
            //pályán kívüli kockát ne vizsgáljunk
            bool palyan_kivul = false;
            if (x_ellen < 0 || x_ellen >= palya->xmeret || y_ellen < 0 || y_ellen >= palya->ymeret)
                palyan_kivul = true;
            //ha találunk ellentétes korongot:
            if (!palyan_kivul && palya->board_state[y_ellen][x_ellen] == vizsgalt_ellentet){
                //keressük meg abban az irányban az azonos színűt
                int x_irany=x_ellen-x, y_irany=y_ellen-y;
                int x_azon=x_ellen, y_azon=y_ellen;
                bool talalt_urest=false;
                bool kesz = false;
                //számoljuk az átfordítandó korongokat
                int db = 0;
                //ciklus amíg meg nem találjuk, vagy kimegyünk a pályáról, vagy üres négyzetbe ütközünk
                while (x_azon>=0 && x_azon<palya->xmeret && y_azon>=0 && y_azon<palya->ymeret && !talalt_urest && !kesz){
                    //ha talál üres négyzetet -> ciklus vége
                    if(palya->board_state[y_azon][x_azon]==ures)
                        talalt_urest = true;
                    //ha megtaláljuk az azonos színût
                    if(palya->board_state[y_azon][x_azon]==mode){
                        //átfordítás: lépegessünk visszafelé, amíg el nem fogy az átfordítandó ellenfél korong
                        int x_fordit=x_azon-x_irany, y_fordit=y_azon-y_irany;
                        while(db!=0){
                            palya->board_state[y_fordit][x_fordit]=mode;
                            kugli_rajzol(mode, renderer, *palya, x_fordit, y_fordit);
                            x_fordit-=x_irany;
                            y_fordit-=y_irany;
                            --db;
                        }
                        kesz=true;
                    }
                    ++db;
                    x_azon+=x_irany;
                    y_azon+=y_irany;
                }
            }
        }
}

//létrehozza a pálya helyzetét tároló kétdimenziós tömböt, fel kell szabadítani!!
void init_board(P_adat *palya){
    palya->board_state = (Mezo**) malloc(palya->ymeret * sizeof(Mezo*));
    //nullákkal legyen feltöltve = üres négyzetekkel
    palya->board_state[0] = (Mezo*) calloc(palya->xmeret * palya->ymeret, sizeof(Mezo));
    for (int y=1; y<palya->ymeret; ++y)
        palya->board_state[y] = palya->board_state[0] + y*palya->xmeret;
}

//felszabadítja az init boardban létrehozott tömböt
void free_board(Mezo **board_state){
    free(board_state[0]);
    free(board_state);
}

//a tábla négyzeteinek középpontjainak pixel-koordinátáival tölt fel két tömböt (kpont_x és y)
//ez a megjelenítéshez használandó, vagyis arra, hogy melyik négyzet hol van az ablakban
void palya_kpont(P_adat *palya, int window_h){
    double const p = window_h; //mindig egy p*p négyzeten belül lesz a palya megjelenitve (=window_h)
        //két eset van: magasabb a pálya mint hosszabb, vagy fordítva (egyenlõségkor akármelyik jó)
        if (palya->xmeret>=palya->ymeret){
            //ilyenkor a négyzeteket a szélesség szerint méretezzük át:
            //olyan négyzetek amiből xpálya darab elfér a p*p négyzetben
            int elem = 0;
            for (double i=p/palya->xmeret/2 ; i<p; i+=p/palya->xmeret)
                palya->kpont_x[elem++] = (int) i;
            elem = 0;
            double const magassag = (p/palya->xmeret) * palya->ymeret;
            for (double i = (p - magassag)/2 + (p/palya->xmeret)/2; i < magassag + (p - magassag)/2; i+=p/palya->xmeret)
                palya->kpont_y[elem++] = (int) i;
        }else if (palya->ymeret>palya->xmeret){
            //itt magasság szerint: olyan négyzetek amik magasságban pont elférnek p pixelen
            int elem = 0;
            for (double i=p/palya->ymeret/2 ; i<p; i+=p/palya->ymeret)
                palya->kpont_y[elem++] = (int) i;
            elem = 0;
            double const hossz = (p/palya->ymeret) * palya->xmeret;
            for (double i = (p - hossz)/2 + (p/palya->ymeret)/2; i<hossz + (p - hossz)/2; i+=p/palya->ymeret)
                palya->kpont_x[elem++] = (int) i;
        }
}

//kirajzolja a táblát
void palya_rajzol(SDL_Renderer *renderer, P_adat palya, Window wind){
    int const negyz_fele = (palya.kpont_x[1]-palya.kpont_x[0])/2; //egy négyzet oldalának a fele
    //ablak háttér
    boxRGBA(renderer, 0, 0, wind.w, wind.h, 20, 130, 20, 255);
    //tábla háttér
    boxRGBA(renderer,   palya.kpont_x[0]-negyz_fele,
                        palya.kpont_y[0]-negyz_fele,
                        palya.kpont_x[palya.xmeret-1]+negyz_fele,
                        palya.kpont_y[palya.ymeret-1]+negyz_fele,
                        0, 255, 0, 255);
    //függõleges vonalak
    for (int x=0; x<palya.xmeret-1; ++x){
        lineRGBA(renderer,  palya.kpont_x[x]+negyz_fele,
                            palya.kpont_y[0]-negyz_fele,
                            palya.kpont_x[x]+negyz_fele,
                            palya.kpont_y[palya.ymeret-1]+negyz_fele,
                            0, 0, 0, 255);
    }
    //vízszintes vonalak
    for (int y=0; y<palya.ymeret-1; ++y){
        lineRGBA(renderer,  palya.kpont_x[0]-negyz_fele,
                            palya.kpont_y[y]+negyz_fele,
                            palya.kpont_x[palya.xmeret-1]+negyz_fele,
                            palya.kpont_y[y]+negyz_fele,
                            0, 0, 0, 255);
    }
    //szélén vastag vonalak(5 négyzet elcsúsztatva)
    SDL_Rect r;
    for (int i=0; i<5; i++){
        r.h = palya.kpont_y[palya.ymeret-1] - palya.kpont_y[0] + 2*negyz_fele-2*i+1;
        r.w = palya.kpont_x[palya.xmeret-1] - palya.kpont_x[0] + 2*negyz_fele-2*i+1;
        r.x = palya.kpont_x[0]-negyz_fele+i;
        r.y = palya.kpont_y[0]-negyz_fele+i;
    SDL_RenderDrawRect(renderer, &r);
    }
}

//alaphelyzet korongjait jeleníti meg és tárolja el
void alaphelyzet(SDL_Renderer *renderer, P_adat *palya){
    int szin = 1;
    for (int i = palya->xmeret/2-1; i<=palya->xmeret/2; ++i){
        for (int j = palya->ymeret/2-1; j<=palya->ymeret/2; ++j){
            //minden második legyen fekete
            palya->board_state[j][i] = szin%2!=0 ? fekete : feher;
            kugli_rajzol(szin%2!=0 ? fekete : feher, renderer, *palya, i, j);
            ++szin;
        }
    ++szin;
    }
}

//tele van e a palya
bool board_full(P_adat palya){
    for (int x=0; x<palya.xmeret; ++x)
        for (int y=0; y<palya.ymeret; ++y)
            if (palya.board_state[y][x]==ures)
                return false;
    return true;
}

//megnézi van a lépés a pályán a mode-ban kapott játékos szerint
bool vanlepes(P_adat palya, Mezo mode){
    for (int x=0; x<palya.xmeret; ++x)
        for (int y=0; y<palya.ymeret; ++y)
            if (legalis_lepes(x, y, palya, mode))
                return true;
    return false;
}

//elvégzi egy mode-ban kapott játékos lépésekor a teendőket
//megjelenítéshez kapja a renderert, az eventet pedig a kattintás vizsgálatához
//igazzal tér vissza, ha sikerült lépni (jó helyre kattintottunk), hamissal ha nem történt meg
bool p_lepes(SDL_Renderer *renderer, P_adat *palya, SDL_Event event, Mezo mode){
    for (int x=0; x<palya->xmeret; ++x){
        for (int y=0; y<palya->ymeret; ++y){
            //ha ebbe az (x,y) tábla négyzetbe kattintottunk és legális a lépés:
            if (negyzetben(x, y, *palya, event)
                && legalis_lepes(x, y, *palya, mode)) {
                    palya->board_state[y][x]=mode;
                    kugli_rajzol(mode, renderer, *palya, x, y);
                    flip(x, y, mode, renderer, palya);
                    SDL_RenderPresent(renderer);
                    return true;
                }
        }
    }
    return false;
}

//a jelenlegi fekete és fehér korongok számát jelzi ki, amihez megkapja a renderert és a a windet, és a pálya helyzetét
void szamlalo(SDL_Renderer *renderer, P_adat palya, Window wind){
    //előző felirat törlése
    boxRGBA(renderer, wind.w/2 + 100, wind.h - wind.h/8 -50, wind.w /2 + 400, wind.h, 20, 130, 20, 255);
    SDL_Color feh = {255, 255, 255, 255}, fek = {0, 0, 0, 255};
    //fekete és fehér bábuk száma
    int b = 0, w = 0;
    //számolás
    for (int x=0; x<palya.xmeret; ++x)
        for (int y=0; y<palya.ymeret; ++y){
            if (palya.board_state[y][x] == fekete)
                ++b;
            if (palya.board_state[y][x] == feher)
                ++w;
        }
    TTF_Font *font = TTF_OpenFont("cour.ttf", 50);
    //feketék megjelenítése
    char *black = num_to_str(b);
    text(renderer, fek, font, black, wind.w/2 + 200, wind.h - wind.h/8);
    free(black);
    //fehérek megjelenítése
    char *white = num_to_str(w);
    text(renderer, feh, font, white, wind.w/2 + 300, wind.h - wind.h/8);
    free(white);
    TTF_CloseFont(font);
    SDL_RenderPresent(renderer);
}


//a gép lépése, amihez megkapja a pálya helyzetét és a renderert
void ai_lepes(SDL_Renderer *renderer, P_adat *palya){
    int ai_lepes_y = -1, ai_lepes_x = -1;
    //ha a sarok ures, oda lepjen
    for (int x=0; x<palya->xmeret; x += palya->ymeret-1)
        for (int y=0; y<palya->ymeret; y += palya->ymeret-1)
            if (legalis_lepes(x, y, *palya, feher))
                ai_lepes_x = x, ai_lepes_y = y;
    //ha meg nem talalt lepest
    if (ai_lepes_x == -1){
        //probaljon meg a szelso sorba/oszlopba lepni, ami nem a sarok vagy a sarok melletti
        for (int x=2; x<=palya->xmeret - 3; ++x)
            for (int y=0; y<=palya->ymeret; y += palya->ymeret-1)
                if (legalis_lepes(x, y, *palya, feher))
                    ai_lepes_x = x, ai_lepes_y = y;
        for (int y=2; y<=palya->ymeret - 3; ++y)
            for (int x=0; x<=palya->xmeret; x += palya->xmeret-1)
                if (legalis_lepes(x, y, *palya, feher))
                    ai_lepes_x = x, ai_lepes_y = y;
    }
    //ha meg nem talalt lepest
    int hiba = 0;
    if (ai_lepes_x == -1 && palya->xmeret>4 && palya->ymeret>4){
        //probaljon meg ne a szelsobe de ne is a szelso mellettibe lepni (random)
        ai_lepes_x = rand()%(palya->xmeret-4)+2;
        ai_lepes_y = rand()%(palya->ymeret-4)+2;
        while (!(legalis_lepes(ai_lepes_x, ai_lepes_y, *palya, feher)) && hiba != 100){
            ai_lepes_x = rand()%(palya->xmeret-4)+2;
            ai_lepes_y = rand()%(palya->ymeret-4)+2;
            ++hiba;
        }
    }
    if (hiba == 100 || (palya->ymeret == 4 && ai_lepes_y == -1) || (palya->xmeret == 4 && ai_lepes_x == -1)){
        //ha igy sincs lepes akkor lepjen random
        ai_lepes_x = rand()%palya->xmeret;
        ai_lepes_y = rand()%palya->ymeret;
        while (!(legalis_lepes(ai_lepes_x, ai_lepes_y, *palya, feher))){
            ai_lepes_x = rand()%palya->xmeret;
            ai_lepes_y = rand()%palya->ymeret;
        }
    }

    palya->board_state[ai_lepes_y][ai_lepes_x]=feher;
    kugli_rajzol(feher, renderer, *palya, ai_lepes_x, ai_lepes_y);
    flip(ai_lepes_x, ai_lepes_y, feher, renderer, palya);
    SDL_RenderPresent(renderer);
}

//back felirat kirajzolása, visszatér a felirat helyével egy négyzetként
SDL_Rect back_rajzol(SDL_Renderer *renderer, Window wind){
    SDL_Color white = {255, 255, 255, 255};
    TTF_Font *font = TTF_OpenFont("cour.ttf", 30);
    SDL_Rect back = text(renderer, white, font, "Back", wind.w-wind.w/8, wind.h-wind.h/8);
    SDL_RenderPresent(renderer);
    TTF_CloseFont(font);
    return back;
}

//a nyertes kijelzése, a játék végén hívjuk meg
void endscreen(SDL_Renderer *renderer, P_adat palya, Window wind){
    //korongok megszámolása
    int b = 0, w = 0;
    for (int x=0; x<palya.xmeret; ++x)
        for (int y=0; y<palya.ymeret; ++y){
            if (palya.board_state[y][x] == fekete)
                ++b;
            if (palya.board_state[y][x] == feher)
                ++w;
        }
    char vict[20];
    //ha egyenlő, akkor döntetlen
    if (b == w)
        strcpy(vict, "TIE");
    //amúgy fekete vagy fehér nyert
    else if (b>w)
            strcpy(vict, "BLACK VICTORY");
        else
            strcpy(vict, "WHITE VICTORY");
    //felirat, stb. megjelenítése
    TTF_Font *font = TTF_OpenFont("cour.ttf", 100);
    SDL_Color black = {0, 0, 0, 255};
    SDL_Rect victory = text(renderer, black, font, vict, wind.w/2, wind.h/2);
    boxRGBA(renderer, victory.x-5, victory.y-5, victory.x+victory.w+5, victory.y+victory.h+5, 0, 0, 0, 255);
    boxRGBA(renderer, victory.x, victory.y, victory.x+victory.w, victory.y+victory.h, 230, 230, 230, 255);
    text(renderer, black, font, vict, wind.w/2, wind.h/2);
    TTF_CloseFont(font);
    SDL_RenderPresent(renderer);
}

//jelenlegi játékos kijelzése egy megfelelő színű koronggal
void turn_disp(SDL_Renderer *renderer, bool lepes1, Window wind){
    //előző törlése
    boxRGBA(renderer, wind.w/2 + 180, wind.h - wind.h/8-100, wind.w/2 +320, wind.h - wind.h/8-60, 20, 130, 20, 255);
    //ha az első lépés aktív akkor fekete
    if (lepes1){
        filledCircleRGBA(renderer, wind.w/2 + 200, wind.h - wind.h/8-80, 20, 0, 0, 0, 255);
    }
    //ha a második (nem az első) akkor fehér
    else {
        filledCircleRGBA(renderer, wind.w/2 + 300, wind.h - wind.h/8-80, 20, 0, 0, 0, 255);
        filledCircleRGBA(renderer, wind.w/2 + 300, wind.h - wind.h/8-80, 19, 255, 255, 255, 255);
    }
    SDL_RenderPresent(renderer);
}

//a játék eseményciklusa
bool game_eventcycle(SDL_Renderer *renderer, P_adat *palya, Window wind){
    //inicializálás
    SDL_Rect savegame = save_rajzol(renderer, wind);
    SDL_Rect save_success;
    SDL_Rect back = back_rajzol(renderer, wind);
    bool egyjatekos = palya->players == 's';
    bool lepes1=palya->current == 'b', lepes2=palya->current == 'w';
    szamlalo(renderer, *palya, wind);
    bool isend = false;
    while (true){
        //ha end eléri kettőt, vége
        int end = 0;
        //ha már vége van, akkor ne vizsgálja, hogy éppen kinek a köre van
        if (!isend){
            if (lepes1 && !vanlepes(*palya, fekete)){
                //ha nincsen  szabályos lépés, egyel közelebb a végéhez
                ++end;
                lepes1 = false;
                lepes2 = true;
                turn_disp(renderer, lepes1, wind);
            }
            if (lepes2 && !vanlepes(*palya, feher)){
                ++end;
                lepes2 = false;
                lepes1 = true;
                turn_disp(renderer, lepes1, wind);
            }
            if (egyjatekos && lepes2){
                if (vanlepes(*palya, feher)){
                    SDL_Delay(500);
                    ai_lepes(renderer, palya);
                    szamlalo(renderer, *palya, wind);
                } else
                    ++end;
                lepes2 = false;
                lepes1 = true;
                turn_disp(renderer, lepes1, wind);
            }
        }
        if (end==2 || board_full(*palya)){
            endscreen(renderer, *palya, wind);
            isend = true;
        }

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            //kattintás esetén
            case SDL_MOUSEBUTTONDOWN:
                //bal klikk esetén
                if (event.button.button==SDL_BUTTON_LEFT) {
                    if (lepes1){
                        if (vanlepes(*palya, fekete)){
                            if (p_lepes(renderer, palya, event, fekete)){
                                lepes1 = false;
                                lepes2 = true;
                                szamlalo(renderer, *palya, wind);
                                turn_disp(renderer, lepes1, wind);
                            }
                        }
                    }
                    if (!egyjatekos && lepes2){
                        if (vanlepes(*palya, feher)){
                            if (p_lepes(renderer, palya, event, feher)){
                                lepes2 = false;
                                lepes1 = true;
                                szamlalo(renderer, *palya, wind);
                                turn_disp(renderer, lepes1, wind);
                            }
                        }
                    }
                success_clear(renderer, save_success);
                if (event.button.x < savegame.x+savegame.w+20 && event.button.x > savegame.x-20 &&
                    event.button.y < savegame.y+savegame.h+20 && event.button.y > savegame.y-20)
                        save_success = save_save(renderer, *palya, savegame, isend);
                if (event.button.x < back.x+back.w && event.button.x > back.x && event.button.y < back.y+back.h && event.button.y > back.y)
                    return false;
                }
                break;
            case SDL_QUIT:
                return true;
        }
    }
}
