/*
 *******************************************************************************
 *																			   *
 *						     Filename: myshell.c                               *
 *                                                                             *
 *						Author: Amoiridis Vasileios 8772                       *  
 *                                                                             *
 *******************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *******************************************************************************
 * DEFINES
 *******************************************************************************
 */
#define BUFFER_SIZE 1024
#define MAX_LINE_SIZE 512

/*
 *******************************************************************************
 * Functions' definitions
 *******************************************************************************
 */
void mainLoop(int argc, char *argv[]);
FILE* chooseInput(int argc, char *argv[]);
void printPromptName();
void readLine(FILE* input, char **line);

/*
 *******************************************************************************
 * Main Code
 *******************************************************************************
 */
int main(int argc, char *argv[])
{
	mainLoop(argc, argv);

	return 0;
}

/*
 *******************************************************************************
 * mainLoop() is the code that runs on every execution until a "quit" command 
 * or until an invalid argument.
 *******************************************************************************
 */
void mainLoop(int argc, char *argv[])
{
	printf("Welcome to my Shell! My name is Vasileios Amoiridis and I am the creator.\n");
	FILE* input = chooseInput(argc, argv);
	char* line = NULL;
	char** args = NULL;
	int exit_status = 0;

	if(input == stdin)
	{
		printPromptName();
	}
	readLine(input, &line);

}

/*
 *******************************************************************************
 * chooseInput() is a function which checks the number of argc variable and  
 * decides if the shell will be in interactive or batch mode. Moreover it  
 * checks for empty batch files or inappropriate number of arguments.
 *******************************************************************************
 */

FILE* chooseInput(int argc, char* argv[])
{	
	FILE* input;

	switch (argc)
	{
		case 1:
			input = stdin;
			break;
		case 2:
			input = fopen(argv[1], "r");
			if(input == NULL)
			{
				printf("ERROR: fopen() Failure.\n");
				exit(1);
			}
			break;
		default:
			printf("Invalid input arguments.\n");
	}
	return input;
}

/*
 *******************************************************************************
 * printPromptName() is a function which print the name of the prompt in the 
 * start of each line.
 *******************************************************************************
 */
void printPromptName()
{
	printf("amoiridis_8772>\n");
}

/*
 *******************************************************************************
 * readLine() function reads either 1 line from stdin or multiple lines from a
 * batch file and saves them in the <char** line> variable.
 *******************************************************************************
 */
void readLine(FILE* input, char **line)
{
	*line = malloc(BUFFER_SIZE * sizeof(char));
	if(*line == NULL)
	{
		printf("ERROR: malloc() Failure.\n");
		exit(1);
	}

	if(fgets(*line, MAX_LINE_SIZE, input) != NULL)
	{	
		if(strlen(*line) > (MAX_LINE_SIZE + 1))
		{
			printf("WARNING: Maximum permitted Line size exceeded.\n");
			printf("         Assume line as empty.\n");
			**line = '\0';
		}

	}
	else //EOF
	{
		printf("EOF reached. Ciao!");
		exit(0);
	}
}