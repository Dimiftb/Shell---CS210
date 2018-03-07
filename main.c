#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 512
#define DELIMITER_LENGTH 8
#define MAX_ARGUMENTS 50
#define MAX_HISTORY_COUNT 20
char *originalPath;

struct historyCommand {
    int commandNumber;
    char command[MAX_INPUT_SIZE];
} typedef historyCommand;

void getInput(char *input);
void parse(char *input, char **arguments);
void executeCommand(char **arguments, historyCommand* history);
void execute(char **arguments);
void executeHistoryCommand(char **arguments, historyCommand* history, int historyNumber);

//Internal commands
void exitShell();
void getPath(char **arguments);
void setPath(char **arguments);
void changeDirectory(char **arguments);

void saveCommand(char *input, historyCommand* history, int historyCount);
void printHistory(char **arguments, historyCommand* history);

int isStringNumber(char *string);

int main() {
    originalPath = getenv("PATH");
    printf("Initial PATH test: %s\n", originalPath);
    chdir(getenv("HOME"));
    int historyCount = 0;
    historyCommand history[MAX_HISTORY_COUNT] = {0};
    while(1) {
        char input[MAX_INPUT_SIZE] = {'\0'};
        char *arguments[MAX_ARGUMENTS];
        getInput(input);
        if (input[0] == '!') {
            //Handle history stuff

            parse(input, arguments);
            //Check if there's more than 1 arguments
            if (arguments[1] != NULL) {
                printf("Too many arguments for history invocation\n");
                continue;
            }
            //Repeat last command
            if (arguments[0][1] == '!') {
                if (historyCount == 0) {
                    printf("History is empty\n");
                    continue;
                }

                executeHistoryCommand(arguments, history, historyCount - 1);

            } else {
                //Repeating some previous command

                int numberStartIndex;
                //Check for negative sign so we can avoid '-' and '!' being considered for numbers
                if (arguments[0][1] == '-') {
                    numberStartIndex = 2;
                } else {
                    numberStartIndex = 1;
                }

                int isANumber = isStringNumber(arguments[0] + numberStartIndex);

                if (!isANumber) {
                    printf("Argument is not a number\n");
                    continue;
                }
                char numberString[MAX_INPUT_SIZE] = {'\0'};
                strcpy(numberString, arguments[0] + 1);

                int number;
                number = atoi(numberString);
                if (number == 0) {
                    printf("Invalid number for history\n");
                    continue;
                }
                if (abs(number) - 1 >= historyCount) {
                    printf("Not enough history\n");
                    continue;
                }
                if (number > 0) {
                    //Positive
                    executeHistoryCommand(arguments, history, number - 1);

                } else {
                    //Negative
                    executeHistoryCommand(arguments, history, historyCount + number);

                }
            }
        } else {
            //Not a history invocation
            parse(input, arguments);
            char temp[MAX_INPUT_SIZE] = {'\0'};

            //Save the command to history
            //Ensure we're not going to save an empty line
            if (arguments[0] != NULL) {
                int i = 0;
                //Join arguments together with spaces in between
                while (arguments[i] != NULL) {
                    strcat(temp, arguments[i]);
                    strcat(temp, " ");
                    i++; 
                }
                //Replace dangling space with a newline
                int len = strlen(temp);
                temp[len - 1] = '\n';
                saveCommand(temp, history, historyCount);
                historyCount = (historyCount + 1) % MAX_HISTORY_COUNT;
            }
            executeCommand(arguments, history);
        }
    }
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
void executeCommand(char **arguments, historyCommand* history) {
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
        //Print history
        printHistory(arguments, history);
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
 *  Saves the last non-history command into the historyCommand at historyCount
 */
void saveCommand(char *input, historyCommand *history, int historyCount) {
   history[historyCount].commandNumber = historyCount; 
   strcpy(history[historyCount].command, input);
}

/*
 *  Print entries in history (possibly up to the first null)
 */
void printHistory(char **arguments, historyCommand *history) {
    if (arguments[1] != NULL) {
        printf("Too many arguments for history\n");
        return;
    }
    for (int i = 0; i < MAX_HISTORY_COUNT; i++) {
        if (strcmp(history[i].command, "") == 0)
            break;
        printf("%d %s", history[i].commandNumber + 1, history[i].command);
   } 
}

/*
 * Iterates through a given string to see if the string is a number.
 * Returns 1 if string is a number, 0 otherwise. 
 */
int isStringNumber(char *string) {
    int i = 0;
    while (string[i] != '\0') {

        char ch = string[i];
        if (!isdigit(ch)) {
            return 0;
        }
        i++;
    }

    return 1;
}

/*
 * Executes a command stored in the history given by historyNumber
 */
void executeHistoryCommand(char **arguments, historyCommand* history, int historyNumber) {
    char temp[MAX_INPUT_SIZE];
    strcpy(temp, history[historyNumber].command);
    parse(temp, arguments);
    executeCommand(arguments, history);
}
