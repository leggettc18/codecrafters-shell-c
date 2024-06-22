#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
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

void fork_and_exec_cmd(char *full_path, int argc, char **argv) {
  pid_t pid = fork();
  if (pid == 0) {
    execv(full_path, argv);
    perror("execv");
    exit(1);
  } else if (pid < 0) {
    perror("fork");
  } else {
    int status;
    waitpid(pid, &status, 0);
  }
}

int handle_input(char *input) {
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
  if (!strcmp(command, "pwd")) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 1;
  }
  if (!strcmp(command, "cd")) {
    char *path = input + 3;
    if (strcmp(path, "~") == 0) {
      path = getenv("HOME");
    }
    if (chdir(path) == 0) {
      return 1;
    } else {
      printf("cd: %s: No such file or directory\n", path);
      return 1;
    }
  }
  if (!strcmp(command, "type")) {
    char arg[20];
    sscanf(input, "%*s %s", arg);
    if (!strcmp(arg, "exit") || !strcmp(arg, "echo") || !strcmp(arg, "type") ||
        !strcmp(arg, "pwd")) {
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
  } else {
    char *argv[10];
    int argc = 0;
    char *token = strtok(input, " ");
    while (token != NULL && argc < 10) {
      argv[argc++] = token;
      token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    char *cmd_path = find_in_path(argv[0]);
    if (cmd_path) {
      fork_and_exec_cmd(cmd_path, argc, argv);
      return 1;
    } else {
      return 0;
    }
  }
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
