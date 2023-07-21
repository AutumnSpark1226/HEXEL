#include "communication.cpp"
#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

Communication com;
int init_map_size_x = 15;
int init_map_size_y = 15;

void read_map(string input) {
  vector<string> rows = tokenize(input, ", ;");
  game_map->clear();
  for (int y = 0; y < (int)rows.size() - 1; y++) {
    vector<string> columns = tokenize(rows.at(y), ", ");
    vector<int> inner_vector(columns.size());
    for (int x = 0; x < (int)columns.size(); x++) {
      inner_vector[x] = stoi(columns.at(x));
    }
    game_map->push_back(inner_vector);
  }
}

void load_config(string input) {
  if (input.rfind("#", 0) == 0)
    return;
  vector<string> tokenized_input = tokenize(input, " = ");
  if (tokenized_input.size() == 2) {
    if (strcmp(tokenized_input.at(0).data(), "map_size_x") == 0) {
      init_map_size_x = stoi(tokenized_input.at(1));
    } else if (strcmp(tokenized_input.at(0).data(), "map_size_y") == 0) {
      init_map_size_y = stoi(tokenized_input.at(1));
    } else if (strcmp(tokenized_input.at(0).data(), "map_file") == 0) {
      ifstream file;
      file.open(tokenized_input.at(1));
      string map_data = "";
      string data;
      while (getline(file, data)) {
        // TODO read metadata and objects
        if (!data.rfind("#", 0) == 0)
          map_data += data;
      }
      read_map(map_data);
      file.close();
    } else {
      cout << "parsing error 1: '" << input << "'\n";
    }
  } else {
    cout << "parsing error 0: '" << input << "'\n";
  }
}

void read_config() {
  ifstream file;
  file.open("assets/game_data/server_data/config.txt");
  string data;
  while (getline(file, data)) {
    load_config(data);
  }
  file.close();
}

int main(int argc, char **argv) {
  read_config();
  reg->init(init_map_size_x, init_map_size_y);
  com = Communication(43233);
  while (true) {
    com.accept_client();
    com.check_clients();
    SDL_Delay(100);
  }
}
