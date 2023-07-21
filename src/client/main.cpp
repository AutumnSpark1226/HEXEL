#include "HRE.cpp"
#include "communication.cpp"
#include <SDL2/SDL.h>

#define INIT_SCREEN_WIDTH 1920
#define INIT_SCREEN_HEIGHT 1080
// in ms
#define DATA_REFRESH_INTERVAL 200

class Networking {
public:
  Networking();
  Networking(char *host, int port);
  void start();
  void stop();
  void add_task(char *type);
  void add_task(char *type, char *options);

private:
  void network_job();
  std::thread *network_thread;
  bool *nKeepRunning = (bool *)true;
  vector<vector<char *>> *task_queue =
      new vector<vector<char *>>(1, vector<char *>(2));
};

Networking networking;
Communication com;
HRE hre;
SDL_Window *window;

Networking::Networking() {
  // do nothing
}

Networking::Networking(char *host, int port) {
  com = Communication(host, port);
  int map_size_x = std::stoi(com.receive_text());
  int map_size_y = std::stoi(com.receive_text());
  com.send_text((char *)"ok");
  map = new vector<vector<int>>(map_size_y, vector<int>(map_size_x));
  objects = new vector<vector<int>>(map_size_y, vector<int>(map_size_x));
  task_queue->clear();
}

void Networking::network_job() {
  while (nKeepRunning) {
    task_queue->push_back({(char *)"request_data", (char *)"all"});
    while (task_queue->size() > 0) {
      if (strcmp(task_queue->at(0).at(0), "request_data") == 0) {
        if (strcmp(task_queue->at(0).at(1), "all") == 0 ||
            strcmp(task_queue->at(0).at(1), "map") == 0) {
          char *server_data = com.request_data((char *)"map");
          reg->process_map_update(server_data);
        }
        if (strcmp(task_queue->at(0).at(1), "all") == 0 ||
            strcmp(task_queue->at(0).at(1), "objects") == 0) {
          char *server_data = com.request_data((char *)"objects");
          reg->process_object_update(server_data);
        }
      } else if (strcmp(task_queue->at(0).at(0), "end_turn") == 0) {
        com.send_text((char *)"end_turn");
        reg->ui_mode = 13;
        reg->reset_buttons();
        int width;
        int height;
        SDL_GL_GetDrawableSize(window, &width, &height);
        reg->add_button(width / 2 - 100, height / 2 - 37, (char *)"Waiting...");
        char *response = com.receive_text();
        if (strcmp(response, "dead") == 0) {
          reg->ui_mode = 14;
          reg->reset_buttons();
          int width;
          int height;
          SDL_GL_GetDrawableSize(window, &width, &height);
          reg->add_button(width / 2 - 100, height / 2 - 37,
                          (char *)"GAME OVER!");
          return;
        } else if (strcmp(response, "nextTurn") == 0) {
          reg->ui_mode = 10;
          reg->reset_buttons();
        } else {
          exit(2);
        }
      } else if (strcmp(task_queue->at(0).at(0), "attack") == 0) {
        com.send_text((char *)"attack");
        com.send_text(task_queue->at(0).at(1));
      } else if (strcmp(task_queue->at(0).at(0), "move") == 0) {
        com.send_text((char *)"move");
        com.send_text(task_queue->at(0).at(1));
      } else if (strcmp(task_queue->at(0).at(0), "get_status") == 0) {
        com.send_text((char *)"get_status");
        com.send_text(task_queue->at(0).at(1));
        int width;
        int height;
        SDL_GL_GetDrawableSize(window, &width, &height);
        reg->add_button(width / 2 - 100, height / 2 - 112,
                        com.receive_text()); // name
        reg->add_button(width / 2 - 100, height / 2 - 37,
                        com.receive_text()); // hp
      } else {
        printf("Unknown network task: %s\n", task_queue->at(0).at(0));
      }
      task_queue->erase(task_queue->begin());
    }
    SDL_Delay(DATA_REFRESH_INTERVAL);
  }
}

void Networking::start() {
  network_thread = new std::thread(&Networking::network_job, this);
}

void Networking::stop() {
  nKeepRunning = (bool *)false;
  network_thread->join();
}

void Networking::add_task(char *type) {
  task_queue->push_back({type, (char *)"NULL"});
}

void Networking::add_task(char *type, char *options) {
  task_queue->push_back({type, options});
}

string get_destination(SDL_Event event, const Uint8 *keyboard_state) {
  int width;
  int height;
  SDL_GL_GetDrawableSize(window, &width, &height);
  reg->reset_buttons();
  reg->add_button(width - 200, height - 75, (char *)"Choose a destination");
  while (true) {
    SDL_WaitEvent(&event);
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      SDL_PumpEvents();
      if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
        return "";
      }
    } else if (event.type == SDL_MOUSEBUTTONDOWN &&
               event.button.button == SDL_BUTTON_LEFT) {
      std::vector<int> clickable =
          hre.get_clickable(event.button.x, event.button.y);
      if (clickable[0] == 2) {
        return to_string(clickable[1]) + " . " + to_string(clickable[2]);
      } else {
        return "";
      }
    }
  }
}

int process_left_click(SDL_Event event, const Uint8 *keyboard_state) {
  std::vector<int> clickable =
      hre.get_clickable(event.button.x, event.button.y);
  if (clickable.at(0) == 2) {
    if (reg->ui_mode == 10) {
      int width;
      int height;
      SDL_GL_GetDrawableSize(window, &width, &height);
      if (reg->get_objects().at(clickable[2]).at(clickable[1]) != 0) {
        reg->last_clicked_object = {clickable[1], clickable[2]};
        reg->add_button(width - 200, height - 75, "Attack");
        reg->add_button(width - 200, height - 150, "Move");
        reg->add_button(width - 200, height - 225, "Status");
        reg->add_button(width - 200, height - 300,
                        to_string(clickable[1]) + "." +
                            to_string(clickable[2]));
      } else {
        reg->reset_buttons();
      }
    }
  } else if (clickable.at(0) == 1) {
    if (reg->ui_mode == 11) {
      switch (clickable[1]) {
      case 0:
        reg->ui_mode = 10;
        reg->reset_buttons();
        break;
      case 1:
        return 1;
      }
    } else if (reg->ui_mode == 10) {
      switch (clickable[1]) {
      case 0:
        networking.add_task((char *)"end_turn");
        break;
      case 1: {
        string task_option = "";
        task_option += to_string(reg->last_clicked_object.at(0));
        task_option += " . ";
        task_option += to_string(reg->last_clicked_object.at(1));
        string destination = get_destination(event, keyboard_state);
        reg->reset_buttons();
        if (destination == "")
          break;
        task_option += " . ";
        task_option += destination;
        // not a beautiful way to turn a string into a char* but everything
        // else doesn't work
        char *reformatted = new char[task_option.length() + 1];
        reformatted[task_option.length()] = '\0';
        for (int i = 0; i < (int)task_option.length(); i++) {
          reformatted[i] = task_option[i];
        }
        networking.add_task((char *)"attack", reformatted);
        break;
      }
      case 2: {
        string task_option = "";
        task_option += to_string(reg->last_clicked_object.at(0));
        task_option += " . ";
        task_option += to_string(reg->last_clicked_object.at(1));
        string destination = get_destination(event, keyboard_state);
        reg->reset_buttons();
        if (destination == "")
          break;
        task_option += " . ";
        task_option += destination;
        // not a beautiful way to turn a string into a char* but everything
        // else doesn't work
        char *reformatted = new char[task_option.length() + 1];
        reformatted[task_option.length()] = '\0';
        for (int i = 0; i < (int)task_option.length(); i++) {
          reformatted[i] = task_option[i];
        }
        networking.add_task((char *)"move", reformatted);
        break;
      }
      case 3: {
        string task_option = "";
        task_option += to_string(reg->last_clicked_object.at(0));
        task_option += " . ";
        task_option += to_string(reg->last_clicked_object.at(1));
        // not a beautiful way to turn a string into a char* but everything else
        // doesn't work
        char *reformatted = new char[task_option.length() + 1];
        reformatted[task_option.length()] = '\0';
        for (int i = 0; i < (int)task_option.length(); i++) {
          reformatted[i] = task_option[i];
        }
        networking.add_task((char *)"get_status", reformatted);
        reg->ui_mode = 12;
        reg->reset_buttons();
        break;
      }
      }
    } else if (reg->ui_mode == 13) {
      if (clickable[1] == 1) {
        // too buggy (doesn't kill the waiting network thread)
        // might be reenabled later
        // return 1;
      }
    } else if (reg->ui_mode == 14) {
      if (clickable[1] == 0) {
        return 1;
      }
    }
  } else {
    if (reg->ui_mode == 10) {
      reg->reset_buttons();
    }
  }
  return 0;
}

void process_input() {
  SDL_Event event;
  const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
  while (reg->keep_running) {
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      } else if (event.type == SDL_MOUSEWHEEL && reg->ui_mode == 10) {
        if (event.wheel.y > 0) // scroll up
          reg->rendersize += 4;
        else if (event.wheel.y < 0) // scroll down
          reg->rendersize -= 4;
      } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDL_PumpEvents();
        int move_interval = 4;
        if (keyboard_state[SDL_SCANCODE_LSHIFT]) {
          move_interval = 12;
        }
        if (keyboard_state[SDL_SCANCODE_W] &&
            !(keyboard_state[SDL_SCANCODE_S]) && reg->ui_mode == 10) {
          reg->camera_position_y += move_interval;
        }
        if (keyboard_state[SDL_SCANCODE_A] &&
            !(keyboard_state[SDL_SCANCODE_D]) && reg->ui_mode == 10) {
          reg->camera_position_x += move_interval;
        }
        if (keyboard_state[SDL_SCANCODE_S] &&
            !(keyboard_state[SDL_SCANCODE_W]) && reg->ui_mode == 10) {
          reg->camera_position_y -= move_interval;
        }
        if (keyboard_state[SDL_SCANCODE_D] &&
            !(keyboard_state[SDL_SCANCODE_A]) && reg->ui_mode == 10) {
          reg->camera_position_x -= move_interval;
        }
        if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
          if (reg->ui_mode == 10) {
            reg->ui_mode = 11;
            reg->reset_buttons();
            reg->add_button(75, 75, "Continue");
            reg->add_button(75, 160, "Exit");
          } else if (reg->ui_mode == 11) {
            reg->ui_mode = 10;
            reg->reset_buttons();
          } else if (reg->ui_mode == 12) {
            reg->ui_mode = 10;
            reg->reset_buttons();
          } else if (reg->ui_mode == 13) {
            // too buggy (doesn't kill the waiting network thread)
            // might be reenabled later
            // reg->add_button(75, 160, "Exit");
          }
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
          if (process_left_click(event, keyboard_state) == 1) {
            return;
          }
        }
      }
    }
  }
}

void ui_start() {
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  window = SDL_CreateWindow(
      "HEXEL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN);
  hre = HRE(window);
  reg->window = window;
  reg->reset_buttons();
  process_input();
  hre.stop();
  networking.stop();
  SDL_DestroyWindow(window);
  window = nullptr;
  reg->deinit();
  TTF_Quit();
  SDL_Quit();
}

int main(int argc, char **argv) {
  networking = Networking((char *)"127.0.0.1", 43233);
  networking.start();
  ui_start();
}
