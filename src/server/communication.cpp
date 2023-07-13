#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 1024

class Communication {
public:
  Communication();
  Communication(int port);
  void stop();
  char *receive_text();
  void send_text(char *message);

private:
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;
  socklen_t socket_length;
};

Communication::Communication(){
  // do nothing
}

Communication::Communication(int port) {
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(1);
  }
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(1);
  }
  socket_length = sizeof(cliaddr);
}

void Communication::send_text(char *message) {
  sendto(sockfd, (const char *)message, strlen(message), MSG_CONFIRM,
         (const struct sockaddr *)&cliaddr, socket_length);
}

char *Communication::receive_text() {
  char *buffer = new char[MAXLINE];
  int n;
  n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
               (struct sockaddr *)&cliaddr, &socket_length);
  buffer[n] = '\0';
  return buffer;
}
