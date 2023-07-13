/* data cache and management
 */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>

class Registry {
public:
  Registry();
  void init(SDL_Renderer *renderer);
  void deinit();
  bool keep_running = true;
  int rendersize = 128;
  int camera_position_x = 50;
  int camera_position_y = 50;
  std::vector<std::vector<int>> get_tiles();
  std::vector<std::vector<int>> get_objects();
  SDL_Texture *forest_tile;
  SDL_Texture *ocean_tile;
  SDL_Texture *object0;
  SDL_Texture *object1;
  SDL_Texture *background_dark_purple_void_texture;

private:
  std::vector<std::vector<int>> map = {{2, 1, 2, 1, 2, 1, 1, 1, 1, 1},
                                       {2, 1, 2, 1, 2, 1, 2, 1, 1, 2},
                                       {2, 2, 2, 1, 2, 1, 1, 1, 1, 1},
                                       {2, 1, 2, 1, 2, 1, 2, 1, 1, 2},
                                       {2, 1, 2, 1, 2, 1, 1, 2, 2, 1}};
  std::vector<std::vector<int>> objects = {{0, 0, 0, 0, 2, 0, 0, 0, 2, 1},
                                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
                                           {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
                                           {0, 1, 0, 0, 2, 0, 0, 0, 0, 0},
                                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
};

Registry::Registry() {
  // do something
}

void Registry::init(SDL_Renderer *renderer) {
  forest_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/forest_tile.bmp"));
  ocean_tile = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/tiles/ocean_tile.bmp"));
  object0 = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/test_object0.bmp"));
  object1 = SDL_CreateTextureFromSurface(
      renderer, SDL_LoadBMP("./assets/objects/test_object1.bmp"));
  background_dark_purple_void_texture = SDL_CreateTextureFromSurface(
      renderer,
      SDL_LoadBMP("./assets/background/background_dark-purple_void.bmp"));
}

void Registry::deinit() {
  SDL_DestroyTexture(forest_tile);
  SDL_DestroyTexture(ocean_tile);
  SDL_DestroyTexture(object0);
  SDL_DestroyTexture(object1);
  SDL_DestroyTexture(background_dark_purple_void_texture);
}

std::vector<std::vector<int>> Registry::get_tiles() { return map; }

std::vector<std::vector<int>> Registry::get_objects() { return objects; }
