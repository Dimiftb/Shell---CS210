#pragma once
#include "common.h"

#define MAX_HISTORY_COUNT 20
#define HISTORY_FILE_NAME "/.hist_list"

struct historyCommand {
    int commandNumber;
    char command[MAX_INPUT_SIZE];
} typedef historyCommand;

void saveCommand(char *input, historyCommand *history, int historyCount);

void readHistoryFile(historyCommand *history, int *historyCount);
void saveHistoryToFile(historyCommand *history);
char *getHistoryFilename();
