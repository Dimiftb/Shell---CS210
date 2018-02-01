#include <stdio.h>
#include <string.h>

#define INPUT_SIZE 512
#define DELIMITER_LENGTH 8

void getInput(char *input);
void printTokens(char *input);
void removeNewLine(char *input);

int main() {

    char input[INPUT_SIZE] = {'\0'};
    getInput(input);
    removeNewLine(input);
    printTokens(input);
    //printf("%s",input);
    return 1;
}

void getInput(char *input) {

    printf("> ");
    fgets(input, INPUT_SIZE, stdin);

}

void printTokens(char *input) {

    const char delimiters[] = {' ', '\t', '|','>','<','&',';','\0'};
    char* token;

    token = strtok(input,delimiters);
    while(token != NULL) {
        printf("%s\n",token);
        token = strtok(NULL,delimiters);
    }

}

void removeNewLine(char *input) {

    char* pos;
    if ((pos=strchr(input, '\n')) != NULL) {
        *pos = '\0';
    }

}
