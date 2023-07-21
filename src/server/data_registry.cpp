/* data cache and management for the server
 */
#include <iostream>
#include <random>
#include <stdio.h>
#include <string.h>
#include <vector>
using namespace std;

#define INIT_MAPSIZE 2

vector<vector<int>> *game_map =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
vector<vector<int>> *objects =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
vector<vector<int>> *hp =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
vector<vector<int>> *owner =
    new vector<vector<int>>(INIT_MAPSIZE, vector<int>(INIT_MAPSIZE));
vector<vector<bool>> *performed_action =
    new vector<vector<bool>>(INIT_MAPSIZE, vector<bool>(INIT_MAPSIZE));
vector<bool> *turn_finished = new vector<bool>(30);
vector<vector<string>> *task_queue =
    new vector<vector<string>>(1, vector<string>(3));
vector<vector<string>> *processing_outputs =
    new vector<vector<string>>(1, vector<string>(2));

int turn_count = 0;

vector<string> tokenize(string input, string delimiter) {
  vector<string> output;
  int start, end = -1 * delimiter.size();
  while (end != -1) {
    start = end + delimiter.size();
    end = input.find(delimiter, start);
    output.push_back(input.substr(start, end - start));
  }
  return output;
}

class Registry {
public:
  Registry();
  void init(int game_map_size_x, int game_map_size_y);
  void deinit();
  vector<vector<int>> get_tiles();
  string export_map(int client_socket_id);
  vector<vector<int>> get_objects();
  string export_objects(int client_socket_id);
  string get_object_name(int client_socket_id, int x, int y);
  void process_user_join(int new_socket_id);
  void delete_user(int client_socket_id);
  void increment_turn_count();
  void add_task(string type, string options, int client_socket_id);
  void process_queue();
};

Registry::Registry() {
  // do something
}

void Registry::init(int game_map_size_x, int game_map_size_y) {
  turn_count = 0;
  turn_finished->clear();
  processing_outputs->clear();
  for (size_t i = 0; i < 30; i++) {
    turn_finished->push_back(false);
  }
  random_device dev;
  mt19937 rng(dev());
  if (game_map->size() == INIT_MAPSIZE) {
    uniform_int_distribution<mt19937::result_type> dist2_4(2, 4);
    game_map->clear();
    for (int y = 0; y < game_map_size_y; y++) {
      vector<int> inner_vector(game_map_size_x);
      for (int x = 0; x < game_map_size_x; x++) {
        inner_vector[x] = dist2_4(rng);
      }
      game_map->push_back(inner_vector);
    }
  }
  if (objects->size() == INIT_MAPSIZE) {
    objects->clear();
    for (int y = 0; y < (int)game_map->size(); y++) {
      vector<int> inner_vector(game_map->at(y).size());
      for (int x = 0; x < (int)game_map->at(y).size(); x++) {
        // TODO
      }
      objects->push_back(inner_vector);
    }
  }
  hp->clear();
  for (int y = 0; y < (int)game_map->size(); y++) {
    vector<int> inner_vector(game_map->at(y).size());
    for (int x = 0; x < (int)game_map->at(y).size(); x++) {
      if (objects->at(y).at(x) == 1) {
        inner_vector[x] = 50;
      }
    }
    hp->push_back(inner_vector);
  }
  owner->clear();
  for (int y = 0; y < (int)game_map->size(); y++) {
    vector<int> inner_vector(game_map->at(y).size());
    for (int x = 0; x < (int)game_map->at(y).size(); x++) {
      inner_vector[x] = -1;
    }
    owner->push_back(inner_vector);
  }
  performed_action->clear();
  for (int y = 0; y < (int)game_map->size(); y++) {
    vector<bool> inner_vector(game_map->at(y).size());
    for (int x = 0; x < (int)game_map->at(y).size(); x++) {
      inner_vector[x] = false;
    }
    performed_action->push_back(inner_vector);
  }
}

void Registry::deinit() {
  // do something
}

std::vector<vector<int>> Registry::get_tiles() { return (*game_map); }

std::vector<vector<int>> Registry::get_objects() { return (*objects); }

string Registry::export_map(int client_socket_id) {
  string output = "";
  for (int y = 0; y < (int)(*game_map).size(); y++) {
    for (int x = 0; x < (int)(*game_map)[0].size(); x++) {
      output += to_string(game_map->at(y).at(x));
      output += ", ";
    }
    output += ";";
  }
  return output;
}

string Registry::export_objects(int client_socket_id) {
  string output = "";
  for (int y = 0; y < (int)objects->size(); y++) {
    for (int x = 0; x < (int)objects->at(0).size(); x++) {
      if (objects->at(y).at(x) == 0) {
        output += "0";
      } else if (objects->at(y).at(x) == 1) {
        if (owner->at(y).at(x) == client_socket_id) {
          output += "1";
        } else {
          output += "2";
        }
      } else if (owner->at(y).at(x) == client_socket_id &&
                 objects->at(y).at(x) == 2) {
        output += "3";
      } else {
        output += "4";
      }
      output += ", ";
    }
    output += ";";
  }
  return output;
}

string Registry::get_object_name(int client_socket_id, int x, int y) {
  switch (objects->at(y).at(x)) {
  case 1:
    if (owner->at(y).at(x) == client_socket_id) {
      return "base";
    } else {
      return "enemy base";
    }
  case 2:
    if (owner->at(y).at(x) == client_socket_id) {
      return "unit";
    } else {
      return "enemy";
    }
  default:
    return "notFound";
  }
}

void Registry::process_user_join(int new_socket_id) {
  random_device dev;
  mt19937 rng(dev());
  uniform_int_distribution<mt19937::result_type> dist_map_size_y(
      0, game_map->size() - 1);
  uniform_int_distribution<mt19937::result_type> dist_map_size_x(
      0, game_map->at(0).size() - 1);
  int y = dist_map_size_y(rng);
  int x = dist_map_size_x(rng);
  (*turn_finished)[new_socket_id] = false;
  if (objects->at(y).at(x) == 0) {
    (*objects)[y][x] = 1;
    (*owner)[y][x] = new_socket_id;
    (*hp)[y][x] = 1000;
  } else {
    process_user_join(new_socket_id);
  }
}

void Registry::delete_user(int client_socket_id) {
  for (int y = 0; y < (int)objects->size(); y++) {
    for (int x = 0; x < (int)objects->at(0).size(); x++) {
      if (owner->at(y).at(x) == client_socket_id) {
        (*objects)[y][x] = 0;
        (*hp)[y][x] = 0;
        (*owner)[y][x] = -1;
      }
    }
  }
}

void Registry::increment_turn_count() { turn_count++; }

void Registry::add_task(string type, string options, int client_socket_id) {
  task_queue->push_back({type, options, to_string(client_socket_id)});
}

void Registry::process_queue() {
  while (task_queue->size() > 0) {
    if (strcmp(task_queue->at(0).at(0).data(), "attack") == 0) {
      vector<string> options = tokenize(task_queue->at(0).at(1), " . ");
      if (options.size() == 4) {
        int attacker_x = stoi(options[0]);
        int attacker_y = stoi(options[1]);
        int target_x = stoi(options[2]);
        int target_y = stoi(options[3]);
        if (task_queue->at(0).at(2) ==
                to_string(owner->at(attacker_y).at(attacker_x)) &&
            objects->at(attacker_y).at(attacker_x) == 2 &&
            (objects->at(target_y).at(target_x) == 1 ||
             objects->at(target_y).at(target_x) == 2) &&
            (abs(attacker_x - target_x) == 1 ||
             abs(attacker_y - target_y) == 1) &&
            !performed_action->at(attacker_y).at(attacker_x)) {
          (*performed_action)[attacker_y][attacker_x] = true;
          random_device dev;
          mt19937 rng(dev());
          uniform_int_distribution<mt19937::result_type> dist10_100(10, 100);
          (*hp)[target_y][target_x] -= dist10_100(rng);
          if ((*hp)[target_y][target_x] <= 0) {
            if ((*objects)[target_y][target_x] == 1) {
              processing_outputs->push_back(
                  {"dead", to_string((*owner)[target_y][target_x])});
              delete_user((*owner)[target_y][target_x]);
            }
            (*objects)[target_y][target_x] = 0;
            (*hp)[target_y][target_x] = 0;
            (*owner)[target_y][target_x] = -1;
          }
        } else if (task_queue->at(0).at(2) ==
                       to_string(owner->at(attacker_y).at(attacker_x)) &&
                   objects->at(attacker_y).at(attacker_x) == 1 &&
                   objects->at(target_y).at(target_x) == 0 &&
                   (abs(attacker_x - target_x) == 1 ||
                    abs(attacker_y - target_y) == 1) &&
                   !performed_action->at(attacker_y).at(attacker_x)) {
          (*performed_action)[attacker_y][attacker_x] = true;
          (*objects)[target_y][target_x] = 2;
          (*owner)[target_y][target_x] = stoi(task_queue->at(0).at(2));
          (*hp)[target_y][target_x] = 100;
        }
      }
    } else if (strcmp(task_queue->at(0).at(0).data(), "move") == 0) {
      vector<string> options = tokenize(task_queue->at(0).at(1), " . ");
      if (options.size() == 4) {
        int object_x = stoi(options[0]);
        int object_y = stoi(options[1]);
        int destination_x = stoi(options[2]);
        int destination_y = stoi(options[3]);
        if (task_queue->at(0).at(2) ==
                to_string(owner->at(object_y).at(object_x)) &&
            objects->at(object_y).at(object_x) == 2 &&
            objects->at(destination_y).at(destination_x) == 0 &&
            (abs(object_x - destination_x) == 1 ||
             abs(object_y - destination_y) == 1) &&
            !performed_action->at(object_y).at(object_x)) {
          (*performed_action)[destination_y][destination_x] = true;
          (*objects)[destination_y][destination_x] =
              (*objects)[object_y][object_x];
          (*objects)[object_y][object_x] = 0;
          (*owner)[destination_y][destination_x] = (*owner)[object_y][object_x];
          (*owner)[object_y][object_x] = -1;
          (*hp)[destination_y][destination_x] = (*hp)[object_y][object_x];
          (*hp)[object_y][object_x] = 0;
        }
      }
    }
    task_queue->erase(task_queue->begin());
  }
  for (int y = 0; y < (int)game_map->size(); y++) {
    for (int x = 0; x < (int)game_map->at(y).size(); x++) {
      (*performed_action)[y][x] = false;
    }
  }
}
