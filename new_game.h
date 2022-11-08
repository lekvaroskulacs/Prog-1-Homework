#ifndef NEW_GAME_H_INCLUDED
#define NEW_GAME_H_INCLUDED

//a tábla "kockáinak" típusa
typedef enum Mezo {ures, fekete, feher} Mezo;

//palya adatai
typedef struct P_adat {
    Mezo **board_state;
    int xmeret, ymeret;
    int *kpont_x, *kpont_y;
    char players, current;
} P_adat;

//ablak méretei
typedef struct Window {
    int w, h;
} Window;

void kugli_rajzol(Mezo mode, SDL_Renderer *renderer, P_adat palya, int x, int y);

//létrehozza a pálya helyzetét tároló kétdimenziós tömböt, fel kell szabadítani!!
void init_board(P_adat *palya);

//felszabadítja az init boardban létrehozott tömböt
void free_board(Mezo **board_state);

//a tábla négyzeteinek középpontjainak pixel-koordinátáival tölt fel két tömböt (kpont_x és kpont_y)
//a feltöltendõ tömböket elõre létre kell hozni és átadni neki
void palya_kpont(P_adat *palya, int window_h);

//kirajzolja a táblát
void palya_rajzol(SDL_Renderer *renderer, P_adat palya, Window wind);

//alaphelyzet korongjait jeleníti meg/tárolja el
void alaphelyzet(SDL_Renderer *renderer, P_adat *palya);

//a játék eseményciklusa
bool game_eventcycle(SDL_Renderer *renderer, P_adat *palya, Window wind);


#endif // NEW_GAME_H_INCLUDED
