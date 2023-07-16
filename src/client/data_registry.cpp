/* data cache and management
 */
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

#define INIT_MAPSIZE 8

vector<vector<int>> *map =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
vector<vector<int>> *objects =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));

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
  std::vector<std::vector<int>> get_tiles();
  std::vector<std::vector<int>> get_objects();
  SDL_Texture *background_dark_purple_void_texture;
  SDL_Texture *forest_tile;
  SDL_Texture *ocean_tile;
  SDL_Texture *plains_tile;
  SDL_Texture *base;
  SDL_Texture *object0;
  SDL_Texture *object1;
  TTF_Font *default_font;
  void process_map_update(char *input);
  void process_object_update(char *input);

private:
  bool map_locked = false;
  bool objects_locked = false;
};

Registry::Registry() {
  // do nothing
}

void Registry::init(SDL_Renderer *renderer) {
  background_dark_purple_void_texture = SDL_CreateTextureFromSurface(
      renderer,
      SDL_LoadBMP("./assets/background/background_dark-purple_void.bmp"));
  forest_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/forest_tile.bmp"));
  ocean_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/ocean_tile.bmp"));
  plains_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/plains_tile.bmp"));
  base = SDL_CreateTextureFromSurface(renderer,
                                      SDL_LoadBMP("./assets/objects/base.bmp"));
  object0 = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/test_object0.bmp"));
  object1 = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/test_object1.bmp"));
  default_font = TTF_OpenFont(
      "./assets/fonts/FreeMono.ttf",
      24); // using a smaller font size in order to have a piyelated result
}

void Registry::deinit() {
  SDL_DestroyTexture(background_dark_purple_void_texture);
  SDL_DestroyTexture(forest_tile);
  SDL_DestroyTexture(ocean_tile);
  SDL_DestroyTexture(plains_tile);
  SDL_DestroyTexture(base);
  SDL_DestroyTexture(object0);
  SDL_DestroyTexture(object1);
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

void Registry::process_map_update(char *input) {
  map_locked = true;
  if (strcmp(input, "notFound") == 0 || strcmp(input, "IDnotFound") == 0) {
    printf("ERROR! not a map: %s\n", input);
    return;
  }
  // read input
  vector<string> rows = tokenize(input, ", ; ");
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
  vector<string> rows = tokenize(input, ", ; ");
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
