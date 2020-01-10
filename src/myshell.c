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
void   mainLoop        (int argc, const char *argv[]);
void   printPromptName (void);
void   quitShell       (void);
FILE*  chooseInput     (int argc, const char *argv[]);
char*  readLine        (FILE* input);
char** parseLine       (char *line);
int    checkArgs       (char **args);
int    parseArgs       (char **args, char **cmd_args);
int    executeCmd      (char **args);
void   executeAll      (char **args);
void executeRecursively(char **args, char **cmd_args);

 int printed = 1;

/*
 *******************************************************************************
 * Main Code
 *******************************************************************************
 */
int main(int argc, const char *argv[])
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
void mainLoop(int argc, const char *argv[])
{
	printf("Welcome to my Shell! My name is Vasileios Amoiridis and I am the creator.\n");
	FILE* input = chooseInput(argc, argv);
	char* line = NULL;
	char** args = NULL;
	int exit_status = 0, check_status = 0;
	int i = 0;

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
		if (checkArgs(args)) continue; //if there is a false argument jump to
		//next line.

		executeAll(args);

		free(line);
		free(args);
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

FILE* chooseInput(int argc, const char* argv[])
{	
	FILE* input;

	switch (argc)
	{
		case 1: 			/* User typed: ./bin/myshell */
			input = stdin;
			break;
		case 2: 			/* User typed: ./bin/myshell "batchfile_name" */
			input = fopen(argv[1], "r");
			if(input == NULL)
			{
				printf("ERROR: fopen() failure.\n");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr,RED "Invalid input arguments.\n" RESET_COLOR);
			exit(EXIT_FAILURE);
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
	char *line = (char*)malloc(BUFFER_SIZE * sizeof(char));

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
	char **tokens = (char**)malloc(BUFFER_SIZE * sizeof(char*));
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
 * checkArgs() is a function which checks for possible syntax errors in the
 * arguments provided by the user.
 *******************************************************************************
 */
int checkArgs(char **args)
{
	int check_status = 0, i = 0;
	if(!strcmp(args[0], "&") || !strcmp(args[0], ";") ||
	   !strcmp(args[0], "<") || !strcmp(args[0], ">") || 
	   !strcmp(args[0], "|") || !strcmp(args[0], "&&"))
	{
		printf("ERROR: Bad syntax. Unexpected first character.\n");
		check_status = 1;
	}
	while(args[i] != NULL)
	{	
		if(strstr(args[i], ";;") != NULL)
		{	
			printf("ERROR: Bad syntax. Two or more ';' found sequentially\n");
			check_status = 1;
			break;
		}
		else if(strstr(args[i], "&&&") != NULL)
		{
			printf("ERROR: Bad syntax. Three or more '&' found sequentially\n");
			check_status = 1;
			break;
		}
		else if (i > 0)
		{
			if(!strcmp(args[i], args[i-1]) && (!strcmp(args[i], ";") || 
			   !strcmp(args[i], "&&")))
			{
				printf("ERROR: Bad syntax. Unexpected token after ; or &&.\n");
				check_status = 1;
				break;				
			}
		}
		i++;
	}
	if(!strcmp(args[i-1], ";") || !strcmp(args[i-1], "&") ||
	   !strcmp(args[i-1], "<") || !strcmp(args[i-1], ">") ||
	   !strcmp(args[i-1], "|") || !strcmp(args[i-1], "&&"))
	{
		printf("ERROR: Bad syntax. Unexpected last token.\n");
		check_status = 1;
	}

	return check_status;
}

/*
 *******************************************************************************
 * parseArgs()
 *******************************************************************************
 */
int parseArgs(char **args, char **cmd_args)
{
	char **temp = (char**)malloc(BUFFER_SIZE * sizeof(char*));
	int i = 0, j = 0, k = 0, m = 0, cnt = 0, execute_status = 0;

	while(cmd_args[cnt] != NULL)
	{
		cmd_args[cnt] = NULL;
		cnt++;
	}

	while(args[i] != NULL)
	{
		if(!(strcmp(args[i], ";")))
		{
			execute_status = 1;
			break;
		}
		else if(!(strcmp(args[i], "&&")))
		{
			execute_status = 2;
			break;
		}
		else if(!(strcmp(args[i], "<")))
		{
			execute_status = 3;
			break;
		}
		else if(!(strcmp(args[i], ">")))
		{
			execute_status = 4;
			break;
		}
		else if(!(strcmp(args[i], "|")))
		{
			execute_status = 5;
			break;
		}
		else
		{
			cmd_args[i] = args[i]; //Store arguments until to find a delimiter
			//When we find a delimiter, it means that we just parsed a command
			//and so we have saved it in **cmd_args.
		}
		i++;
	}
	i++;   //bypass the delimiter in order to find the start of the next 
		   //command.

	while(args[i] != NULL) // args[i] is the value next to the delimiter.
    //for example if we typed the command 'pwd ; ls -l' then the value is
	//args[i] = "ls". We store the remainder of the line without the first
	//command which is in **cmd_args.
	{
		temp[j] = args[i];
		i++;	
		j++;
	}
	while(temp[j] != NULL) //Deleting remainder values of **temp from previous
	//function executions
	{	
		temp[j] = NULL;
		j++;
	}
	while(temp[k] != NULL) //Now we load the cleansed **temp values in the
	//**args variable once again.
	{
		args[k] = temp[k];
		k++;
	}
	while(args[k] != NULL) //the remainder values of args[k] from previous
	//function execution are being cleansed.
	{	
		args[k] = NULL;
		k++;
	}

	free(temp);
	return execute_status;
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

	if(!strcmp(*args, "quit"))
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

/*
 *******************************************************************************
 * executeAll() is the main execute functions. It handles all the executions
 *******************************************************************************
 */
void executeAll(char **args)
{	
	int execute_status = 0, exit_status = 0;
	char **cmd_args = (char**)malloc(BUFFER_SIZE * sizeof(char*));
	char **rest_args = (char**)malloc(BUFFER_SIZE * sizeof(char*));
	int i=0,j=0,k=0;

	if(cmd_args == NULL || rest_args == NULL)
	{
		fprintf(stderr,"ERROR: malloc() failure.\n");
		exit(EXIT_FAILURE);
	}

	executeRecursively(args,cmd_args);

	//exit_status = executeCmd(cmd_args);
	//printf("The exit status is: %d\n",exit_status);
}

void executeRecursively(char **args, char **cmd_args)
{
	int execute_status = 0, exit_status = 0, i = 0, j = 0;

	execute_status = parseArgs(args, cmd_args);

	printed++;
	switch (execute_status)
	{
		case 0:              /* Inline command */
			exit_status = executeCmd(cmd_args);
			printf("The exit status is: %d\n",exit_status);
			break;
		case 1:              /* Commands with ';' */
			exit_status = executeCmd(cmd_args);
			executeRecursively(args, cmd_args);
			break;
		case 2:
			exit_status = executeCmd(cmd_args);
			if(exit_status == 0)
			{
				executeRecursively(args, cmd_args);
			}
			else
			{
				exit(EXIT_FAILURE);
			}
			break;
		case 3:
			printf("Not yet supported. Ciao...\n");
			break;
		case 4:
			printf("Not yet supported. Ciao...\n");
			break;
		case 5:
			printf("Not yet supported. Ciao...\n");
			break;
		default:
			printf("ERROR: Unexpected execute status.\n");
			exit(EXIT_FAILURE);
	}
}