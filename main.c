#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT_SIZE 512
#define DELIMITER_LENGTH 8
#define MAX_ARGUMENTS 50


void getInput(char *input, char *pathog);
void parse(char *input, char **arguments);
int executeCommand(char **arguments);
void execute(char **arguments);
void getPath();
void setPath(char **arguments);
void setDirectory();
void changeDirectory(char **arguments);

int main() {
    int exitStatus = 0;
    char *pathog = getenv("PATH");
    printf("Initial PATH test: %s\n", pathog);
    setDirectory();
    do {
        
        char input[MAX_INPUT_SIZE] = {'\0'};
        getInput(input, pathog);
        char *arguments[MAX_ARGUMENTS];
        parse(input, arguments);
        exitStatus = executeCommand(arguments);
       
        if(exitStatus == 1) {
            
        }
        
    } while (exitStatus != 1);
    setenv("PATH", pathog, 1);
       printf("Last 'exit' PATH check: %s\n", getenv("PATH"));
    return 0;
}

/*
 *  Gets input from the user
 */
void getInput(char *input, char *pathog) {

    printf("> ");
    if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
       setenv("PATH", pathog, 1);
       printf("Last EOF PATH check: %s\n", getenv("PATH"));
        exit(0);
    }

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
 * Excutes the command
 * Returns 1 if exiting
 */
int executeCommand(char **arguments) {
    char *command = arguments[0];
    //EOF (ctrl-D) 
    if(arguments[0] == NULL){
       return 0;
    }
    //exit
    else if(strcmp("exit", command) == 0) {
        return 1;
    } 
    else if(strcmp("getpath", command) == 0) {
        getPath();
        return 0;
    }
    else if(strcmp("setpath", command) == 0) {
        setPath(arguments);
        return 0;
    }
    else if(strcmp("cd", command) == 0) {
        changeDirectory(arguments);
        return 0;
    }
    else {
        //Non internal command 
        execute(arguments);
    }
    return 0;
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

/*
 * Built-in command prints the value of PATH.
 */
void getPath() {
     
    printf("PATH: %s\n" ,getenv("PATH"));
     
}

/*
 * Built-in command sets the value of PATH.
 */
void setPath(char **arguments) {
    char *argument1 = arguments[1];
    setenv("PATH", argument1, 1);
    printf("Current PATH: %s\n", getenv("PATH"));
}

/*
 * Set the cwd to HOME.
 */
void setDirectory() {
    long size = 31;
    char temp[128];
    chdir(getenv("HOME"));
    printf("Current working directory: %s/\n", getcwd(temp, (size_t)size));
}
/*
 * Change the cwd
 */
void changeDirectory(char **arguments) {
     long size = 31;
    char temp[128];
    char *argument1 = arguments[1];
    if(strcmp(" ", argument1) == 0) {
        printf("HOME: %s\n", getenv("HOME"));
        chdir(getenv("HOME"));
        printf("\n\n");
        printf("Current working directory: %s/\n", getcwd(temp, (size_t)size));
    }
    else if(strcmp(".", argument1) == 0) {
        chdir(".");
        printf("Current working directory: %s/\n", getcwd(temp, (size_t)size));
    }
    else if(strcmp("..", argument1) == 0) {
        chdir("..");
        printf("Current working directory: %s/\n", getcwd(temp, (size_t)size));
    }
    
    else {
       if(chdir(argument1) == -1){
           printf("Error: %s", strerror(errno));
       }
    }
    
}