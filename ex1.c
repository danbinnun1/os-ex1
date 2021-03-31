#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define COMMAND_MAX_LENGTH 100
#define MAX_COMMANDS 100
#define IN_QUOTATION 0
#define IN_SPACE 1
#define IN_WORD 2
#define DONE "DONE"
#define RUNNING "RUNNING"
#define COMMAND_FINISHED 0

typedef struct {
    char** data;
    uint8_t length;
} StringsArray;

typedef struct {
    char str[COMMAND_MAX_LENGTH];
    pid_t pid;
} command;

StringsArray parseCommand(const char* command) {
    char args[COMMAND_MAX_LENGTH][COMMAND_MAX_LENGTH];
    const char* offset = command;
    uint8_t length = 0;
    uint8_t argNum = 0;
    uint8_t state;
    if (*command == '"') {
        state = IN_QUOTATION;
    } else if (*command == ' ' || *command == '\t') {
        state = IN_SPACE;
    } else {
        state = IN_WORD;
    }
    while (*offset != '\0') {
        switch (state) {
            case IN_QUOTATION:
                if (*offset == '"') {
                    state = IN_WORD;
                } else {
                    args[argNum][length] = *offset;
                    length++;
                }
                break;
            case IN_SPACE:
                if (*offset == '"') {
                    state = IN_QUOTATION;
                    length = 0;
                } else if (*offset != ' ' && *offset != '\t') {
                    state = IN_WORD;
                    length = 0;
                    args[argNum][length] = *offset;
                    length++;
                }
                break;
            case IN_WORD:
                if (*offset == '"') {
                    state = IN_QUOTATION;
                } else if (*offset != ' ' && *offset != '\t') {
                    args[argNum][length] = *offset;
                    length++;
                } else {
                    args[argNum][length] = '\0';
                    state = IN_SPACE;
                    argNum++;
                }
                break;

            default:
                break;
        }
        offset++;
    }
    args[argNum][length] = '\0';
    char** result = malloc(sizeof(char*) * (argNum + 2));
    for (uint8_t i = 0; i < argNum + 1; i++) {
        result[i] = malloc(strlen(args[i]) + 1);
        strcpy(result[i], args[i]);
    }
    result[argNum + 1] = NULL;
    StringsArray arr;
    arr.data = result;
    arr.length = argNum + 1;
    return arr;
}

int main() {
    command commands[MAX_COMMANDS];
    char lastDir[100];
    uint8_t command_index = 0;
    while (1) {
        printf("$ ");
        fflush(stdout);
        fgets(commands[command_index].str, COMMAND_MAX_LENGTH, stdin);
        uint8_t commandLength = strlen(commands[command_index].str);
        if (commandLength > 0 &&
            commands[command_index].str[commandLength - 1] == '\n')
            commands[command_index].str[commandLength - 1] = '\0';
        commandLength--;
        // split by white character
        StringsArray args = parseCommand(commands[command_index].str);
        if (strcmp(args.data[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(args.data[0], "history") == 0) {
            for (uint8_t i = 0; i < command_index; i++) {
                printf("%s ", commands[i].str);
                if (commands[i].pid == COMMAND_FINISHED) {
                    printf("%s\n", DONE);
                } else if (waitpid(commands[i].pid, NULL, WNOHANG) == 0) {
                    printf("%s\n", RUNNING);
                } else {
                    printf("%s\n", DONE);
                }
            }
            printf("%s\n", "history RUNNING");
        } else if (strcmp(args.data[0], "jobs") == 0) {
            for (uint8_t i = 0; i < command_index; i++) {
                if (commands[i].pid != COMMAND_FINISHED &&
                    waitpid(commands[i].pid, NULL, WNOHANG) == 0) {
                    printf("%s\n", commands[i].str);
                }
            }
        } else if (strcmp(args.data[0], "cd") == 0) {
            char path[100];
            if (args.length == 1) {
                chdir(getenv("HOME"));
            } else if (args.length == 2) {
                if (args.data[1][0] == '-') {
                    strcpy(path, lastDir);
                    if (strlen(args.data[1]) > 1) {
                        strcat(path, args.data[1] + 1);
                    }
                } else if (args.data[1][0] == '~') {
                    strcpy(path, getenv("HOME"));
                    if (strlen(args.data[1]) > 1) {
                        strcat(path, args.data[1] + 1);
                    }
                } else {
                    strcpy(path, args.data[1]);
                }

                getcwd(lastDir, 100);
                if (chdir(path) == -1) {
                    printf("chdir failed\n");
                }
            } else {
                printf("Too many arguments\n");
            }
        } else {
            pid_t pid = fork();
            if (pid == -1) {
                printf("fork failed\n");
            } else if (pid == 0) {
                if (strcmp(args.data[args.length - 1], "&") == 0) {
                    free(args.data[args.length - 1]);
                    args.data[args.length - 1] = NULL;
                }
                if (execvp(args.data[0], args.data) == -1) {
                    printf("exec failed\n");
                    exit(0);
                }
            } else {
                if (strcmp(args.data[args.length - 1], "&") != 0) {
                    waitpid(pid, NULL, 0);
                    commands[command_index].pid = COMMAND_FINISHED;

                } else {
                    uint8_t i = commandLength - 2;
                    for (; commands[command_index].str[i] == ' ' ||
                           commands[command_index].str[i] == '\t';
                         --i)
                        ;
                    commands[command_index].str[i + 1] = '\0';
                    commands[command_index].pid = pid;
                }
            }
        }
        command_index++;
        for (uint8_t i = 0; i < args.length; i++) {
            free(args.data[i]);
        }
        free(args.data);
    }
}