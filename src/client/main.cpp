#include "HRE.cpp"
#include "communication.cpp"
#include <SDL2/SDL.h>

#define INIT_SCREEN_WIDTH 1920
#define INIT_SCREEN_HEIGTH 1080

class Networking {
public:
  Networking();
  Networking(char *host, int port);
  void start();
  void stop();

private:
  Communication com;
  void network_job();
  std::thread *network_thread;
  bool *nKeepRunning = (bool *)true;
};

Networking networking;
HRE hre;

Networking::Networking() {
  // do nothing
}

Networking::Networking(char *host, int port) {
  com = Communication(host, port);
}

void Networking::network_job() {
  while (nKeepRunning) {
    com.send_text((char *)"ping");
    char *in = com.receive_text();
    std::cout << in << '\n';
    SDL_Delay(2000);
  }
}

void Networking::start() {
  network_thread = new std::thread(&Networking::network_job, this);
}

void Networking::stop() {
  nKeepRunning = (bool *)false;
  network_thread->join();
}

void process_input() {
  SDL_Event event;
  const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
  while (reg->keep_running) {
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      } else if (event.type == SDL_KEYDOWN) {
        if (keyboard_state[SDL_SCANCODE_W] && !(keyboard_state[SDL_SCANCODE_S]))
          reg->camera_position_y += 6;
        if (keyboard_state[SDL_SCANCODE_A] && !(keyboard_state[SDL_SCANCODE_D]))
          reg->camera_position_x += 6;
        if (keyboard_state[SDL_SCANCODE_S] && !(keyboard_state[SDL_SCANCODE_W]))
          reg->camera_position_y -= 6;
        if (keyboard_state[SDL_SCANCODE_D] && !(keyboard_state[SDL_SCANCODE_A]))
          reg->camera_position_x -= 6;
      } else if (event.type == SDL_MOUSEWHEEL) {
        if (event.wheel.y > 0) // scroll up
          reg->rendersize += 4;
        else if (event.wheel.y < 0) // scroll down
          reg->rendersize -= 4;
      }
    }
  }
}

void ui_start() {
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window *window = SDL_CreateWindow(
      "HEXEL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGTH,
      SDL_WINDOW_MAXIMIZED | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  hre = HRE(window);
  process_input();
  hre.stop();
  networking.stop();
  SDL_DestroyWindow(window);
  window = nullptr;
  TTF_Quit();
  SDL_Quit();
}

int main(int argc, char **argv) {
  networking = Networking((char *)"127.0.0.1", 43233);
  networking.start();
  ui_start();
}