all: main.c
	gcc -Wall alias.c main.c history.c internalCommands.c
