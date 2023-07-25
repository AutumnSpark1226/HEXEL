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

#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

class Communication {
public:
  Communication();
  Communication(char *host, int port);
  void stop();
  char *receive_text();
  void send_text(char *message);
  char *request_data(char *id);

private:
  int server_socket;
  struct sockaddr_in server;
};

Communication::Communication() {
  // do nothing
}

Communication::Communication(char *host, int port) {
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("error: couldn't create server_socketet");
    exit(1);
  }
  server.sin_addr.s_addr = inet_addr(host);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  if (connect(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("error: couldn't connect");
    exit(1);
  }
}

void Communication::send_text(char *message) {
  std::string temp(message);
  uint32_t len = temp.length();
  send(server_socket, &len, 4, 0);
  send(server_socket, message, len, 0);
}

char *Communication::receive_text() {
  uint32_t len = 0;
  read(server_socket, &len, 4);
  int valread;
  char buffer[len];
  valread = read(server_socket, buffer, len);
  if (valread == 0) {
    server_socket = 0;
    exit(1);
  } else {
    char *reformatted_output = new char[valread + 1];
    reformatted_output[valread] = '\0';
    for (int i = 0; i < valread; i++) {
      reformatted_output[i] = buffer[i];
    }
    return reformatted_output;
  }
}

char *Communication::request_data(char *id) {
  send_text((char *)"request_data");
  char *response = receive_text();
  if (strcmp(response, "await_id") == 0) {
    send_text(id);
    char *result = receive_text();
    return result;
  } else {
    printf("ERROR! response was: %s\n", response);
    return (char *)"notFound";
  }
}
