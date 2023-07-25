/*
    HEXEL - a cool strategy game
    Copyright (C) 2023  AutumnSpark1226

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
  printf("\nHEXEL  Copyright (C) 2023  AutumnSpark1226\n");
  printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
  printf("This is free software, and you are welcome to\n");
  printf("redistribute it under certain conditions. For\n");
  printf("details read <https://www.gnu.org/licenses/>\n\n\n");
  read_config();
  reg->init(init_map_size_x, init_map_size_y);
  com = Communication(43233);
  while (true) {
    com.accept_client();
    com.check_clients();
    SDL_Delay(100);
  }
}
