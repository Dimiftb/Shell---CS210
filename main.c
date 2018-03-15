#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 512
#define MAX_ARGUMENTS 50
#define MAX_ALIASES 10
#define MAX_HISTORY_COUNT 20
#define DELIMITER_LENGTH 8
#define HISTORY_FILE_NAME "/.hist_list"
#define ALIASES_FILE_NAME "/.aliases"

struct alias {
    char aliasName[MAX_INPUT_SIZE];
    char command[MAX_INPUT_SIZE];
} typedef alias;

struct historyCommand {
    int commandNumber;
    char command[MAX_INPUT_SIZE];
} typedef historyCommand;

void getInput(char *input, historyCommand *history);
void parse(char *input, char **arguments);
void executeCommand(char **arguments, historyCommand *history);
void execute(char **arguments);
void executeHistoryCommand(char **arguments, historyCommand *history, int historyNumber);
void repeatLastCommand(char **arguments, historyCommand *history, int historyCount);
void repeatPastCommand(char **arguments, historyCommand *history, int historyCount);

//Internal commands
void exitShell(historyCommand *history);
void getPath(char **arguments);
void setPath(char **arguments);
void changeDirectory(char **arguments);
void printHistory(char **arguments, historyCommand *history);

void saveCommand(char *input, historyCommand *history, int historyCount);

void readHistoryFile(historyCommand *history, int *historyCount);
void saveHistoryToFile(historyCommand *history);
char *getHistoryFilename();

int isStringNumber(char *string);
void joinArguments(char **arguments, char *string);

void printAliases();
void addAlias(char **arguments);
void removeAlias(char **arguments);
void replaceAlias(char *input);

void readAliasesFile();
void saveAliasesFile();
char *getAliasesFilename();

bool isAliasesEmpty(); 

alias aliases[MAX_ALIASES] = {{{0}}, {{0}}};
char *originalPath;

int main() {
    originalPath = getenv("PATH");
    //printf("Initial PATH test: %s\n", originalPath);
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
 *   Exits the shell
 *   Saves command history to history file
 *   Resets the PATH to the original
 */
void exitShell(historyCommand *history) {
    saveAliasesFile();
    saveHistoryToFile(history);
    setenv("PATH", originalPath, 1);
    //printf("Last PATH check whilst exiting: %s\n", getenv("PATH"));
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
    char *firstArgument = arguments[1];
    //Check for too many arguments
    if (arguments[2] != NULL) {
        printf("Too many arguments for cd\n");
        return;
    }
    if (firstArgument == NULL) {
        //Change to home
        chdir(getenv("HOME"));
    } else {
        if (strcmp(".", firstArgument) == 0) {
            chdir(".");
        } else if(strcmp("..", firstArgument) == 0) {
            chdir("..");
        } else { 
            if(chdir(firstArgument) == -1) {
		perror(firstArgument);
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

    if (historyCount >= MAX_HISTORY_COUNT) {
        //SHIFTUP 
        for (int i = 0; i < MAX_HISTORY_COUNT - 1; i++) {
            strcpy(history[i].command, history[i + 1].command);
        }
        strcpy(history[MAX_HISTORY_COUNT - 1].command, input);
    } else {
        history[historyCount].commandNumber = historyCount; 
        strcpy(history[historyCount].command, input);
    }
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
    replaceAlias(temp);
    parse(temp, arguments);
    executeCommand(arguments, history);
}

/*
 *  Reads the history file and loads that into the given historyCommand struct
 *  and sets historyCount to the number of lines read
 */
void readHistoryFile(historyCommand *history, int *historyCount) {
    FILE *file;
    char *filename = getHistoryFilename();
    printf("Filename: %s\n", filename);
    file = fopen(filename, "r");
     
    if (file == NULL) {
        perror("Error opening history file: ");
        return;
    }
    int i = 0;
    char line[MAX_INPUT_SIZE] = {'\0'};
    while (fgets(line, MAX_INPUT_SIZE, file)) {
        if (*historyCount >= MAX_HISTORY_COUNT) {
            *historyCount = MAX_HISTORY_COUNT;
            //Stop program if there's an error maybe?
            printf("Too many history lines in history file\n");
            return;
        }
        char command[MAX_INPUT_SIZE] = {'\0'};   
        int commandNumber;

        //Gets the whole string up until the new line
        //Seems to be one of the only ways to get the whole line after the digits
        int result;
        result = sscanf(line, "%d %[^\n]", &commandNumber, command);
        if (result < 2) {
            printf("Error at line %d in history file.\n", i + 1);
            printf("Saving history up until error\n");
            exitShell(history);
        }

        //Add newline at end of line
        int len = strlen(command);
        command[len] = '\n';

        //Store the command in history
        *historyCount = *historyCount + 1;
        
        //+1 to bring number to human counting
        if (i == 0) {
            if (commandNumber != 1) {
                printf("History number out of order: %d line %d\n", commandNumber, i + 1);
                printf("Saving history up until error\n");
                exitShell(history);
            }
        } else {
            if (commandNumber - 1 != history[i - 1].commandNumber + 1) {
                printf("History number out of order: %d line %d\n", commandNumber, i + 1);
                printf("Saving history up until error\n");
                exitShell(history);
            }
        }
        history[i].commandNumber = commandNumber - 1;
        strcpy(history[i].command, command);
        i++;
    }

    free(filename);
    fclose(file);

}

/*
 * Saves the current history stored in historyCommand to the
 *  history file
 */
void saveHistoryToFile(historyCommand *history) {
    FILE *file;
    char *filename = getHistoryFilename();
    printf("Filename: %s\n", filename);
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening history file: ");
        return;
    }
    for (int i = 0; i < MAX_HISTORY_COUNT; i++) {
        //Check if history entry is empty
        if (history[i].command[0] == '\0') {
            break;
        }

        fprintf(file, "%d %s", history[i].commandNumber + 1, history[i].command);        
    }
    free(filename);
    fclose(file);
}

/*
 *  Creates the filename for the history file and returns a pointer to it
 *  Caller must free the pointer
 */
char *getHistoryFilename() {
    char *filename;
    //filename = malloc(MAX_INPUT_SIZE);
    filename = calloc(MAX_INPUT_SIZE, 1);
    strcat(filename, getenv("HOME"));
    strcat(filename, HISTORY_FILE_NAME);
    return filename;
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

/*
 *  Adds the given alias to the alias struct
 */
void addAlias(char **arguments) {
    char *aliasName = arguments[1];
    char *aliasCommand = arguments[2];

    if (aliasCommand == NULL) {
        printf("Not enough arguments for alias\n");
        return;
    }
    char wholeCommand[MAX_INPUT_SIZE] = {'\0'};
    //Combine all command arguments into one string
    //Start at the argument that gives the command
    int i = 2;
    while (arguments[i] != NULL) {
        strcat(wholeCommand, arguments[i]);
        strcat(wholeCommand, " ");
        i++;
    }
    //Replace trailing whitespace with newline
    int len = strlen(wholeCommand);
    wholeCommand[len - 1] = '\0';

    //Find the first empty position
    for (int i = 0; i < MAX_ALIASES; i++) {
        //Check for duplicate alias
        if (strcmp(aliases[i].aliasName, aliasName) == 0) {
            printf("Overwriting alias %s\n", aliasName);
            strcpy(aliases[i].aliasName, aliasName);
            strcpy(aliases[i].command, wholeCommand);
            return;
        }
        if (strcmp(aliases[i].aliasName, "") == 0) {
            strcpy(aliases[i].aliasName, aliasName);
            strcpy(aliases[i].command, wholeCommand);
            printf("Aliased %s to %s\n", aliasName, wholeCommand);
            return;
        }
    }
    printf("No more space for aliases\n");

}

/*
 * Removes the given alias from the aliases struct
 */
void removeAlias(char **arguments) {
    char *aliasName = arguments[1];

    if (aliasName == NULL) {
        printf("Not enough arguments for unalias\n");
        return;
    }

    if (arguments[2] != NULL) {
        printf("Too many arguments for unalias\n");
        return;
    }

    for (int i = 0; i < MAX_ALIASES; i++) {
         if (strcmp(aliases[i].aliasName, aliasName) == 0) {
            strcpy(aliases[i].aliasName, "");
            strcpy(aliases[i].command, "");
            printf("Removed %s from aliases\n", aliasName); 
            return;
         }
    }
}

/*
 *  Prints the aliases by Name: Command:
 */
void printAliases() {
    if (isAliasesEmpty()) {
        printf("No aliases to display\n");
        return;
    }
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (strcmp(aliases[i].aliasName, "") != 0) {
            printf("Name: %s Command: %s\n", aliases[i].aliasName, aliases[i].command);
        }
    }
}

/*
 *  Checks if the aliases are empty.
 *  Returns true if there are no aliases
 */
bool isAliasesEmpty() {
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (strcmp(aliases[i].aliasName, "") != 0) {
            //Not empty
            return false;
        }
    }
    return true;
}

/*
 *  Checks for every command whether it's an alias
 */     
void replaceAlias(char *input) {
    const char delimiters[10] = " \t;<>|\n&";
    char* token;
    char line[MAX_INPUT_SIZE] = { '\0' };
    char originalLine[MAX_INPUT_SIZE] = { '\0' };

    strcpy(originalLine, input);
    //get command
    token = strtok(originalLine, delimiters);
    //look for an alias and get the alias command if one is found
    for(int j = 0; j < MAX_ALIASES; j++) {
        if(token != NULL && aliases[j].aliasName != NULL && (strcmp(token, aliases[j].aliasName) == 0)) {
            token = aliases[j].command;
        }
    }
    // start building the actual line that must be executed
    if (token != NULL) {
        strcpy(line, token);
    }
    //get rest of original line after the possible alias
    token = strtok(NULL, "");
    if(token != NULL) {
        strcat(line, " ");
        strcat(line, token);
    }

    strcpy(input, line);
}

/*
 *  Creates the filename for the aliases file and returns a pointer to it
 *  Caller must free the pointer
 */
char *getAliasesFilename() {
    char *filename;
    filename = calloc(MAX_INPUT_SIZE, 1);
    strcat(filename, getenv("HOME"));
    strcat(filename, ALIASES_FILE_NAME);
    return filename;
}

/*
 *  Saves all aliases in the aliases struct to the file at filename
 */
void saveAliasesFile() {
    FILE *file;
    char *filename = getAliasesFilename();
    printf("Filename: %s\n", filename);
    file = fopen(filename, "w");

    if (file == NULL) {
        perror("Error opening alias file: ");
        return;
    }

    for (int i = 0; i < MAX_ALIASES; i++) {
        //Check if aliases entry is empty
        if (strcmp(aliases[i].aliasName, "") == 0) {
            break;
        }

        fprintf(file, "%s %s\n", aliases[i].aliasName, aliases[i].command);        
    }

    free(filename);
    fclose(file);
}

void readAliasesFile() {
    FILE *file;
    char *filename = getAliasesFilename();
    printf("Filename: %s\n", filename);
    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening aliases file: ");
        return;
    }

    int i = 0;
    char line[MAX_INPUT_SIZE] = {'\0'};
    while (fgets(line, MAX_INPUT_SIZE, file)) {
        if (i >= MAX_ALIASES) {
            printf("Too many aliases in the aliases file. No longer reading\n");
            break;
        }

        char aliasName[MAX_INPUT_SIZE];
        char command[MAX_INPUT_SIZE];
        //Gets the whole string up until the new line
        //Seems to be one of the only ways to get the whole line after the digits
        int result;
        result = sscanf(line, "%s %[^\n]", aliasName, command);
        if (result < 2) {
            printf("Error at line %d in alias file.\n", i + 1);
        }

        strcpy(aliases[i].aliasName, aliasName);
        strcpy(aliases[i].command, command);
        i++;
    }

    free(filename);
    fclose(file);
}
