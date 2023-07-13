#include "communication.cpp"
#include <stdio.h>

int main(int argc, char **argv) {
  Communication com = Communication(43233);
  while (true) {
    char *input = com.receive_text();
    std::cout << input << '\n';
    com.send_text(input);
  }
  return 0;
}
