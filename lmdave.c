/*
Dangerous Dave Remake
Original code by MaiZure 2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL.h>
#include "lmdave.h"

const uint8_t TILE_SIZE = 16;

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

  while(!game->quit){
    timer_begin = SDL_GetTicks();

    check_input(game);
    update_game(game);
    render(game, renderer, assets);

    timer_end = SDL_GetTicks();

    delay = 16 - (timer_end - timer_begin);
    delay = delay > 33 ? 0 : delay;
    SDL_Delay(delay);
  }

  SDL_Quit();
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
  game->dave_x = 2;
  game->dave_y = 8;
  game->dave_px = game->dave_x * TILE_SIZE;
  game->dave_py = game->dave_y * TILE_SIZE;
  game->jump_timer = 0;
  game->on_ground = 1;
  game->try_right = 0;
  game->try_left = 0;
  game->try_jump = 0;
  game->dave_right = 0;
  game->dave_left = 0;
  game->dave_jump = 0;
  game->time_scale = 1;
  game->check_pickup_x = 0;
  game->check_pickup_y = 0;


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
  int i, j;
  char fname[13];
  char file_num[4];
  char mname[13];
  char mask_num[4];
  SDL_Surface *surface;
  SDL_Surface *mask;
  uint8_t mask_offset;
  uint8_t *surf_p;
  uint8_t *mask_p;


  for(i=0;i<158;i++){
    fname[0]='\0';
    strcat(fname,"tile");
    sprintf(&file_num[0], "%u", i);
    strcat(fname, file_num);
    strcat(fname, ".bmp");

    if((i>=53 && i<= 59) || i == 67 || i == 68 || (i>=71 && i<=73) || (i>=77 && i<=82))
    {
      if(i>=53 && i<=59)
        mask_offset = 7;
      if(i>=67 && i<=68)
        mask_offset = 2;
      if(i>=71 && i<=73)
        mask_offset = 3;
      if(i>=77 && i<=82)
        mask_offset = 6;

      mname[0]='\0';
      strcat(mname,"tile");
      sprintf(&mask_num[0], "%u", i + mask_offset);
      strcat(mname, mask_num);
      strcat(mname, ".bmp");

      surface = SDL_LoadBMP(fname);
      mask = SDL_LoadBMP(mname);

      surf_p = (uint8_t*)surface->pixels;
      mask_p = (uint8_t*)mask->pixels;

      for (j=0; j< (mask->pitch*mask->h); j++) 
        surf_p[j] = mask_p[j] ? 0xFF : surf_p[j];

      SDL_SetColorKey(surface, 1, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
      assets->graphics_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);
      SDL_FreeSurface(surface);
      SDL_FreeSurface(mask);
    } 
    else 
      assets->graphics_tiles[i] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP(fname));

  }
}

void check_input(struct game_state *game){
  SDL_Event event;
  SDL_PollEvent(&event);
  const uint8_t *keystate = SDL_GetKeyboardState(NULL);
  if(keystate[SDL_SCANCODE_RIGHT])
    game->try_right = 1;
  if(keystate[SDL_SCANCODE_LEFT])
    game->try_left = 1;
  if(keystate[SDL_SCANCODE_UP])
    game->try_jump = 1;

  if(event.type == SDL_QUIT)
    game->quit = 1;
}

void update_game(struct game_state *game){
  check_collision(game);
  pickup_item(game, game->check_pickup_x, game->check_pickup_y);
  verify_input(game);
  move_dave(game);
  scroll_screen(game);
  apply_gravity(game);
  clear_input(game);
}

void render(struct game_state *game, SDL_Renderer *renderer, struct game_assets *assets){
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);

  draw_world(game, assets, renderer);
  draw_dave(game, assets, renderer);

  SDL_RenderPresent(renderer);
}

void check_collision(struct game_state *game){
  game->collision_point[0] = is_clear(game, game->dave_px+4, game->dave_py-1);
  game->collision_point[1] = is_clear(game, game->dave_px+10, game->dave_py-1);
  game->collision_point[2] = is_clear(game, game->dave_px+11, game->dave_py+4);
  game->collision_point[3] = is_clear(game, game->dave_px+11, game->dave_py+12);
  game->collision_point[4] = is_clear(game, game->dave_px+10, game->dave_py+16);
  game->collision_point[5] = is_clear(game, game->dave_px+4, game->dave_py+16);
  game->collision_point[6] = is_clear(game, game->dave_px+3, game->dave_py+12);
  game->collision_point[7] = is_clear(game, game->dave_px+3, game->dave_py+4);
  game->on_ground = (!game->collision_point[4] && !game->collision_point[5]);
}

void verify_input(struct game_state *game){
  if(game->try_right && game->collision_point[2] && game->collision_point[3]) 
    game->dave_right = 1;
  if(game->try_left && game->collision_point[6] && game->collision_point[7]) 
    game->dave_left = 1;
  if(game->try_jump && game->on_ground && !game->dave_jump && game->collision_point[0] && game->collision_point[1]) 
    game->dave_jump = 1;
}

void move_dave(struct game_state *game) {
  if(game->dave_right){
    game->dave_px += 2 * game->time_scale;
    game->dave_right = 0;
  }
  if(game->dave_left){
    game->dave_px -= 2 * game->time_scale;
    game->dave_left = 0;
  }
  if(game->dave_jump){
    if(!game->jump_timer)
      game->jump_timer = 20;

    if(game->collision_point[0] && game->collision_point[1]){
      if (game->jump_timer > 5) 
        game->dave_py -= 2 * game->time_scale;
      else 
        game->dave_py -= 1 * game->time_scale;
      
    }
    else 
      game->jump_timer = 0;

    if(!game->jump_timer)
      game->dave_jump = 0;
  }
}

void apply_gravity(struct game_state *game){
  if(!game->dave_jump && !game->on_ground){
    if(is_clear(game, game->dave_px+4, game->dave_py+17))
      game->dave_py += 2 * game->time_scale;
    else {
      uint8_t not_align;
      not_align = game->dave_py % TILE_SIZE;
      if(not_align){
        game->dave_py = not_align < 8? 
          game->dave_py - not_align :
          game->dave_py + TILE_SIZE - not_align;
      }
    }
  }
}

void clear_input(struct game_state *game){
  game->try_jump=0;
  game->try_left=0;
  game->try_right=0;
}

void scroll_screen(struct game_state *game){
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

void pickup_item(struct game_state *game, uint8_t grid_x, uint8_t grid_y) {
  uint8_t type;
  if(!grid_x || !grid_y)
    return;
  
  type = game->level[game->current_level].tiles[grid_y*100+grid_x];

  switch(type) {
    // add score and special item cases here later
    default: break;
  }
  
  game->level[game->current_level].tiles[grid_y*100+grid_x] = 0;

  game->check_pickup_x = 0;
  game->check_pickup_y = 0;
}

void draw_world(struct game_state *game, struct game_assets *assets, SDL_Renderer *renderer){
  uint8_t i,j;
  SDL_Rect dest;
  uint8_t tile_index;

  for (j = 0; j < 10; j++)
  {
    dest.y = j*TILE_SIZE;
    dest.w = TILE_SIZE;
    dest.h = TILE_SIZE;
    for (i = 0; i < 20; i++)
    {
      dest.x = i * TILE_SIZE;
      tile_index = game->level[game->current_level].tiles[j*100+game->view_x+i];
      SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
    }

  }
}

void draw_dave(struct game_state *game, struct game_assets *assets, SDL_Renderer *renderer){
  SDL_Rect dest;
  dest.x = game->dave_px;
  dest.y = game->dave_py;
  dest.w = 20;
  dest.h = 16;

  SDL_RenderCopy(renderer, assets->graphics_tiles[56], NULL, &dest);
}

uint8_t is_clear(struct game_state *game, uint16_t px, uint16_t py){
  uint8_t grid_x;
  uint8_t grid_y;
  uint8_t type;

  grid_x = px / TILE_SIZE;
  grid_y = py / TILE_SIZE;
  type = game->level[game->current_level].tiles[grid_y*100+grid_x];

  if(type == 1) {return 0;}
  if(type == 3) {return 0;}
  if(type == 5) {return 0;}
  if(type == 15) {return 0;}
  if(type == 16) {return 0;}
  if(type == 17) {return 0;}
  if(type == 18) {return 0;}
  if(type == 19) {return 0;}
  if(type == 21) {return 0;}
  if(type == 22) {return 0;}
  if(type == 23) {return 0;}
  if(type == 24) {return 0;}
  if(type == 29) {return 0;}
  if(type == 30) {return 0;}

  switch(type) {
    case 10:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    {
      game->check_pickup_x = grid_x;
      game->check_pickup_y = grid_y;
    } 
      break;
    default: 
    break;
  }

  return 1;
}