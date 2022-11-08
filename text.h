#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

SDL_Rect text(SDL_Renderer *renderer, SDL_Color color, TTF_Font *font, char *text, int x, int y);

char *num_to_str(int x);

#endif // TEXT_H_INCLUDED
