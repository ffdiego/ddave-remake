#pragma once

#include <SDL.h>

struct dave_level{
  uint8_t path[256];
  uint8_t tiles[1000];
  uint8_t padding[24];
};

struct game_state{
  double_t time_scale;
  
  uint8_t quit;
  uint8_t current_level;
  uint8_t view_x;
  uint8_t view_y;
  int8_t scroll_x;
  uint8_t dave_x;
  uint8_t dave_y;
  uint16_t dave_px;
  uint16_t dave_py;

  uint8_t try_right;
  uint8_t try_left;
  uint8_t try_jump;

  uint8_t dave_right;
  uint8_t dave_left;
  uint8_t dave_jump;
  uint8_t jump_timer;
  uint8_t check_pickup_x;
  uint8_t check_pickup_y;

  uint8_t on_ground;

  uint8_t collision_point[9];


  struct dave_level level[10];
};

struct game_assets {
  SDL_Texture *graphics_tiles[158];
};


void init_game(struct game_state *);
void init_assets(struct game_assets *, SDL_Renderer *);
void check_input(struct game_state *);
void update_game(struct game_state *);
void check_collision(struct game_state *);
void pickup_item(struct game_state *, uint8_t, uint8_t);
void render(struct game_state *, SDL_Renderer *, struct game_assets *);
void verify_input(struct game_state *game);
void scroll_screen(struct game_state *);
void move_dave(struct game_state *);
void apply_gravity(struct game_state *);
void clear_input(struct game_state *);
void draw_world(struct game_state *, struct game_assets *, SDL_Renderer *);
void draw_dave(struct game_state *, struct game_assets *, SDL_Renderer *);

uint8_t is_clear(struct game_state *, uint16_t, uint16_t);