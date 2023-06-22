#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <stdio.h>
#include <thread>

const int init_screen_width = 1920;
const int init_screen_height = 1080;

class Renderer {
public:
  Renderer(SDL_Window *window);
  ~Renderer();
  void stop();
  float get_fps();
  void set_fps_delay(int new_fps_delay);
  int get_fps_delay();

private:
  float fps;
  int fps_delay = 10;

  void renderJob();
  void render();
  SDL_GLContext rContext;
  SDL_Window *rWindow;
  SDL_Renderer *rRenderer;
  std::thread *rThread;
  SDL_mutex *rLock;
  SDL_cond *rCond;
  bool rFirstRun, rKeepRunning;
};

Renderer::Renderer(SDL_Window *window)
    : rWindow(window), rRenderer(nullptr), rLock(nullptr), rCond(nullptr),
      rFirstRun(true), rKeepRunning(true) {
  rContext = SDL_GL_GetCurrentContext();
  SDL_GL_MakeCurrent(window, nullptr);
  rLock = SDL_CreateMutex();
  rCond = SDL_CreateCond();
  rThread = new std::thread(&Renderer::renderJob, this);
  SDL_LockMutex(rLock);
  while (rFirstRun)
    SDL_CondWait(rCond, rLock);
  SDL_UnlockMutex(rLock);
}

Renderer::~Renderer() {
  SDL_DestroyCond(rCond);
  SDL_DestroyMutex(rLock);
  delete rThread;
}

void Renderer::stop() {
  rKeepRunning = false;
  SDL_DestroyRenderer(rRenderer);
  rThread->join();
}

float Renderer::get_fps() { return fps; }

void Renderer::set_fps_delay(int new_fps_delay) { fps_delay = new_fps_delay; }

int Renderer::get_fps_delay() { return fps_delay; }

void Renderer::render() {
  // TODO lower VRAM usage
  int rendersize = 128;
  // TODO layer rendering
  SDL_RenderClear(rRenderer);
  // best way to store data for prototyping
  int map[5][10] = {{2, 1, 2, 1, 2, 1, 1, 1, 1, 1},
                    {2, 1, 2, 1, 2, 1, 2, 1, 1, 2},
                    {2, 2, 2, 1, 2, 1, 1, 1, 1, 1},
                    {2, 1, 2, 1, 2, 1, 2, 1, 1, 2},
                    {2, 1, 2, 1, 2, 1, 1, 2, 2, 1}};
  int object_rendersize = rendersize * 0.6;
  int objects[5][10] = {{0, 0, 0, 1, 2, 0, 1, 0, 2, 2},
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
                        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
                        {0, 1, 0, 0, 2, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  // TODO optimize rendering by loading required assets to RAM
  SDL_Surface *p_image = SDL_LoadBMP("./assets/tiles/forest_tile.bmp");
  SDL_Texture *p_texture = SDL_CreateTextureFromSurface(rRenderer, p_image);
  SDL_Surface *s_image = SDL_LoadBMP("./assets/tiles/ocean_tile.bmp");
  SDL_Texture *s_texture = SDL_CreateTextureFromSurface(rRenderer, s_image);
  SDL_Surface *o0_image = SDL_LoadBMP("./assets/objects/test_object0.bmp");
  SDL_Texture *o0_texture = SDL_CreateTextureFromSurface(rRenderer, o0_image);
  SDL_Surface *o1_image = SDL_LoadBMP("./assets/objects/test_object1.bmp");
  SDL_Texture *o1_texture = SDL_CreateTextureFromSurface(rRenderer, o1_image);
  int x_diff = 0;
  int xx_diff = rendersize / 2;
  for (int y = 0; y < (int)(sizeof(map) / sizeof(map[0])); y++) {
    for (int x = 0; x < (int)(sizeof(map[0]) / sizeof(int)); x++) {
      SDL_Rect dstrect = {x * rendersize + x_diff, y * (int)(rendersize * 0.77),
                          rendersize, rendersize};
      if (map[y][x] == 1)
        SDL_RenderCopy(rRenderer, p_texture, NULL, &dstrect);
      else if (map[y][x] == 2)
        SDL_RenderCopy(rRenderer, s_texture, NULL, &dstrect);
      SDL_Rect dstrecto = {
          x * rendersize + x_diff + (rendersize - object_rendersize) / 2,
          y * (int)(rendersize * 0.77) + (rendersize - object_rendersize) / 2,
          object_rendersize, object_rendersize};
      if (objects[y][x] == 1)
        SDL_RenderCopy(rRenderer, o0_texture, NULL, &dstrecto);
      else if (objects[y][x] == 2)
        SDL_RenderCopy(rRenderer, o1_texture, NULL, &dstrecto);
    }
    std::swap(x_diff, xx_diff);
  }
  SDL_RenderPresent(rRenderer);
  SDL_DestroyTexture(s_texture);
  SDL_FreeSurface(s_image);
  SDL_DestroyTexture(p_texture);
  SDL_FreeSurface(p_image);
  SDL_DestroyTexture(o0_texture);
  SDL_FreeSurface(o0_image);
  SDL_DestroyTexture(o1_texture);
  SDL_FreeSurface(o1_image);
  SDL_Delay(fps_delay);
}

void Renderer::renderJob() {
  SDL_GL_MakeCurrent(rWindow, rContext);
  rRenderer = SDL_CreateRenderer(
      rWindow, -1,
      SDL_RENDERER_ACCELERATED |       /*SDL_RENDERER_PRESENTVSYNC |*/
          SDL_RENDERER_TARGETTEXTURE); // TODO possibility to (de)activate vsync
                                       // (-> game settings)
  SDL_LockMutex(rLock);
  rFirstRun = false;
  SDL_CondSignal(rCond);
  SDL_UnlockMutex(rLock);
  while (rKeepRunning) {
    if (rRenderer != nullptr) {
      Uint32 start_time, frame_time;
      start_time = SDL_GetTicks();
      render();
      frame_time = SDL_GetTicks() - start_time;
      fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
    }
  }
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *window = SDL_CreateWindow(
      "HEXEL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      init_screen_width, init_screen_height,
      SDL_WINDOW_MAXIMIZED | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  Renderer renderer(window);
  while (window) {
    SDL_Event event;
    if (SDL_WaitEvent(&event)) {
      if (event.type == SDL_QUIT) {
        renderer.stop();
        SDL_DestroyWindow(window);
        window = nullptr;
      }
    }
  }
  SDL_Quit();
  return 0;
}
