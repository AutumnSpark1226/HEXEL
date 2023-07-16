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
  int fps_delay = 10;
  void renderJob();
  void render();
  SDL_GLContext context;
  SDL_Window *rendered_window;
  SDL_Renderer *renderer;
  std::thread *renderer_thread;
  SDL_mutex *mutex;
  SDL_cond *cond;
  bool first_run;
  void render_background();
  void render_tiles_objects();
  void render_ui();
};

HRE::HRE() {
  // do nothing
}

HRE::HRE(SDL_Window *window)
    : rendered_window(window), renderer(nullptr), mutex(nullptr), cond(nullptr),
      first_run(true) {
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

void HRE::render() {
  // TODO lower VRAM usage
  SDL_RenderClear(renderer);
  render_background();
  render_tiles_objects();
  render_ui();
  SDL_RenderPresent(renderer);
  SDL_Delay(fps_delay);
}

void HRE::renderJob() {
  SDL_GL_MakeCurrent(rendered_window, context);
  renderer = SDL_CreateRenderer(
      rendered_window, -1,
      SDL_RENDERER_ACCELERATED |       /*SDL_RENDERER_PRESENTVSYNC |*/
          SDL_RENDERER_TARGETTEXTURE); // TODO possibility to (de)activate
  // vsync (-> game settings)
  reg->init(renderer);
  SDL_LockMutex(mutex);
  first_run = false;
  SDL_CondSignal(cond);
  SDL_UnlockMutex(mutex);
  while (reg->keep_running) {
    if (renderer != nullptr) {
      render();
    }
  }
  exit(0);
}

void HRE::render_background() {
  SDL_DisplayMode display_mode;
  SDL_GetCurrentDisplayMode(0, &display_mode);
  int width = display_mode.w;
  int height = display_mode.h;
  SDL_Rect destination = {0, 0, width, height};
  SDL_RenderCopy(renderer, reg->background_dark_purple_void_texture, NULL,
                 &destination);
}

void HRE::render_tiles_objects() {
  // TODO WIP
  const std::vector<std::vector<int>> map = reg->get_tiles();
  const std::vector<std::vector<int>> objects = reg->get_objects();
  SDL_DisplayMode display_mode;
  SDL_GetCurrentDisplayMode(0, &display_mode);
  int width = display_mode.w;
  int height = display_mode.h;
  //           ( middle )
  int zero_x = width / 2 - map.at(0).size() * 0.25 * reg->rendersize +
               reg->camera_position_x; // TODO fix
  int zero_y = height / 2 - map.size() * 0.25 * reg->rendersize +
               reg->camera_position_y; // TODO fix
  int object_rendersize = reg->rendersize * 0.6;
  int x_diff = 0;
  int xx_diff = reg->rendersize * 1.01 / 2;
  for (int y = 0; y < (int)map.size(); y++) {
    for (int x = 0; x < (int)map.at(y).size(); x++) {
      SDL_Rect destination = {(int)(x * reg->rendersize * 1.01) + x_diff +
                                  zero_x,
                              (int)(y * reg->rendersize * 0.765) + zero_y,
                              reg->rendersize, reg->rendersize};
      if (map.at(y).at(x) == 1)
        SDL_RenderCopy(renderer, reg->forest_tile, NULL, &destination);
      else if (map.at(y).at(x) == 2)
        SDL_RenderCopy(renderer, reg->ocean_tile, NULL, &destination);
      else if (map.at(y).at(x) == 3)
        SDL_RenderCopy(renderer, reg->plains_tile, NULL, &destination);
      destination = {(int)(x * reg->rendersize * 1.01) + x_diff +
                         (reg->rendersize - object_rendersize) / 2 + zero_x,
                     (int)(y * reg->rendersize * 0.765) +
                         (reg->rendersize - object_rendersize) / 2 + zero_y,
                     object_rendersize, object_rendersize};
      if (objects.at(y).at(x) == 1)
        SDL_RenderCopy(renderer, reg->base, NULL, &destination);
      else if (objects.at(y).at(x) == 2)
        SDL_RenderCopy(renderer, reg->object0, NULL, &destination);
      else if (objects.at(y).at(x) == 3)
        SDL_RenderCopy(renderer, reg->object1, NULL, &destination);
    }
    std::swap(x_diff, xx_diff);
  }
}

void HRE::render_ui() {
  // TODO still WIP
  SDL_Rect destination = {100, 100, 512, 128};
  SDL_Color color = {255, 255, 255};
  SDL_Surface *text;
  text = TTF_RenderText_Solid(reg->default_font, "HEXEL alpha preview", color);
  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text);
  SDL_RenderCopy(renderer, text_texture, NULL, &destination);
  SDL_RenderCopy(renderer, text_texture, NULL, &destination);
}
