#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_TRANSMISSION_LENGTH 1024

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
