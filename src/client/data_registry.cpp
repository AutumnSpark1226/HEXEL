/*
    HEXEL - a cool strategy game
    Copyright (C) 2023  AutumnSpark1226

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

// data caching and management

#define INIT_MAPSIZE 8

vector<vector<int>> *map =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
/*
0 -> nothing
1 -> fog of war
2 -> forest
3 -> ocean
4 -> plains
*/
vector<vector<int>> *objects =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
/*
0 -> nothing
1 -> base
2 -> enemy base
3 -> unit
4 -> enemy (unit)
*/

vector<string> tokenize(string input, string delimiter) {
  vector<string> output;
  int start, end = -1 * delimiter.size();
  while (end != -1) {
    start = end + delimiter.size();
    end = input.find(delimiter, start);
    output.push_back(input.substr(start, end - start));
  }
  return output;
}

class Registry {
public:
  Registry();
  void init(SDL_Renderer *renderer);
  void deinit();
  bool keep_running = true;
  int rendersize = 128;
  int camera_position_x = 0;
  int camera_position_y = 0;
  /*
  0 -> main menu
  1 -> join game
  2 -> settings
  9 -> credits
  10 -> normal game
  11 -> ingame menu
  12 -> status view
  13 -> waiting
  14 -> game over
  */
  int ui_mode = 0;
  SDL_Window *window;
  vector<vector<int>> get_tiles();
  vector<vector<int>> get_objects();
  vector<vector<int>> get_buttons();
  vector<string> get_button_texts();
  void add_button(int screen_x, int screen_y, string text);
  void reset_buttons();
  SDL_Texture *background_dark_purple_void_texture;
  SDL_Texture *background_fog_texture;
  SDL_Texture *fog_of_war_tile;
  SDL_Texture *forest_tile;
  SDL_Texture *ocean_tile;
  SDL_Texture *plains_tile;
  SDL_Texture *base_blue;
  SDL_Texture *base_red;
  SDL_Texture *swordsman_blue;
  SDL_Texture *swordsman_red;
  SDL_Texture *button;
  TTF_Font *default_font;
  void process_map_update(char *input);
  void process_object_update(char *input);
  std::vector<int> last_clicked_object = {-1, -1};
  string get_object_name(int x, int y);

private:
  bool map_locked = false;
  bool objects_locked = false;
  vector<vector<int>> *buttons = new vector<vector<int>>(1, vector<int>(4));
  vector<string> *button_texts = new vector<string>(1);
};

Registry::Registry() {
  // do nothing
}

void Registry::init(SDL_Renderer *renderer) {
  background_dark_purple_void_texture = SDL_CreateTextureFromSurface(
      renderer,
      SDL_LoadBMP("./assets/background/background_dark-purple_void.bmp"));
  background_fog_texture = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/background/fog.bmp"));
  fog_of_war_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/fog_of_war_tile.bmp"));
  forest_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/forest_tile.bmp"));
  ocean_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/ocean_tile.bmp"));
  plains_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/plains_tile.bmp"));
  base_blue = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/base_blue.bmp"));
  base_red = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/base_red.bmp"));
  swordsman_blue = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/swordsman_blue.bmp"));
  swordsman_red = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/swordsman_red.bmp"));
  button = SDL_CreateTextureFromSurface(renderer,
                                        SDL_LoadBMP("./assets/ui/button.bmp"));
  default_font = TTF_OpenFont(
      "./assets/fonts/FreeMono.ttf",
      24); // using a smaller font size in order to have a pixelated result
  reset_buttons();
}

void Registry::deinit() {
  SDL_DestroyTexture(background_dark_purple_void_texture);
  SDL_DestroyTexture(background_fog_texture);
  SDL_DestroyTexture(forest_tile);
  SDL_DestroyTexture(fog_of_war_tile);
  SDL_DestroyTexture(ocean_tile);
  SDL_DestroyTexture(plains_tile);
  SDL_DestroyTexture(base_blue);
  SDL_DestroyTexture(base_red);
  SDL_DestroyTexture(swordsman_blue);
  SDL_DestroyTexture(swordsman_red);
  SDL_DestroyTexture(button);
  TTF_CloseFont(default_font);
}

vector<vector<int>> Registry::get_tiles() {
  while (map_locked)
    SDL_Delay(1);
  return (*map);
}

vector<vector<int>> Registry::get_objects() {
  while (objects_locked)
    SDL_Delay(1);
  return (*objects);
}

vector<vector<int>> Registry::get_buttons() { return (*buttons); }

vector<string> Registry::get_button_texts() { return (*button_texts); }

void Registry::add_button(int screen_x, int screen_y, string text) {
  buttons->push_back({screen_x, screen_y, 200, 75});
  button_texts->push_back(text);
}

void Registry::reset_buttons() {
  buttons->clear();
  button_texts->clear();
  if (ui_mode == 10) {
    int width;
    int height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    add_button(width - 200, 0, "End turn");
  }
}

void Registry::process_map_update(char *input) {
  map_locked = true;
  if (strcmp(input, "notFound") == 0 || strcmp(input, "IDnotFound") == 0) {
    printf("ERROR! not a map: %s\n", input);
    return;
  }
  // read input
  vector<string> rows = tokenize(input, ", ;");
  map->clear();
  for (int y = 0; y < (int)rows.size() - 1; y++) {
    vector<string> columns = tokenize(rows.at(y), ", ");
    vector<int> inner_vector(columns.size());
    for (int x = 0; x < (int)columns.size(); x++) {
      inner_vector[x] = stoi(columns.at(x));
    }
    map->push_back(inner_vector);
  }
  map_locked = false;
}

void Registry::process_object_update(char *input) {
  objects_locked = true;
  if (strcmp(input, "notFound") == 0 || strcmp(input, "IDnotFound") == 0) {
    printf("ERROR! not an object: %s\n", input);
    return;
  }
  // read input
  vector<string> rows = tokenize(input, ", ;");
  objects->clear();
  for (int y = 0; y < (int)rows.size() - 1; y++) {
    vector<string> columns = tokenize(rows.at(y), ", ");
    vector<int> inner_vector(columns.size());
    for (int x = 0; x < (int)columns.size(); x++) {
      inner_vector[x] = stoi(columns.at(x));
    }
    objects->push_back(inner_vector);
  }
  objects_locked = false;
}

string Registry::get_object_name(int x, int y) {
  switch (objects->at(y).at(x)) {
  case 1:
    return "base";
  case 2:
    return "enemy base";
  case 3:
    return "unit";
  case 4:
    return "enemy";
  default:
    return "notFound";
  }
}
