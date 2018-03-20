#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#include "common.h"
#include "alias.h"
#include "history.h"
#include "internalCommands.h"

#define MAX_ARGUMENTS 50

void getInput(char *input, historyCommand *history);
void parse(char *input, char **arguments);
void executeCommand(char **arguments, historyCommand *history);
void execute(char **arguments);
void executeHistoryCommand(char **arguments, historyCommand *history, int historyNumber);
void repeatLastCommand(char **arguments, historyCommand *history, int historyCount);
void repeatPastCommand(char **arguments, historyCommand *history, int historyCount);

//Internal commands

int isStringNumber(char *string);
void joinArguments(char **arguments, char *string);

alias aliases[MAX_ALIASES] = {{{0}}, {{0}}};
char *originalPath;

int main() {
    originalPath = getenv("PATH");
    printf("Initial PATH: %s\n", originalPath);
    chdir(getenv("HOME"));
    int historyCount = 0;
    historyCommand history[MAX_HISTORY_COUNT] = {{0}};
    readHistoryFile(history, &historyCount);
    readAliasesFile();

    while(true) {
        //Get input
        //Parse that input
        char input[MAX_INPUT_SIZE] = {'\0'};
        char *arguments[MAX_ARGUMENTS];
        getInput(input, history);

        char unAliasedInput[MAX_INPUT_SIZE];
        strcpy(unAliasedInput, input);

        replaceAlias(input);
        parse (input, arguments);
        if (arguments[0] == NULL) {
            continue;
        }
        if (arguments[0][0] == '!') {
            //Handle history

            //Check if there's more than 1 arguments
            if (arguments[1] != NULL) {
                printf("Too many arguments for history invocation\n");
                continue;
            }
            //Repeat last command
            if (arguments[0][1] == '!') {
                repeatLastCommand(arguments, history, historyCount);
            } else {
                //Repeating some previous command
                repeatPastCommand(arguments, history, historyCount);
            }
        } else {
            //Else, not a history invocation

            //Save the command to history
            //Ensure we're not going to save an empty line
            if (arguments[0] != NULL) {
                char joinedArguments[MAX_INPUT_SIZE] = {'\0'};
                char *tempArguments[MAX_ARGUMENTS];
                parse(unAliasedInput, tempArguments);
                joinArguments(tempArguments, joinedArguments);

                saveCommand(joinedArguments, history, historyCount);
                historyCount++;
            }
            executeCommand(arguments, history);
        }
    }
    return 0;
}

/*
 *  Gets input from the user
 */
void getInput(char *input, historyCommand *history) {
    printf("> ");
    //Checking if CTRL+D is pressed and handle exitShell
    if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
        exitShell(history);
    }
}

/*
 * Tokenizes the input, storing each token into arguments
 * Places NULL after the last token, into arguments
 * Assumes arguments has space
 */
void parse(char *input, char **arguments) {

    const char delimiters[] = " \t;<>|\n&";
    char* token;
    //replace alias words with their command
    //replace "this" with "ls -l"
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
        exitShell(history);
    } else if(strcmp("getpath", command) == 0) {
        getPath(arguments);
    } else if(strcmp("setpath", command) == 0) {
        setPath(arguments);
    } else if(strcmp("cd", command) == 0) {
        changeDirectory(arguments);
    } else if(strcmp("history", command) == 0) {
        printHistory(arguments, history);
    } else if(strcmp("alias", command) == 0) {
        //Check if there is not another argument if there isn't then input was 'alias' so printAliases
        if (arguments[1] == NULL) {
            printAliases();
        } else {
            addAlias(arguments);
        }
    } else if(strcmp("unalias", command) == 0) {
        removeAlias(arguments);
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
    replaceAlias(temp);
    parse(temp, arguments);
    executeCommand(arguments, history);
}

/*
 *  Repeats the last command from the user's history
 */
void repeatLastCommand(char **arguments, historyCommand *history, int historyCount) {
    if (historyCount == 0) {
        printf("History is empty\n");
        return;
    }
    if (historyCount > MAX_HISTORY_COUNT - 1) {
        executeHistoryCommand(arguments, history, MAX_HISTORY_COUNT - 1);
    } else {
        executeHistoryCommand(arguments, history, historyCount - 1);
    }

}

/*
 *  Repeats a command from the user's history
 */
void repeatPastCommand(char **arguments, historyCommand *history, int historyCount) {
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
        return;
    }
    char numberString[MAX_INPUT_SIZE] = {'\0'};
    //+1 to get characters after the initial '!'
    strcpy(numberString, arguments[0] + 1);

    int number;
    number = atoi(numberString);

    if (number == 0) {
        printf("Invalid number for history\n");
        return;
    }
    
    //Make sure entered number is not greater than current history
    if ((abs(number)) > MAX_HISTORY_COUNT || abs(number) > historyCount ) {
        printf("Not enough history\n");
        return;
    }
    if (number > 0) {
        //Positive
        executeHistoryCommand(arguments, history, number - 1);
    } else {
        //Negative
        if (historyCount > MAX_HISTORY_COUNT - 1) {
            executeHistoryCommand(arguments, history, MAX_HISTORY_COUNT + number);
        } else {
            executeHistoryCommand(arguments, history, historyCount + number);
        }
    }
}

/*
 * Concatenates arguments into a given string.
 * Assumes string has enough space for all arguments and is null
 */
void joinArguments(char **arguments, char *string) {
    int i = 0;
    //Join arguments together with spaces in between
    while (arguments[i] != NULL) {
        strcat(string, arguments[i]);
        strcat(string, " ");
        i++; 
    }
    //Replace dangling space with a newline
    int len = strlen(string);
    string[len - 1] = '\n';
}
