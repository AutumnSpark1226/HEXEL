/* data cache and management for the server
 */
#include <iostream>
#include <random>
#include <stdio.h>
#include <vector>
using namespace std;

#define INIT_MAPSIZE 8

// allocate a lot of memory to prevent segfaults
vector<vector<int>> *game_map =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
vector<vector<int>> *objects =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));

class Registry {
public:
  Registry();
  void init(int game_map_size_x, int game_map_size_y);
  void deinit();
  vector<vector<int>> get_tiles();
  string export_map();
  vector<vector<int>> get_objects();
  string export_objects();
};

Registry::Registry() {
  // do something
}

void Registry::init(int game_map_size_x, int game_map_size_y) {
  random_device dev;
  mt19937 rng(dev());
  uniform_int_distribution<mt19937::result_type> dist3(1, 3);
  game_map->clear();
  for (int y = 0; y < game_map_size_y; y++) {
    vector<int> inner_vector(game_map_size_x);
    for (int x = 0; x < game_map_size_x; x++) {
      inner_vector[x] = dist3(rng);
    }
    game_map->push_back(inner_vector);
  }
  uniform_int_distribution<mt19937::result_type> dist4(0, 3);
  objects->clear();
  for (int y = 0; y < game_map_size_y; y++) {
    vector<int> inner_vector(game_map_size_x);
    for (int x = 0; x < game_map_size_x; x++) {
      inner_vector[x] = dist4(rng);
    }
    objects->push_back(inner_vector);
  }
}

void Registry::deinit() {
  // do something
}

std::vector<vector<int>> Registry::get_tiles() { return (*game_map); }

std::vector<vector<int>> Registry::get_objects() { return (*objects); }

string Registry::export_map() {
  string output = "";
  for (int y = 0; y < (int)(*game_map).size(); y++) {
    for (int x = 0; x < (int)(*game_map)[0].size(); x++) {
      output += to_string(game_map->at(y).at(x));
      output += ", ";
    }
    output += "; ";
  }
  return output;
}

string Registry::export_objects() {
  string output = "";
  for (int y = 0; y < (int)(*objects).size(); y++) {
    for (int x = 0; x < (int)(*objects)[0].size(); x++) {
      output += to_string(objects->at(y).at(x));
      output += ", ";
    }
    output += "; ";
  }
  return output;
}
