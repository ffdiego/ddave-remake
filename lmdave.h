#pragma once

#include <SDL.h>

struct dave_level{
  uint8_t path[256];
  uint8_t tiles[1000];
  uint8_t padding[24];
};

struct game_state{
  uint8_t quit;
  uint8_t current_level;
  uint8_t view_x;
  uint8_t view_y;
  int8_t scroll_x;

  struct dave_level level[10];
};

struct game_assets {
  SDL_Texture *graphics_tiles[158];
};


void init_game(struct game_state *);
void init_assets(struct game_assets *, SDL_Renderer *);
void check_input(struct game_state *);
void update_game(struct game_state *);
void render(struct game_state *, SDL_Renderer *, struct game_assets *);