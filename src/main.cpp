#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <stdio.h>
#include <thread>

const int init_screen_width = 640;
const int init_screen_height = 480;

class Renderer {
public:
  Renderer(SDL_Window *window);
  ~Renderer();
  void stop();
  float fps; // TODO move to private and add get method
  // TODO move to private and add get/set methods
  int fps_delay = 15; // WIP

private:
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

void Renderer::render() {
  // TODO lower VRAM usage
  int rendersize = 128;
  // TODO layer rendering
  SDL_RenderClear(rRenderer);
  // TODO optimize rendering by loading required assets to RAM
  SDL_Surface *image = SDL_LoadBMP("./assets/tiles/sea_tile.bmp");
  SDL_Texture *texture = SDL_CreateTextureFromSurface(rRenderer, image);
  SDL_Rect dstrect = {0, 0, rendersize, rendersize};
  SDL_RenderCopy(rRenderer, texture, NULL, &dstrect);
  SDL_Rect rect;
  rect.x = 50;
  rect.y = 50;
  rect.w = 50;
  rect.h = 50;
  SDL_SetRenderDrawColor(rRenderer, 0, 0, 0, 255);
  if (SDL_RenderDrawRect(rRenderer, &rect) < 0) {
    std::cerr << SDL_GetError();
  }
  SDL_RenderPresent(rRenderer);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(image);
  SDL_Delay(fps_delay);
}

void Renderer::renderJob() {
  SDL_GL_MakeCurrent(rWindow, rContext);
  rRenderer = SDL_CreateRenderer(rWindow, -1, SDL_RENDERER_ACCELERATED);
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
      init_screen_width, init_screen_height, SDL_WINDOW_RESIZABLE);
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
    std::cout << renderer.fps << '\n';
  }
  SDL_Quit();
  return 0;
}
