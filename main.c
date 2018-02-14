#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT_SIZE 512
#define DELIMITER_LENGTH 8
#define MAX_ARGUMENTS 50
char *originalPath;


void getInput(char *input);
void parse(char *input, char **arguments);
void executeCommand(char **arguments);
void execute(char **arguments);
void setDirectory();

//Internal commands
void exitShell();
void getPath();
void setPath(char **arguments);
void changeDirectory(char **arguments);

int main() {
    int exitStatus = 0;
    originalPath = getenv("PATH");
    printf("Initial PATH test: %s\n", originalPath);
    setDirectory();
    do {
        
        char input[MAX_INPUT_SIZE] = {'\0'};
        getInput(input);
        char *arguments[MAX_ARGUMENTS];
        parse(input, arguments);
        executeCommand(arguments);
 
    } while (1);

    return 0;
}

/*
 *  Gets input from the user
 */
void getInput(char *input) {

    printf("> ");
    if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
        exitShell(originalPath);
    }

}

/*
 *   Exits the shell
 *   Resets the PATH to the original
 */
void exitShell() {
    setenv("PATH", originalPath, 1);
    printf("Last PATH check whilst exiting: %s\n", getenv("PATH"));
    exit(0);
 }

/*
 * Tokenizes the input, storing each token into arguments
 * Places NULL after the last token, into arguments
 * Assumes arguments has space
 */
void parse(char *input, char **arguments) {
    
    const char delimiters[10] = " \t;<>|\n&";
    char* token;

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
 * Exeutes the command
 */
void executeCommand(char **arguments) {
    char *command = arguments[0];
    //Ensure we're not dereferencing a null pointer
    if(arguments[0] == NULL){
        return;
    } else if(strcmp("exit", command) == 0) {
        exitShell(originalPath);
    } else if(strcmp("getpath", command) == 0) {
        getPath();
    } else if(strcmp("setpath", command) == 0) {
        setPath(arguments);
    } else if(strcmp("cd", command) == 0) {
        changeDirectory(arguments);
    } else {
        //Non internal command 
        execute(arguments);
    }
}

/*
 *  Creates a child process, and executes the given command 
 */
void execute(char **arguments) {
    pid_t pid = fork();
    if (pid < 0) {
        //Error if pid < 0
        printf("Error forking\n");
    } else if (pid > 0) {
        //Parent process
        wait(NULL);
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

/*
 * Built-in command prints the value of PATH.
 */
void getPath() {
    printf("PATH: %s\n", getenv("PATH"));
}

/*
 * Built-in command sets the value of PATH.
 */
void setPath(char **arguments) {
    if (arguments[1] == NULL) {
        fprintf(stderr, "setpath requires an argument. PATH unchanged\n");
        return;
    }
    setenv("PATH", arguments[1], 1);
    printf("Current PATH: %s\n", getenv("PATH"));
}

/*
 * Set the cwd to HOME.
 */
void setDirectory() {
    char temp[MAX_INPUT_SIZE];
    chdir(getenv("HOME"));
    printf("Current working directory: %s/\n", getcwd(temp, sizeof(temp)));
}

/*
 * Change the cwd
 */
void changeDirectory(char **arguments) {
    char temp[MAX_INPUT_SIZE];

    if(arguments[1] == NULL || strcmp(" ", arguments[1]) == 0) {
        printf("HOME: %s\n", getenv("HOME"));
        chdir(getenv("HOME"));
    } else if(strcmp(".", arguments[1]) == 0) {
        //Current directory
        chdir(".");
    } else if(strcmp("..", arguments[1]) == 0) {
        //Parent directory
        chdir("..");
    } else {
        //Changing to certain directory
       if(chdir(arguments[1]) == -1) {
           printf("Error: %s\n", strerror(errno));
       }
    }
    printf("Current working directory: %s/\n", getcwd(temp, sizeof(temp)));
}
