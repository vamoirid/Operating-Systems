/*
 *******************************************************************************
 *																			   *
 *						     Filename: myshell.c                               *
 *                                                                             *
 *						Author: Amoiridis Vasileios 8772                       *  
 *                                                                             *
 *                             Date: 08 Jan 2020                               *
 *******************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>			/* strlen(), strtok()  */
#include <sys/types.h>		/* fork(),getpid() system calls */
#include <sys/wait.h>       /* wait() system calls */
#include <unistd.h>			/* fork(),getpid() system calls */  
#include <errno.h>

/*
 *******************************************************************************
 * DEFINES
 *******************************************************************************
 */
#define BUFFER_SIZE 1024
#define MAX_LINE_SIZE 512
#define SPACE_DELIM " \t\r\n\a"
#define GREEN "\033[0;32m"
#define GREEN_BOLD "\033[1;32m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define RESET_COLOR "\033[0m"

/*
 *******************************************************************************
 * Functions' definitions
 *******************************************************************************
 */
void mainLoop(int argc, char *argv[]);
void printPromptName();
void quitShell();
FILE* chooseInput(int argc, char *argv[]);
char* readLine(FILE* input);
char** parseLine(char *line);
int executeCmd(char **args);

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
	int i = 0, execute_ret = 2;

	do
	{
		if(input == stdin)
		{
			printPromptName();
		}

		line = readLine(input);
		if(!strcmp(line,"\n")) continue; //if line is empty just jump to the
		//next line.

		args = parseLine(line);
		while(args[i] != NULL)
		{
			printf("The %d argument is: %s\n",i+1,args[i]);
			i++;
		}
		exit_status = executeCmd(args);
	} while(1);
	
}

/*
 *******************************************************************************
 * printPromptName() is a function which print the name of the prompt in the 
 * start of each line.
 *******************************************************************************
 */
void printPromptName()
{
	printf(GREEN_BOLD "amoiridis_8772> " RESET_COLOR);
}

/*
 *******************************************************************************
 * quitShell() is a function which is used to terminate the shell operation.
 *******************************************************************************
 */
void quitShell()
{
	printf("Thanks for the cooperation. It's been a pleasure!\n");
	exit(EXIT_SUCCESS);
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
		case 1: 			/* User typed: ./bin/myshell */
			input = stdin;
			break;
		case 2: 			/* User typed: ./bin/myshell 'batchfile_name' */
			input = fopen(argv[1], "r");
			if(input == NULL)
			{
				printf("ERROR: fopen() failure.\n");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			printf("Invalid input arguments.\n");
	}
	return input;
}

/*
 *******************************************************************************
 * readLine() function reads either 1 line from stdin or a line from a batch 
 * file and returns it. It checks for exceeding the maximum permitted line 
 * size and also if the end of file is reached. It also handles the memory
 * allocation of the line.
 *******************************************************************************
 */
char* readLine(FILE* input)
{
	char *line = malloc(BUFFER_SIZE * sizeof(char));

	if(line == NULL)
	{
		printf("ERROR: malloc() failure.\n");
		exit(EXIT_FAILURE);
	}
	if(fgets(line, BUFFER_SIZE, input) != NULL)
	{	
		if(strlen(line) > (MAX_LINE_SIZE + 1))
		{	
			printf("WARNING: Maximum permitted Line size exceeded.\n");
			printf("         Assume line as empty.\n");
			*line = '\0';
		}
	}
	else if (feof(input))
	{
		printf("EOF reached. Ciao!\n");
		exit(EXIT_SUCCESS);
	}
	else
	{
		printf("ERROR: fgets() failure.\n");
		exit(EXIT_FAILURE);
	}

	return line;
}

/*
 *******************************************************************************
 * parseLine() function reads an input line and "cuts" it into several pieces
 * of arguments in order to save the several arguments in the commands. It 
 * returns an array of strings which contains the seperate arguments.
 *******************************************************************************
 */
char** parseLine(char *line)
{
	char **tokens = malloc(BUFFER_SIZE * sizeof(char*));
	char *token = NULL;
	int token_num = 0;

	if(tokens == NULL)
	{
		fprintf(stderr, "ERROR: malloc() failure.\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, SPACE_DELIM);
	while(token != NULL)
	{
		tokens[token_num] = token;
		token_num++;
		token = strtok(NULL, SPACE_DELIM);
	}
	tokens[token_num] = NULL;

	return tokens;
}

/*
 *******************************************************************************
 * executeCmd() is a function which takes as an input the array of commands 
 * which generated from parseLine() function and executes the first command.
 * If the command was executed succesfully then the return value is 0. If 
 * the command didn't execute then the return value is 1.
 *******************************************************************************
 */
int executeCmd(char **args)
{
	pid_t pid, wait_pid;
	int status;

	if(!strcmp(args[0], "quit"))
	{
		quitShell();
	}

	pid = fork();
	if(pid == 0)
	{
		if(execvp(args[0], args) == -1)
		{
			perror("Command"); //fprintf(stderr, "%s\n", stderror(errno));
			//errno is the number of error occured and it is taken from the
			//library errno.h .If we combine this number with stderror(errno)
			//then the output is the string which describes this errror. Then
			//this string can be used inside fprintf() to be redirected from
			//stdout which is the standard output of perror() to stderr.
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		fprintf(stderr,"ERROR: fork() failure."); //perror("fork")
	}
	else
	{
		do
		{
			wait_pid = wait(&status); //on success returns
			//the pid of the child process which terminated. On failure it 
			//returns -1.
		} while(!WIFEXITED(status)); //When the macro
		//WIFEXITED(status) returns TRUE it means that the child terminated
		//normally. When the macro WIFSIGNALED(status) returns TRUE it means 
		//that the child process was terminated by a signal.
	}

	return WEXITSTATUS(status); 
}
