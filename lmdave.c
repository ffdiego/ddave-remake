/*
Dangerous Dave Remake
Original code by MaiZure 2017
video stopped at 5:28
https://youtu.be/Y3lvCywVG94?list=PLSkJey49cOgTSj465v2KbLZ7LMn10bCF9&t=328
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL.h>
#include "lmdave.h"

int main(int argc, char* argv[])
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  const uint8_t DISPLAY_SCALE = 3;
  struct game_state *game;
  struct game_assets *assets;

  game = malloc(sizeof(struct game_state));
  init_game(game);

  if(SDL_Init(SDL_INIT_VIDEO))
    SDL_Log("SDL Error: %s", SDL_GetError());

  if(SDL_CreateWindowAndRenderer(320*DISPLAY_SCALE, 200*DISPLAY_SCALE, 0, &window, &renderer))
    SDL_Log("Window/Renderer error: %s", SDL_GetError());

  SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

  assets = malloc(sizeof(struct game_assets));
  init_assets(assets, renderer);

  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);

  while(!game->quit){
    check_input(game);
    update_game(game);
    render(game, renderer, assets);
  }

  free(game);
  free(assets);
  
  return 0;
}

void init_game(struct game_state *game) {
  FILE *file_level;
  char fname[13];
  char file_num[4];
  int i,j;

  game->quit=0;
  game->current_level=0;
  game->view_x = 0;
  game->view_y = 0;

  for (j=0;j<10;j++){
    fname[0]='\0';
    strcat(fname,"level");
    sprintf(&file_num[0], "%u", j);
    strcat(fname, file_num);
    strcat(fname, ".dat");

    file_level = fopen(fname, "rb");

    for (i=0;i<sizeof(game->level[j].path);i++)
      game->level[j].path[i] = fgetc(file_level);

    for (i=0;i<sizeof(game->level[j].tiles);i++)
      game->level[j].tiles[i] = fgetc(file_level);

    for (i=0;i<sizeof(game->level[j].padding);i++)
      game->level[j].padding[i] = fgetc(file_level);

    fclose(file_level);
  }
}
void init_assets(struct game_assets *game, SDL_Renderer *renderer){}
void check_input(struct game_state *game){}
void update_game(struct game_state *game){}
void render(struct game_state *game, SDL_Renderer *renderer, struct game_assets *assets){}