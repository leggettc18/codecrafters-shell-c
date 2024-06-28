#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
  char name[100];
  char *args[100];
  int num_args;
} cmd_t;

typedef struct {
  char *paths[100];
  int num_paths;
} path_t;

void parse_command(char *input, cmd_t *cmd) {
  // Parse command name
  for (int i = 0; i < strlen(input); i++) {
    if (input[i] == ' ' || input[i] == '\n' || input[i] == '\0') {
      cmd->name[i] = '\0';
      break;
    }
    cmd->name[i] = input[i];
  }
  // Parse command arguments
  char *args_str = input + strlen(cmd->name) + 1;
  cmd->num_args = 0;
  for (int i = 0; i < strlen(args_str); i++) {
    if (args_str[i] == ' ' || args_str[i] == '\n' || args_str[i] == '\0') {
      cmd->args[cmd->num_args] = (char *)malloc(i + 1);
      strncpy(cmd->args[cmd->num_args], args_str, i);
      cmd->args[cmd->num_args][i] = '\0';
      cmd->num_args++;
      args_str += i + 1;
      i = 0;
    }
  }
}

void parse_path(path_t *path) {
  char *path_str = getenv("PATH");
  path->num_paths = 0;
  for (int i = 0; i <= strlen(path_str); i++) {
    if (path_str[i] == ':' || path_str[i] == '\0') {
      path->paths[path->num_paths] = (char *)malloc(i + 1);
      strncpy(path->paths[path->num_paths], path_str, i);
      path->paths[path->num_paths][i] = '\0';
      path->num_paths++;
      path_str += i + 1;
      i = 0;
    }
  }
}

int is_executable(const char *path) { return access(path, X_OK) == 0; }

char *find_in_path(const char *command, path_t *path) {
  static char full_path[1024];
  for (int i = 0; i < path->num_paths; i++) {
    snprintf(full_path, sizeof(full_path), "%s/%s", path->paths[i], command);
    if (is_executable(full_path)) {
      return full_path;
    }
  }
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

int handle_input(cmd_t *cmd, path_t *path) {
  if (strcmp(cmd->name, "exit") == 0) {
    int arg = 0;
    sscanf(cmd->args[0], "%d", &arg);
    exit(arg);
  }
  if (strcmp(cmd->name, "echo") == 0) {
    for (int i = 0; i < cmd->num_args; i++) {
      printf("%s", cmd->args[i]);
      if (i < cmd->num_args - 1) {
        printf(" ");
      }
    }
    printf("\n");
    return 0;
  }
  if (strcmp(cmd->name, "pwd") == 0) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 0;
  }
  if (strcmp(cmd->name, "cd") == 0) {
    if (strcmp(cmd->args[0], "~") == 0) {
      cmd->args[0] = getenv("HOME");
    }
    if (chdir(cmd->args[0]) == 0) {
      return 0;
    } else {
      printf("cd: %s: No such file or directory\n", cmd->args[0]);
      return 0;
    }
  }
  if (strcmp(cmd->name, "type") == 0) {
    if (!strcmp(cmd->args[0], "exit") || !strcmp(cmd->args[0], "echo") ||
        !strcmp(cmd->args[0], "type") || !strcmp(cmd->args[0], "pwd")) {
      printf("%s is a shell builtin\n", cmd->args[0]);
      return 0;
    } else {
      char *full_path = find_in_path(cmd->args[0], path);
      if (full_path) {
        printf("%s is %s\n", cmd->args[0], full_path);
        return 0;
      } else {
        printf("%s: not found\n", cmd->args[0]);
        return 0;
      }
    }
  } else {
    char *argv[10];
    int argc = cmd->num_args + 1;
    argv[0] = cmd->name;
    for (int i = 1; i < cmd->num_args + 1; i++) {
      argv[i] = cmd->args[i - 1];
    }
    char *cmd_path = find_in_path(argv[0], path);
    if (cmd_path) {
      fork_and_exec_cmd(cmd_path, argc, argv);
      return 0;
    } else {
      return 1;
    }
  }
}

int main() {
  path_t path;
  parse_path(&path);
  while (true) {
    printf("$ ");
    fflush(stdout);

    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);
    cmd_t cmd;
    parse_command(input, &cmd);

    if (handle_input(&cmd, &path) != 0) {
      printf("%s: command not found\n", cmd.name);
    }
  }
  return 0;
}
