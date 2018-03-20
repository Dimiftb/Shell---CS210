#include "alias.h"

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



    //Look for duplicate aliases
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (strcmp(aliases[i].aliasName, aliasName) == 0) {
            printf("Overwriting alias %s\n", aliasName);
            strcpy(aliases[i].aliasName, aliasName);
            strcpy(aliases[i].command, wholeCommand);
            return;
        }
    }

    //Find the first empty position
    for (int i = 0; i < MAX_ALIASES; i++) {
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
    printf("Alias not found %s\n", arguments[1]);
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
 *  Checks for every command whether it's an alias, replacing it if it is
 */     
void replaceAlias(char *input) {
    const char delimiters[] = " \t;<>|\n&";
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
        return;
    }

    for (int i = 0; i < MAX_ALIASES; i++) {
        //Check if aliases entry is empty
        if (strcmp(aliases[i].aliasName, "") == 0) {
            continue;
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
            //printf("Error at line %d in alias file. Ignoring this line\n", i + 1);
            i++;
            continue;
        }

        strcpy(aliases[i].aliasName, aliasName);
        strcpy(aliases[i].command, command);
        i++;
    }

    free(filename);
    fclose(file);
}
