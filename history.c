#include "history.h"


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
 *  Reads the history file and loads that into the given historyCommand struct
 *  and sets historyCount to the number of lines read
 */
void readHistoryFile(historyCommand *history, int *historyCount) {
    FILE *file;
    char *filename = getHistoryFilename();
    printf("Filename: %s\n", filename);
    file = fopen(filename, "r");
     
    if (file == NULL) {
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
            break;
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
                break;
            }
        } else {
            if (commandNumber - 1 != history[i - 1].commandNumber + 1) {
                printf("History number out of order: %d line %d\n", commandNumber, i + 1);
                printf("Saving history up until error\n");
                break;
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
