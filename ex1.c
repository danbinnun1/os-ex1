#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define COMMAND_MAX_LENGTH 100
#define IN_QUOTATION 0
#define IN_SPACE 1
#define IN_WORD 2

char** parseCommand(const char* command) {
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
                    argNum++;
                    length = 0;
                } else if (*offset != ' ' && *offset != '\t') {
                    state = IN_WORD;
                    argNum++;
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
    return result;
}

int main() {
    execlp("l", "l", NULL);
    while (1) {
        printf("$ ");
        fflush(stdout);
        char command[COMMAND_MAX_LENGTH];
        fflush(stdin);
        fflush(stdout);
        memset(command, 0 , COMMAND_MAX_LENGTH);
        fgets(command, COMMAND_MAX_LENGTH, stdin);
        if ((strlen(command) > 0) && (command[strlen(command) - 1] == '\n'))
            command[strlen(command) - 1] = '\0';
        // split by white character
        char** args = parseCommand(command);
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                printf("exec failed\n");
                exit(0);
            }
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}