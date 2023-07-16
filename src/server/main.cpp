#include "communication.cpp"
#include <SDL2/SDL.h>

Communication com;

int main(int argc, char **argv) {
  reg->init(25, 25);
  com = Communication(43233);
  while (true) {
    com.accept_client();
    com.check_clients();
    SDL_Delay(100);
  }
}
