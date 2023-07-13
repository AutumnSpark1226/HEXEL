#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 1024

class Communication {
public:
  Communication();
  Communication(char *host, int port);
  void stop();
  char *receive_text();
  void send_text(char *message);

private:
  int sockfd;
  struct sockaddr_in server_address;
};

char *Communication::receive_text() {
  int n;
  socklen_t socket_length;
  char *buffer = new char[MAXLINE];
  n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
               (struct sockaddr *)&server_address, &socket_length);
  buffer[n] = '\0';
  return buffer;
}

void Communication::send_text(char *message) {
  sendto(sockfd, (const char *)message, strlen(message), MSG_CONFIRM,
         (const struct sockaddr *)&server_address, sizeof(server_address));
}

Communication::Communication() {
  // do nothing
}

Communication::Communication(char *host, int port) {
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(1);
  }
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(host);
}

void Communication::stop() { close(sockfd); }
