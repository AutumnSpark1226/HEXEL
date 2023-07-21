/* HRE - HEXEL render engine
   responsible for displaying the game
*/
#include "data_registry.cpp"
#include "menu.cpp"
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
  HRE(SDL_Window *new_window);
  ~HRE();
  void stop();
  int get_fps_delay();
  void set_fps_delay(int new_fps_delay);
  vector<int> get_clickable(int screen_x, int screen_y);
  SDL_Window *window; // TODO replace dulicate

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

HRE::HRE(SDL_Window *new_window)
    : rendered_window(new_window), renderer(nullptr), mutex(nullptr),
      cond(nullptr), first_run(true) {
  window = new_window;
  context = SDL_GL_GetCurrentContext();
  SDL_GL_MakeCurrent(new_window, nullptr);
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
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
          SDL_RENDERER_TARGETTEXTURE); // TODO possibility to (de)activate
  // vsync (-> game settings)
  reg->init(renderer);
  SDL_LockMutex(mutex);
  first_run = false;
  SDL_CondSignal(cond);
  SDL_UnlockMutex(mutex);
  while (reg->keep_running) {
    if (renderer != nullptr) {
      switch (reg->ui_mode) {
      case 0:
        // main menu not implemented yet
        reg->ui_mode = 10;
        reg->reset_buttons();
        break;
      case 10:
        render();
      case 11:
        render();
      case 12:
        render();
      case 13:
        render();
      case 14:
        render();
      }
    }
  }
  exit(0);
}

void HRE::render_background() {
  int width;
  int height;
  SDL_GL_GetDrawableSize(window, &width, &height);
  SDL_Rect destination = {0, 0, width, height};
  SDL_RenderCopy(renderer, reg->background_dark_purple_void_texture, NULL,
                 &destination);
}

void HRE::render_tiles_objects() {
  // TODO WIP
  const std::vector<std::vector<int>> map = reg->get_tiles();
  const std::vector<std::vector<int>> objects = reg->get_objects();
  int width;
  int height;
  SDL_GL_GetDrawableSize(window, &width, &height);
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
        SDL_RenderCopy(renderer, reg->fog_of_war_tile, NULL, &destination);
      else if (map.at(y).at(x) == 2)
        SDL_RenderCopy(renderer, reg->forest_tile, NULL, &destination);
      else if (map.at(y).at(x) == 3)
        SDL_RenderCopy(renderer, reg->ocean_tile, NULL, &destination);
      else if (map.at(y).at(x) == 4)
        SDL_RenderCopy(renderer, reg->plains_tile, NULL, &destination);
      destination = {(int)(x * reg->rendersize * 1.01) + x_diff +
                         (reg->rendersize - object_rendersize) / 2 + zero_x,
                     (int)(y * reg->rendersize * 0.765) +
                         (reg->rendersize - object_rendersize) / 2 + zero_y,
                     object_rendersize, object_rendersize};
      if (objects.at(y).at(x) == 1)
        SDL_RenderCopy(renderer, reg->base_blue, NULL, &destination);
      else if (objects.at(y).at(x) == 2)
        SDL_RenderCopy(renderer, reg->base_red, NULL, &destination);
      else if (objects.at(y).at(x) == 3)
        SDL_RenderCopy(renderer, reg->swordsman_blue, NULL, &destination);
      else if (objects.at(y).at(x) == 4)
        SDL_RenderCopy(renderer, reg->swordsman_red, NULL, &destination);
    }
    std::swap(x_diff, xx_diff);
  }
}

void HRE::render_ui() {
  SDL_Rect destination;
  if (reg->ui_mode == 11 || reg->ui_mode == 12 || reg->ui_mode == 13 ||
      reg->ui_mode == 14) {
    int width;
    int height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    destination = {0, 0, width, height};
    SDL_RenderCopy(renderer, reg->background_fog_texture, NULL, &destination);
  }
  vector<vector<int>> buttons = reg->get_buttons();
  vector<string> button_texts = reg->get_button_texts();
  // render buttons
  for (size_t i = 0; i < buttons.size(); i++) {
    destination = {buttons[i][0], buttons[i][1], buttons[i][2], buttons[i][3]};
    SDL_RenderCopy(renderer, reg->button, NULL, &destination);
    SDL_Color color = {255, 255, 255};
    SDL_Surface *text =
        TTF_RenderText_Solid(reg->default_font, button_texts[i].data(), color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text);
    destination = {buttons[i][0] + 20, buttons[i][1] + 10, buttons[i][2] - 40,
                   buttons[i][3] - 25};
    SDL_RenderCopy(renderer, text_texture, NULL, &destination);
  }
}

vector<int> HRE::get_clickable(int screen_x, int screen_y) {
  /*
  {type, vector_x, vector_y}
  type:
    0 -> not found
    1 -> button (priority)
    2 -> object
  */
  vector<vector<int>> buttons = reg->get_buttons();
  vector<string> button_texts = reg->get_button_texts();
  // render buttons
  for (int i = 0; i < (int)buttons.size(); i++) {
    if ((screen_x - buttons[i][0] > 0 &&
         screen_x - buttons[i][0] < buttons[i][2]) &&
        (screen_y - buttons[i][1] > 0 &&
         screen_y - buttons[i][1] < buttons[i][3])) {
      return {1, i};
    }
  }
  if (reg->ui_mode == 10) {
    const std::vector<std::vector<int>> objects = reg->get_objects();
    int width;
    int height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    //           ( middle )
    int zero_x = width / 2 - objects.at(0).size() * 0.25 * reg->rendersize +
                 reg->camera_position_x; // TODO fix
    int zero_y = height / 2 - objects.size() * 0.25 * reg->rendersize +
                 reg->camera_position_y; // TODO fix
    int object_rendersize = reg->rendersize * 0.6;
    int x_diff = 0;
    int xx_diff = reg->rendersize * 1.01 / 2;
    for (int y = 0; y < (int)objects.size(); y++) {
      for (int x = 0; x < (int)objects.at(y).size(); x++) {
        int object_pos_x = (int)(x * reg->rendersize * 1.01) + x_diff +
                           (reg->rendersize - object_rendersize) / 2 + zero_x;
        int object_pos_y = (int)(y * reg->rendersize * 0.765) +
                           (reg->rendersize - object_rendersize) / 2 + zero_y;
        if ((screen_x - object_pos_x > 0 &&
             screen_x - object_pos_x < object_rendersize) &&
            (screen_y - object_pos_y > 0 &&
             screen_y - object_pos_y < object_rendersize)) {
          return {2, x, y};
        }
      }
      swap(x_diff, xx_diff);
    }
  }
  return {0};
}
