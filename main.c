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

struct historyCommand {
    int commandNumber;
    char command[MAX_INPUT_SIZE];
} typedef historyCommand;

void getInput(char *input);
void parse(char *input, char **arguments);
void executeCommand(char **arguments, int historyCount, historyCommand commands[]);
void execute(char **arguments);


//Internal commands
void exitShell();
void getPath(char **arguments);
void setPath(char **arguments);
void changeDirectory(char **arguments);

void saveCommand(char *input, int historyCount, historyCommand commands[]);
void showHistory(int historyCount, historyCommand commands[]);
void getHistory(char *input, int historyCount, historyCommand commands[]);

int main() {
    originalPath = getenv("PATH");
    printf("Initial PATH test: %s\n", originalPath);
    chdir(getenv("HOME"));
    //Add to, based on text file lines
    int historyCount = 21;
    historyCommand commands [20] = {0};
    
    do {
        char input[MAX_INPUT_SIZE] = {'\0'};
        getInput(input);
        if (input[0] != '!') {
            saveCommand(input, historyCount, commands);
            char *arguments[MAX_ARGUMENTS];
            parse(input, arguments);
            executeCommand(arguments, historyCount, commands);
            historyCount++;   
        } else {
            //printf("Call history");            
            getHistory(input, historyCount, commands);
        }
 
    } while (1);

    return 0;
}

/*
 *  Gets input from the user
 */
void getInput(char *input) {

    printf("> ");
    if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
        exitShell();
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
        //printf("(%s)\n", token);
        token = strtok(NULL, delimiters);        
    }
    arguments[i] = NULL;
	
}

/*
 * Exeutes the command
 */
void executeCommand(char **arguments, int historyCount, historyCommand *commands) {
    char *command = arguments[0];
    //Ensure we're not dereferencing a null pointer
    if(arguments[0] == NULL){
        return;
    } else if(strcmp("exit", command) == 0) {
        if (arguments[1] != NULL) {
            printf("Too many arguments for exit\n");
            return;
        }
        exitShell();
    } else if(strcmp("getpath", command) == 0) {
        getPath(arguments);
    } else if(strcmp("setpath", command) == 0) {
        setPath(arguments);
    } else if(strcmp("cd", command) == 0) {
        changeDirectory(arguments);
    } else if(strcmp("history", command) == 0) {
        showHistory(historyCount, commands);
    }else {
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
	    perror(arguments[0]);
        }
        //Just incase
        exit(0);
    }
}

/*
 * Built-in command prints the value of PATH.
 */
void getPath(char **arguments) {
    if (arguments[1] != NULL) {
            printf("Too many arguments for getPath\n"); 
            return;
        }
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
    if (arguments[2] != NULL) {
        printf("Too many arguments for setpath\n");
        return;
    }
    if (strcmp(arguments[1], "HOME") == 0) {
        chdir(getenv("HOME"));  
	char *cwd = getcwd(NULL, 0);
        printf("Current working directory: %s\n", cwd);
        free(cwd);
    } else {
        setenv("PATH", arguments[1], 1);
        printf("Current PATH: %s\n", getenv("PATH"));
    }
}


/*
 * Saves commands to array (20)
 */
void saveCommand(char *input, int historyCount, historyCommand *commands){

    commands[21].commandNumber = historyCount;
    strcpy(commands[21].command, input);
}

/*
 * Changes the working directory
 */
void changeDirectory(char **arguments) {
    //Check for too many arguments
    if (arguments[2] != NULL) {
        printf("Too many arguments for cd\n");
        return;
    }
    if (arguments[1] == NULL) {
        //Change to home
        chdir(getenv("HOME"));
    } else {
        if (strcmp(".", arguments[1]) == 0) {
            chdir(".");
        } else if(strcmp("..", arguments[1]) == 0) {
            chdir("..");
        } else { 
            if(chdir(arguments[1]) == -1) {
		perror(arguments[1]);
            }
          }
      }
    //getcwd mallocs the size for us
    char *cwd = getcwd(NULL, 0);
    printf("Current working directory: %s\n", cwd);
    free(cwd);
}


/*
 * Saves commands to array (20)
 */
void saveCommand(char *input, int historyCount, historyCommand *commands){

    commands[historyCount].commandNumber = historyCount;
    strcpy(commands[historyCount].command, input);

}

/*
 * Shows entire history, 20 commands
 */
void showHistory(int historyCount, historyCommand *commands){

    for (int i = 0; i < historyCount; i++){
        printf("count: %d\n", commands[i].commandNumber+1);
        printf("command: %s\n", commands[i].command);
    }
     
}

/*
 * Do user's chosen history command, based on input
 */
void getHistory(char *input, int historyCount, historyCommand *commands){
    
        printf("input is: %c\n", input[1]);
        
    //!!
    // if ('!' == input[1]){
    //     //wants a pointer to a pointer of char (pointer to an array). Giving a pointer to char right now

        // char *arguments[MAX_ARGUMENTS];
        // parse(commands[historyCount].command, arguments);
        // executeCommand(arguments, historyCount, commands);
    // }
    

}
