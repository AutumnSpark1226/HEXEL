#include "data_registry.cpp"
#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

Registry *reg;

class Communication {
public:
  Communication();
  Communication(int port);
  void accept_client();
  string receive_text(int client_socket_id);
  void send_text(int client_socket_id, string message);
  void check_clients();

private:
  int master_socket;
  int client_sockets[30];
  struct sockaddr_in address;
  int addrlen, new_socket, max_clients = 30, sd, activity, max_sd;
  fd_set readfds;
  void process_received_data(int client_socket_id, string req);
  string get_game_data(int client_socket_id, string id);
};

Communication::Communication() {
  // do nothing
}

Communication::Communication(int port) {
  task_queue->clear();
  int opt = 1;
  for (int i = 0; i < max_clients; i++) {
    client_sockets[i] = 0;
  }
  if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("error");
    exit(1);
  }
  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                 sizeof(opt)) < 0) {
    perror("error");
    exit(1);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(1);
  }
  if (listen(master_socket, 3) < 0) {
    perror("error");
  }
  addrlen = sizeof(address);
  printf("init done\n");
}

void Communication::send_text(int client_socket_id, string message) {
  try {
    uint32_t len = message.length();
    send(client_sockets[client_socket_id], &len, 4, 0);
    send(client_sockets[client_socket_id], message.c_str(), len, 0);
  } catch (...) {
    perror("sending failed for some reason");
  }
}

string Communication::receive_text(int client_socket_id) {
  try {
    uint32_t len = 0;
    read(client_sockets[client_socket_id], &len, 4);
    int valread;
    char buffer[len];
    valread = read(client_sockets[client_socket_id], buffer, len);
    if (valread == 0) {
      client_sockets[client_socket_id] = 0;
      reg->delete_user(client_socket_id);
      return "";
    } else {
      buffer[valread] = '\0';
      char *output = (char *)buffer;
      return output;
    }
  } catch (...) {
    perror("receiving failed for some reason");
    return "";
  }
}

void Communication::accept_client() {
  FD_ZERO(&readfds);
  FD_SET(master_socket, &readfds);
  max_sd = master_socket;
  for (int i = 0; i < max_clients; i++) {
    sd = client_sockets[i];
    if (sd > 0)
      FD_SET(sd, &readfds);
    if (sd > max_sd)
      max_sd = sd;
  }
  activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
  if (FD_ISSET(master_socket, &readfds)) {
    if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("error: couldn't accept client");
      exit(1);
    }
    int new_socket_id;
    for (int i = 0; i < max_clients; i++) {
      if (client_sockets[i] == 0) {
        client_sockets[i] = new_socket;
        new_socket_id = i;
        break;
      }
    }
    reg->process_user_join(new_socket_id);
    printf("client accepted\n");
    string temp_str = to_string(game_map->at(0).size());
    send_text(new_socket_id, temp_str);
    temp_str = to_string(game_map->size());
    send_text(new_socket_id, temp_str);
    receive_text(new_socket_id);
  }
}

void Communication::check_clients() {
  for (int i = 0; i < max_clients; i++) {
    sd = client_sockets[i];
    if (FD_ISSET(sd, &readfds)) {
      process_received_data(i, receive_text(i));
    }
  }
}

void Communication::process_received_data(int client_socket_id, string req) {
  if (req == "") {
    return;
  }
  if (strcmp(req.data(), "request_data") == 0) {
    send_text(client_socket_id, (char *)"await_id");
    req = receive_text(client_socket_id);
    send_text(client_socket_id, get_game_data(client_socket_id, req));
  } else if (strcmp(req.data(), "end_turn") == 0) {
    (*turn_finished)[client_socket_id] = true;
    bool all_finished = true;
    for (int i = 0; i < max_clients; i++) {
      if (client_sockets[i] != 0 && !(*turn_finished)[i]) {
        all_finished = false;
      }
    }
    if (all_finished) {
      reg->increment_turn_count();
      reg->process_queue();
      while (processing_outputs->size() > 0) {
        if (strcmp(processing_outputs->at(0).at(0).data(), "dead") == 0) {
          printf("%s\n", "Player died");
          send_text(stoi(processing_outputs->at(0).at(1)), "dead");
          client_sockets[stoi(processing_outputs->at(0).at(1))] = 0;
        }
        processing_outputs->erase(processing_outputs->begin());
      }
      for (int i = 0; i < max_clients; i++) {
        if (client_sockets[i] != 0) {
          send_text(i, "nextTurn");
          (*turn_finished)[i] = false;
        }
      }
    }
  } else if (strcmp(req.data(), "attack") == 0) {
    req = receive_text(client_socket_id);
    if (!(*turn_finished)[client_socket_id]) {
      reg->add_task((char *)"attack", req, client_socket_id);
    }
  } else if (strcmp(req.data(), "move") == 0) {
    req = receive_text(client_socket_id);
    if (!(*turn_finished)[client_socket_id]) {
      reg->add_task((char *)"move", req, client_socket_id);
    }
  } else if (strcmp(req.data(), "get_status") == 0) {
    req = receive_text(client_socket_id);
    vector<string> options = tokenize(req, " . ");
    send_text(client_socket_id,
              reg->get_object_name(client_socket_id, stoi(options.at(0)),
                                   stoi(options.at(1)))
                  .data());
    send_text(client_socket_id,
              ("HP: " +
               to_string(hp->at(stoi(options.at(1))).at(stoi(options.at(0)))))
                  .data());
  } else {
    send_text(client_socket_id, (char *)"notFound");
    printf("Command not found: %s\n", req.c_str());
  }
}

string Communication::get_game_data(int client_socket_id, string id) {
  if (strcmp(id.data(), "map") == 0) {
    string output = reg->export_map(client_socket_id);
    return output;
  } else if (strcmp(id.data(), "objects") == 0) {
    string output = reg->export_objects(client_socket_id);
    return output;
  } else {
    printf("Data ID not found: %s\n", id.c_str());
    return "IDnotFound";
  }
}
