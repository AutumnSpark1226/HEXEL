#include "data_registry.cpp"
#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_TRANSMISSION_LENGTH 1024

Registry *reg;

class Communication {
public:
  Communication();
  Communication(int port);
  void accept_client();
  char *receive_text(int client_socket_id);
  void send_text(int client_socket_id, char *message);
  void check_clients();

private:
  int master_socket;
  int client_sockets[30];
  struct sockaddr_in address;
  int addrlen, new_socket, max_clients = 30, sd, activity, max_sd;
  fd_set readfds;
  void process_received_data(int client_socket_id, char *req);
  char *get_game_data(char *id);
};

Communication::Communication() {
  // do nothing
}

Communication::Communication(int port) {
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

void Communication::send_text(int client_socket_id, char *message) {
  std::string temp(message);
  uint32_t len = temp.length();
  send(client_sockets[client_socket_id], &len, 4, 0);
  send(client_sockets[client_socket_id], message, len, 0);
}

char *Communication::receive_text(int client_socket_id) {
  uint32_t len = 0;
  read(client_sockets[client_socket_id], &len, 4);
  int valread;
  char buffer[len];
  valread = read(client_sockets[client_socket_id], buffer, len);
  if (valread == 0) {
    client_sockets[client_socket_id] = 0;
    return nullptr;
  } else {
    buffer[valread] = '\0';
    char *output = (char *)buffer;
    return output;
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
    printf("client accepted\n");
    string temp_str = to_string(game_map->at(0).size());
    send_text(new_socket_id, temp_str.data()); // working since c++17
    temp_str = to_string(game_map->size());
    send_text(new_socket_id, temp_str.data()); // working since c++17
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

void Communication::process_received_data(int client_socket_id, char *req) {
  if (req == nullptr) {
    return;
  }
  if (strcmp(req, (char *)"request_data") == 0) {
    send_text(client_socket_id, (char *)"await_id");
    req = receive_text(client_socket_id);
    send_text(client_socket_id, get_game_data(req));
  } else {
    send_text(client_socket_id, (char *)"notFound");
    printf("Command not found: %s\n", req);
  }
}

char *Communication::get_game_data(char *id) {
  if (strcmp(id, (char *)"map") == 0) {
    string output = reg->export_map();
    // not a beautiful way to turn a string into a char* but everything else
    // doen't work
    char *reformatted_output = new char[output.length() + 1];
    reformatted_output[output.length()] = '\0';
    for (int i = 0; i < (int)output.length(); i++) {
      reformatted_output[i] = output[i];
    }
    return reformatted_output;
  } else if (strcmp(id, (char *)"objects") == 0) {
    string output = reg->export_objects();
    // not a beautiful way to turn a string into a char* but everything else
    // doesn't work
    char *reformatted_output = new char[output.length() + 1];
    reformatted_output[output.length()] = '\0';
    for (int i = 0; i < (int)output.length(); i++) {
      reformatted_output[i] = output[i];
    }
    return reformatted_output;
  } else {
    printf("Data ID not found: %s\n", id);
    return (char *)"IDnotFound";
  }
}
