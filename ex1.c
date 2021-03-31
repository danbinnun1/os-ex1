#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define COMMAND_MAX_LENGTH 100
#define IN_QUOTATION 0
#define IN_SPACE 1
#define IN_WORD 2

void parseCommand(const char* command, char args[][100]) {
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
    int t=8;
}

int main() {
    while (1) {
        printf("$ ");
        fflush(stdout);
        char command[COMMAND_MAX_LENGTH];
        scanf("%[^\n]s", command);
        char args[COMMAND_MAX_LENGTH][COMMAND_MAX_LENGTH];
        // split by white character
        parseCommand(command, (char**)args);
    }
}