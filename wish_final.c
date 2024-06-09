#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MAX_COMMAND_LENGTH 256

void cdr(char **arguments);
void exiting(char **arguments);
char *amend_path(char **arguments, char *path);
void bash(int argument_count, char*arguments[]);
void onscreen(int is_interactive);
int cutcmd(char user_command[]);
void difarg(int position);
void exrdr(void);

char error_message[30] = "An error has occurred";


int notPermitted;
int interactiveMode;
char *commands[100];
char *arguments[100];
char *filename;
int outputRedirection;
int multipleFiles;
int currentCommand;
FILE *inputStream;


void starting(void) {
    notPermitted = 0;
    interactiveMode = 1;
    outputRedirection = 0;
    multipleFiles = 0;
    currentCommand = 0;
    inputStream = stdin;
}

void cdr(char **arguments) {
    if (chdir(arguments[1]) == -1) fprintf(stderr, "%s\n", error_message);
}

void exiting(char **arguments) {
    arguments[1] != NULL ? fprintf(stderr, "%s\n", error_message) : exit(0);
}


char *amend_path(char **arguments, char *path) {
    char *pathCopy = strdup(path);
    for (int i = 1; arguments[i] != NULL ? (pathCopy = realloc(pathCopy, strlen(pathCopy) + strlen(arguments[i]) + 2), strcat(pathCopy, ":"), strcat(pathCopy, arguments[i]), setenv("PATH", pathCopy, 1), putenv("PATH"), 1) : 0; i++);
    notPermitted = 0;
    return pathCopy;
}


void bash(int argument_count, char *arguments[]) {
    if (argument_count == 2 && (interactiveMode = 0, (inputStream = fopen(arguments[1], "r")) == NULL)) {
        fprintf(stderr, "%s\n", error_message);
        exit(1);
    }
}

void onscreen(int is_interactive) {
    is_interactive ? printf("wish> ") : (void)0;
}

int cutcmd(char user_command[]) {
    int k = 0;
    for (char *command = strtok(user_command, "&"); command != NULL; commands[k] = command, command = strtok(NULL, "&"), k++);
    return k;
}

void difarg(int position) {
    for (int i = 0; i < 100 && (arguments[i] = strtok(i == 0 ? commands[position] : NULL, " \t\n")) != NULL; i++);
}

void exrdr(void) {
  int j = 0;
  multipleFiles = 0;
  outputRedirection = 0;
  while (arguments[j] != NULL) {
    if (j != 0 && !strcmp(arguments[j], ">")) {
      if (arguments[j + 2] != NULL) {
        multipleFiles = 1;
      } else {
        outputRedirection = 1;
        filename = arguments[j + 1];
        arguments[j] = NULL;
      }
    } else {
      char *position = strchr(arguments[j], '>');
      if (position) {
        if (arguments[j + 1] != NULL) {
          multipleFiles = 1;
        } else {
          char *argument = strtok(arguments[j], ">");
          arguments[j] = argument;
          outputRedirection = 1;
          argument = strtok(NULL, ">");
          filename = argument;
        }
      }
    }
    j++;
  }
}

int main(int argc, char *argv[]) {
    char user_command[MAX_COMMAND_LENGTH];
    char *path = getenv("PATH");
    char *pathCopy = strdup(path);
    int num_commands = 0;
    int breakLoop = 0;
    int found;

    starting();

    if (argc > 2) {
        fprintf(stderr, "%s\n", error_message);
        free(pathCopy);
        exit(1);
    }

    bash(argc, argv);

    do {
    onscreen(interactiveMode);

    if (!fgets(user_command, MAX_COMMAND_LENGTH, inputStream)) {
        free(pathCopy);
        exit(0);
    }

    user_command[strcspn(user_command, "\n")] = '\0';
    if (!strlen(user_command))
        continue;

    num_commands = cutcmd(user_command);

    for (currentCommand = 0; currentCommand < num_commands; currentCommand++) {
        difarg(currentCommand);

        if (!arguments[0])
            continue;

        exrdr();
        
        if (multipleFiles) {
            fprintf(stderr, "%s\n", error_message);
            continue;
        }

        if (!strcmp(arguments[0], "exit")) {
            exiting(arguments);
            breakLoop = 1;
            break;
        } else if (!strcmp(arguments[0], "cd")) {
            cdr(arguments);
        } else if (!strcmp(arguments[0], "path")) {
            notPermitted = (arguments[1] == NULL);
            if (!notPermitted)
                pathCopy = amend_path(arguments, path);
        } else if (!notPermitted) {
            pid_t pickle = fork();
            if (pickle == 0) {
                char *fullpath, *token;
                int i = 0;
                while (!found && (token = strtok(i == 0 ? pathCopy : NULL, ":")) != NULL) {
                    i++;
                    fullpath = malloc(strlen(token) + strlen(arguments[0]) + 2);
                    sprintf(fullpath, "%s/%s", token, arguments[0]);
                    if (access(fullpath, X_OK) == 0) {
                        if (outputRedirection) {
                            int output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                            if (output_fd == -1) {
                                fprintf(stderr, "%s\n", error_message);
                                free(pathCopy);
                                exit(EXIT_FAILURE);
                            }
                            if (dup2(output_fd, STDOUT_FILENO) == -1) {
                                fprintf(stderr, "%s\n", error_message);
                                free(pathCopy);
                                exit(EXIT_FAILURE);
                            }
                        }
                        execv(fullpath, arguments);
                        perror(arguments[0]);
                        free(fullpath);
                        free(pathCopy);
                        exit(EXIT_FAILURE);
                    }
                    free(fullpath);
                }
                if (i == 0)
                    fprintf(stderr, "%s\n", error_message);
                else
                    fprintf(stderr, "%s\n", error_message);
                free(pathCopy);
                exit(EXIT_FAILURE);
            } else if (pickle < 0) {
                fprintf(stderr, "%s\n", error_message);
            }
        } else {
            fprintf(stderr, "%s\n", error_message);
        }
    }
    if (breakLoop)
        break;
    while (wait(NULL) > 0);
} while (1);
    free(pathCopy);
    return 0;
}


