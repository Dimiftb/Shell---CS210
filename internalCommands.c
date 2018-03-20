#include "internalCommands.h"

/*
 *   Exits the shell
 *   Saves command history to history file
 *   Resets the PATH to the original
 */
void exitShell(historyCommand *history) {
    saveAliasesFile();
    saveHistoryToFile(history);
    setenv("PATH", originalPath, 1);
    printf("Last PATH check whilst exiting: %s\n", getenv("PATH"));
    exit(0);
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
        fprintf(stderr, "setpath requires an argument: provide a path\n");
        return;
    }
    if (arguments[2] != NULL) {
        printf("Too many arguments for setpath: provide only one path\n");
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
        printf("Too many arguments for cd: provide only one directory\n");
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
