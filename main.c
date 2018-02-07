#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT_SIZE 512
#define DELIMITER_LENGTH 8
#define MAX_ARGUMENTS 50


void getInput(char* input);
void parse(char* input, char** arguments);
int executeCommand(char** arguments);
void execute(char** arguments);

int main() {
    int exitStatus = 0;
    do {
        char input[MAX_INPUT_SIZE] = {'\0'};
        getInput(input);
        char* arguments[MAX_ARGUMENTS];
        parse(input, arguments);
        exitStatus = executeCommand(arguments);

    } while (exitStatus != 1);
    return 0;
}

/* 
 *   Gets input from the user
 */
void getInput(char* input) {

    printf("> ");
    if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
        exit(0);
    }

}
/* 
 *  Tokenizes the input, storing each token into arguments
 *  Places NULL after the last token, into arguments
 *  Assumes arguments has space
 */
void parse(char* input, char** arguments) {
    
    const char delimiters[10] = " \t;<>|\n&";
    char*  token;

    token = strtok(input, delimiters);  
    int i = 0;
    while(token != NULL) {
        arguments[i] = token;
        i++;
        printf("(%s)\n", token);
        token = strtok(NULL, delimiters);        
    }
    arguments[i] = NULL;
	
}

/* 
 *  Excutes the command
 *  Returns 1 if exiting
 */
int executeCommand(char** arguments) {
    char* command = arguments[0];
    //EOF (ctrl-D) or exit
    if(arguments[0] == NULL){
       return 0;
    }
    else if(strcmp("exit", command) == 0) {
       return 1;
    } else {
        //Non internal command
        execute(arguments);
    }
    
    return 0;

}

/* 
 *   Creates a child process, and executes the given command 
 */
void execute(char** arguments) {
    pid_t pid = fork();
    if (pid < 0) {
        //Error if pid < 0
        printf("Error forking\n");
    } else if (pid > 0) {
        //Parent process
        int status;
        wait(&status);
    } else {
        //Child process
        //Use execvp so we can pass arguments, and it checks the PATH
        if (execvp(arguments[0], arguments) < 0) {
            printf("Executing process %s failed: %s\n", arguments[0], strerror(errno));
        }
        //Just incase
        exit(0);
    }

}
