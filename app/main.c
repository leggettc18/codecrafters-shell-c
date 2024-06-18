#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int handle_input(const char *input) {
  char command[20];
  sscanf(input, "%s", command);
  if (!strcmp(command, "exit")) {
    int arg = 0;
    sscanf(input, "%*s %d", &arg);
    exit(arg);
  }
  return 0;
}

int main() {
  while (true) {
    printf("$ ");
    fflush(stdout);

    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);

    input[strlen(input) - 1] = '\0';
    if (!handle_input(input)) {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
