all: main.c
	gcc -Wall common.h alias.h alias.c main.c history.h history.c internalCommands.h internalCommands.c
