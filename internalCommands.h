#pragma once
#include "common.h"
#include "history.h"
#include "alias.h"

extern char *originalPath;

void exitShell(historyCommand *history);
void getPath(char **arguments);
void setPath(char **arguments);
void changeDirectory(char **arguments);
void printHistory(char **arguments, historyCommand *history);
