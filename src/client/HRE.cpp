/* HRE - HEXEL render engine
   responsible for displaying the game
*/
#include "data_registry.cpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <vector>

Registry *reg = new Registry();

class HRE {
public:
  HRE();
  HRE(SDL_Window *window);
  ~HRE();
  void stop();
  int get_fps_delay();
  void set_fps_delay(int new_fps_delay);

private:
  int fps_delay = 20;
  void renderJob();
  void render(Registry *reg);
  SDL_GLContext context;
  SDL_Window *rendered_window;
  SDL_Renderer *renderer;
  std::thread *renderer_thread;
  SDL_mutex *mutex;
  SDL_cond *cond;
  bool first_run;
  void render_background(Registry *reg);
  void render_tiles_objects(Registry *reg);
  void render_text(Registry *reg);
};

HRE::HRE() {
  // do nothing
}

HRE::HRE(SDL_Window *window)
    : rendered_window(window), renderer(nullptr), mutex(nullptr), cond(nullptr),
      first_run(true) {
  TTF_Init();
  context = SDL_GL_GetCurrentContext();
  SDL_GL_MakeCurrent(window, nullptr);
  mutex = SDL_CreateMutex();
  cond = SDL_CreateCond();
  renderer_thread = new std::thread(&HRE::renderJob, this);
  SDL_LockMutex(mutex);
  while (first_run)
    SDL_CondWait(cond, mutex);
  SDL_UnlockMutex(mutex);
  renderer_thread->detach();
}

HRE::~HRE() {
  SDL_DestroyCond(cond);
  SDL_DestroyMutex(mutex);
  delete renderer_thread;
}

void HRE::stop() {
  reg->keep_running = false;
  SDL_DestroyRenderer(renderer);
}

void HRE::set_fps_delay(int new_fps_delay) { fps_delay = new_fps_delay; }

int HRE::get_fps_delay() { return fps_delay; }

void HRE::render(Registry *reg) {
  // TODO lower VRAM usage
  SDL_RenderClear(renderer);
  render_background(reg);
  render_tiles_objects(reg);
  render_text(reg);
  SDL_RenderPresent(renderer);
  SDL_Delay(fps_delay);
}

void HRE::renderJob() {
  SDL_GL_MakeCurrent(rendered_window, context);
  renderer = SDL_CreateRenderer(
      rendered_window, -1,
      SDL_RENDERER_ACCELERATED |       /*SDL_RENDERER_PRESENTVSYNC |*/
          SDL_RENDERER_TARGETTEXTURE); // TODO possibility to (de)activate
  // vsync
  // (-> game settings)
  reg->init(renderer);
  SDL_LockMutex(mutex);
  first_run = false;
  SDL_CondSignal(cond);
  SDL_UnlockMutex(mutex);
  while (reg->keep_running) {
    if (renderer != nullptr) {
      render(reg);
    }
  }
  exit(0);
}

void HRE::render_background(Registry *reg) {
  SDL_DisplayMode display_mode;
  SDL_GetCurrentDisplayMode(0, &display_mode);
  int width = display_mode.w;
  int height = display_mode.h;
  SDL_Rect destination = {0, 0, width, height};
  SDL_RenderCopy(renderer, reg->background_dark_purple_void_texture, NULL,
                 &destination);
}

void HRE::render_tiles_objects(Registry *reg) {
  SDL_DisplayMode display_mode;
  SDL_GetCurrentDisplayMode(0, &display_mode);
  int display_width = display_mode.w;
  int display_height = display_mode.h;
  int object_rendersize = reg->rendersize * 0.6;
  std::vector<std::vector<int>> map = reg->get_tiles();
  std::vector<std::vector<int>> objects = reg->get_objects();
  // TODO optimize rendering by loading required assets to RAM
  int x_diff = 0;
  int xx_diff = reg->rendersize / 2;
  for (int y = 0; y < (int)map.size(); y++) {
    for (int x = 0; x < (int)map[y].size(); x++) {
      SDL_Rect destination = {
          (int)(x * reg->rendersize * 1.01) + x_diff + reg->camera_position_x,
          y * (int)(reg->rendersize * 0.77) + reg->camera_position_y,
          reg->rendersize, reg->rendersize};
      if (map[y][x] == 1)
        SDL_RenderCopy(renderer, reg->forest_tile, NULL, &destination);
      else if (map[y][x] == 2)
        SDL_RenderCopy(renderer, reg->ocean_tile, NULL, &destination);
      SDL_Rect destinationo = {(int)(x * reg->rendersize * 1.01) + x_diff +
                                   (reg->rendersize - object_rendersize) / 2 +
                                   reg->camera_position_x,
                               y * (int)(reg->rendersize * 0.77) +
                                   (reg->rendersize - object_rendersize) / 2 +
                                   reg->camera_position_y,
                               object_rendersize, object_rendersize};
      if (objects[y][x] == 1)
        SDL_RenderCopy(renderer, reg->object0, NULL, &destinationo);
      else if (objects[y][x] == 2)
        SDL_RenderCopy(renderer, reg->object1, NULL, &destinationo);
    }
    std::swap(x_diff, xx_diff);
  }
}

void HRE::render_text(
    Registry *reg) { // TODO rendering text somewhere on the map
  TTF_Font *default_font = TTF_OpenFont("./assets/fonts/FreeMono.ttf",
                                        24); // using a smaller font size in
  // order to have a pixelated result
  SDL_Rect destination = {100, 100, 128, 128};
  SDL_Color color = {255, 255, 255};
  SDL_Surface *text;
  text = TTF_RenderText_Solid(default_font, "HEXEL", color);
  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text);
  SDL_RenderCopy(renderer, text_texture, NULL, &destination);
  SDL_RenderCopy(renderer, text_texture, NULL, &destination);
  TTF_CloseFont(default_font);
}
