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
  uint32_t timer_begin;
  uint32_t timer_end;
  uint32_t delay;
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
    timer_begin = SDL_GetTicks();

    check_input(game);
    update_game(game);
    render(game, renderer, assets);

    timer_end = SDL_GetTicks();

    delay = 33 - (timer_end - timer_begin);
    delay = delay > 33 ? 0 : delay;
    SDL_Delay(10);
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
  game->scroll_x = 0;

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
void init_assets(struct game_assets *assets, SDL_Renderer *renderer){
  int i;
  char fname[13];
  char file_num[4];


  for(i=0;i<158;i++){
    fname[0]='\0';
    strcat(fname,"tile");
    sprintf(&file_num[0], "%u", i);
    strcat(fname, file_num);
    strcat(fname, ".bmp");

    assets->graphics_tiles[i] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP(fname));
  }
}
void check_input(struct game_state *game){
  SDL_Event event;
  SDL_PollEvent(&event);

  if(event.type == SDL_QUIT)
    game->quit = 1;

  if(event.type == SDL_KEYDOWN){
    if(event.key.keysym.sym == SDLK_RIGHT)
      game->scroll_x = 15;
    if(event.key.keysym.sym == SDLK_LEFT)
      game->scroll_x = -15;
    if(event.key.keysym.sym == SDLK_UP)
      game->current_level++;
    if(event.key.keysym.sym == SDLK_DOWN)
      game->current_level--;
  }
}
void update_game(struct game_state *game){
  if(game->current_level == 0xFF) {
    game->current_level = 0;
  }
  if(game->current_level > 9) {
    game->current_level = 9;
  }
  if(game->scroll_x > 0){
    if(game->view_x == 80)
      game->scroll_x =0;
    else {
      game->view_x++;
      game->scroll_x--;
    }
  }
  if(game->scroll_x < 0){
    if(game->view_x == 0)
      game->scroll_x =0;
    else {
      game->view_x--;
      game->scroll_x++;
    }
  }
}

void render(struct game_state *game, SDL_Renderer *renderer, struct game_assets *assets){
  uint8_t i,j;
  SDL_Rect dest;
  uint8_t tile_index;

  for (j = 0; j < 10; j++)
  {
    dest.y = j*16;
    dest.w = 16;
    dest.h = 16;
    for (i = 0; i < 20; i++)
    {
      dest.x = i * 16;
      tile_index = game->level[game->current_level].tiles[j*100+game->view_x+i];
      SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
    }
    SDL_RenderPresent(renderer);
  }
  
}