#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int is_executable(const char *path) { return access(path, X_OK) == 0; }

char *find_in_path(const char *command) {
  char *path_env = getenv("PATH");
  if (path_env == NULL) {
    return NULL;
  }

  char *path_copy = strdup(path_env);
  char *dir = strtok(path_copy, ":");
  static char full_path[1024];

  while (dir != NULL) {
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
    if (is_executable(full_path)) {
      free(path_copy);
      return full_path;
    }
    dir = strtok(NULL, ":");
  }
  free(path_copy);
  return NULL;
}

int handle_input(const char *input) {
  char command[20];
  sscanf(input, "%s", command);
  if (!strcmp(command, "exit")) {
    int arg = 0;
    sscanf(input, "%*s %d", &arg);
    exit(arg);
  }
  if (!strcmp(command, "echo")) {
    printf("%s\n", input + 5);
    return 1;
  }
  if (!strcmp(command, "type")) {
    char arg[20];
    sscanf(input, "%*s %s", arg);
    if (!strcmp(arg, "exit") || !strcmp(arg, "echo") || !strcmp(arg, "type")) {
      printf("%s is a shell builtin\n", arg);
      return 1;
    } else {
      char *path = find_in_path(arg);
      if (path) {
        printf("%s is %s\n", arg, path);
        return 1;
      } else {
        printf("%s: not found\n", arg);
        return 1;
      }
    }
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
