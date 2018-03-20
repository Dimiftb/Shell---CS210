#pragma once
#include "common.h"

#define ALIASES_FILE_NAME "/.aliases"
#define MAX_ALIASES 10

struct alias {
    char aliasName[MAX_INPUT_SIZE];
    char command[MAX_INPUT_SIZE];
} typedef alias;

void printAliases();
void addAlias(char **arguments);
void removeAlias(char **arguments);
void replaceAlias(char *input);

void readAliasesFile();
void saveAliasesFile();
char *getAliasesFilename();

bool isAliasesEmpty(); 

extern alias aliases[MAX_ALIASES];
