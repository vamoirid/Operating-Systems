/*
 *******************************************************************************
 *                                                                             *
 *                           Filename: myshell.c                               *
 *                                                                             *
 *                      Author: Amoiridis Vasileios 8772                       *
 *                                                                             *
 *                             Date: 12 Jan 2020                               *
 *******************************************************************************
 */
#include <stdio.h>          /* Standard Library                               */
#include <stdlib.h>         /* Standard Library                               */
#include <string.h>         /* strlen(), strtok()                             */
#include <sys/types.h>      /* fork(),getpid() system calls                   */
#include <sys/stat.h>       /* contains info about open(), creat()            */
#include <sys/wait.h>       /* wait() system calls                            */
#include <unistd.h>         /* fork(),getpid() system calls                   */  
#include <errno.h>          /* contains the errors' descriptions              */
#include <fcntl.h>          /* contains information about file descriptor     */

/*
 *******************************************************************************
 * DEFINES                                                                     *
 *******************************************************************************
 */
#define BUFFER_SIZE 1024
#define MAX_LINE_SIZE 512
#define SPACE_DELIM " \t\r\n\a"
#define MAX_CMD_NUM 256
#define GREEN "\033[0;32m"
#define GREEN_BOLD "\033[1;32m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define RESET_COLOR "\033[0m"

/*
 *******************************************************************************
 * Functions' definitions                                                      *
 *******************************************************************************
 */
void   mainLoop         (int argc, const char *argv[]);
void   printPromptName  (void);
void   quitShell        (void);
FILE*  chooseInput      (int argc, const char *argv[]);
char*  readLine         (FILE* input);
char** parseLine        (char *line);
int    checkArgs        (char **args);
int    parseArgs        (char **args, char **cmd_args);
int    executeCmd       (char **args);
void   executeAll       (char **args);
void   executeRecursive (char **args, char **cmd_args);
int    executeRedirect  (char **args, char **cmd_args, int redirect_mode);
void   shiftLeftArgs    (char **args);
int    executePipe      (char **args, char **cmd_args);

/*
 *******************************************************************************
 * Main Code                                                                   *
 *******************************************************************************
 */
int main(int argc, const char *argv[])
{
	mainLoop(argc, argv);

	return 0;
}

/*
 *******************************************************************************
 * mainLoop() is the code that runs on every execution until a "quit" command  * 
 * or until an invalid argument.                                               *
 *******************************************************************************
 */
void mainLoop(int argc, const char *argv[])
{
	printf("Welcome to my Shell! My name is Vasileios Amoiridis and I am the creator.\n");
	FILE* input = chooseInput(argc, argv);
	char* line = NULL;
	char** args = NULL;
	int exit_status = 0, check_status = 0;
	int i = 1;

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

	} while(1);
	
}

/*
 *******************************************************************************
 * printPromptName() is a function which print the name of the prompt in the   * 
 * start of each line.                                                         *
 *******************************************************************************
 */
void printPromptName()
{
	printf(GREEN_BOLD "amoiridis_8772> " RESET_COLOR);
}

/*
 *******************************************************************************
 * quitShell() is a function which is used to terminate the shell operation.   *
 *******************************************************************************
 */
void quitShell()
{
	printf("Thanks for the cooperation. It's been a pleasure!\n");
	exit(EXIT_SUCCESS);
}

/*
 *******************************************************************************
 * chooseInput() is a function which checks the number of argc variable and    *
 * decides if the shell will be in interactive or batch mode. Moreover it      *
 * checks for empty batch files or inappropriate number of arguments.          *
 *******************************************************************************
 */
FILE* chooseInput(int argc, const char* argv[])
{	
	FILE* input;

	switch (argc)
	{
		case 1: /* User typed: ./bin/myshell */
			input = stdin;
			break;
		case 2: /* User typed: ./bin/myshell "batchfile_name" */
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
 * readLine() function reads either 1 line from stdin or a line from a batch   *
 * file and returns it. It checks for exceeding the maximum permitted line     *
 * size and also if the end of file is reached. It also handles the memory     *
 * allocation of the line.                                                     *
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
 * parseLine() function reads an input line and "cuts" it into several pieces  *
 * of arguments in order to save the several arguments in the commands. It     *
 * returns an array of strings which contains the seperate arguments.          *
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
 * checkArgs() is a function which checks for possible syntax errors in the    *
 * arguments provided by the user.                                             *
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
		return check_status;
	}
	while(args[i] != NULL)
	{	
		if(strstr(args[i], ";;") != NULL)
		{	
			printf("ERROR: Bad syntax. Two or more ';' found sequentially\n");
			check_status = 1;
			return check_status;
		}
		else if(strstr(args[i], "&&&") != NULL)
		{
			printf("ERROR: Bad syntax. Three or more '&' found sequentially\n");
			check_status = 1;
			return check_status;
		}
		else if (i > 0)
		{
			if(!strcmp(args[i], args[i-1]) && (!strcmp(args[i], ";") || 
			   !strcmp(args[i], "&&")))
			{
				printf("ERROR: Bad syntax. Unexpected token after ; or &&.\n");
				check_status = 1;
				return check_status;			
			}
			else if(!strcmp(args[i],"&&") && !strcmp(args[i-1],";") ||
				    !strcmp(args[i],";") && !strcmp(args[i-1],"&&"))
			{
				printf("ERROR: Bad syntax. Unexpected token after ; or &&.\n");
				check_status = 1;
				return check_status;	
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
		return check_status;
	}

	return check_status;
}

/*
 *******************************************************************************
 * parseArgs() is a function which has as input the arguments that were in the * 
 * line and outputs an integer with the execute status of the commands         *
 * regarding the special characters that were found in them. Moreover is a     *
 * function which takes as an input the cmd_args and the args. Both of the are *
 * arrays of strings which are about to be executed. The variable cmd_args     *
 * contains the command that is to be executed, and the variale args contaings *
 * the arguments after the special character. For example in a situation that  *
 * we have ``` echo hello && ls -l ; pwd ``` then                              *
 * cmd_args = echo hello                                                       *
 * args = ls -l ; pwd                                                          *
 * execute_status = 2
 *******************************************************************************
 */
int parseArgs(char **args, char **cmd_args)
{
	char **temp = (char**)malloc(MAX_CMD_NUM * sizeof(char*));
	int i = 0, cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0, cnt5 = 0, execute_status = 0;

	while(cmd_args[cnt1] != NULL)
	{
		cmd_args[cnt1] = NULL;
		cnt1++;
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
			if(args[i+2] != NULL)
			{
				if(!strcmp(args[i+2],">"))
				{
					execute_status = 6;
					break;
				}
			}
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
		temp[cnt2] = args[i];
		i++;	
		cnt2++;
	}
	while(args[cnt3] != NULL) //the remainder values of args[k] from previous
	//function execution are being cleansed.
	{	
		args[cnt3] = NULL;
		cnt3++;
	}
	while(temp[cnt4] != NULL) //Now we load the cleansed **temp values in the
	//**args variable once again.
	{
		args[cnt4] = temp[cnt4];
		cnt4++;
	}
	while(temp[cnt5] != NULL) //Now we clean all the remaining values of temp
	//in order to free the memory.
	{
		temp[cnt5] = NULL;
		cnt5++;
	}

	free(temp);

	return execute_status;
}
/*
 *******************************************************************************
 * shiftLeftArgs() is a function which takes as an input an array of strings   *
 * which in our case is the line of arguments, and what it does is that it     *
 * shifts left so as to remove the first argument each time that it is called. *
 *******************************************************************************
 */
void shiftLeftArgs(char **args)
{
	char **temp = (char**)malloc(MAX_CMD_NUM * sizeof(char*));
	int i = 0, cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0, cnt5 = 0, execute_status = 0;

	while(temp[i] != NULL)
	{
		temp[i] = NULL;
		i++;
	}
	i = 0;

	while(args[i+1] != NULL)
	{
		temp[i] = args[i+1];
		i++;
	}
	i = 0;

	while(args[i] != NULL)
	{
		args[i] = NULL;
		i++;
	}
	i = 0;

	while(temp[i] != NULL)
	{
		args[i] = temp[i];
		i++;
	}
	i = 0;
}

/*
 *******************************************************************************
 * executeCmd() is a function which takes as an input the array of commands    *
 * which generated from parseLine() function and executes the first command.   *
 * If the command was executed succesfully then the return value is 0. If      *
 * the command didn't execute then the return value is 1.                      *
 *******************************************************************************
 */
int executeCmd(char **args)
{
	pid_t pid, wait_pid;
	int status,i=0;

	if(!strcmp(*args, "quit"))
	{
		quitShell();
	}

	pid = fork();
	if(pid == 0) //Child
	{
		//fprintf(stdout, "executeCmd Command %s: %s\n", args[0], strerror(errno));
		if(execvp(args[0], args) == -1)
		{
			//fprintf(stdout, "executeCmd Command %s: %s\n", args[0], strerror(errno));
			perror("Command"); 
			//fprintf(stderr, "%s\n", stderror(errno));
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
	else //Parent
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
 * executeAll() is the main execute functions. It handles all the executions   *
 *******************************************************************************
 */
void executeAll(char **args)
{	
	int execute_status = 0, exit_status = 0;
	char **cmd_args = (char**)malloc(BUFFER_SIZE * sizeof(char*));

	if(cmd_args == NULL)
	{
		fprintf(stderr,"ERROR: malloc() failure.\n");
		exit(EXIT_FAILURE);
	}

	executeRecursive(args,cmd_args);
}
/*
 *******************************************************************************
 * executeRecursive() is a function which takes as an input the cmd_args and   *
 * the args. Both of the are arrays of strings which were executed in the      *
 * previous execution. parseArgs() is responsible for the cmd_args, args and   *
 * execute_status and then executeRecursive() is responsible for the cases of  *
 * execute_status that are to be executed.                                     *
 *******************************************************************************
 */
void executeRecursive(char **args, char **cmd_args)
{
	int execute_status = 0, exit_status = 0, new_exit_status = 0, i = 0, j = 0;

	execute_status = parseArgs(args, cmd_args);

	switch (execute_status)
	{
		case 0:              /* Inline command     */
			exit_status = executeCmd(cmd_args);
			break;
		case 1:              /* Commands with ';'  */
			exit_status = executeCmd(cmd_args);
			executeRecursive(args,cmd_args);
			break;
		case 2:				 /* Commands with '&&' */
			exit_status = executeCmd(cmd_args);
			if(exit_status == 0)
			{
				executeRecursive(args,cmd_args);
			}
			break;
		case 3:              /* Redirect from < */
			if(args[1] != NULL) // 'command' < file.txt
			{
				if(!strcmp(args[1],";"))
			    {
					exit_status = executeRedirect(args,cmd_args,execute_status);
					shiftLeftArgs(args);
					shiftLeftArgs(args);
					executeRecursive(args,cmd_args);
				}
				else if(!strcmp(args[1],"&&"))
				{
					exit_status = executeRedirect(args,cmd_args,execute_status);
					if(exit_status == 0)
					{
						shiftLeftArgs(args);
						shiftLeftArgs(args);
						executeRecursive(args,cmd_args);					
					}
				}
			}
			else
			{
				exit_status = executeRedirect(args,cmd_args,execute_status);
				break;
			}
			break;
		case 4:             /* Redirect to > */
			exit_status = executeRedirect(args,cmd_args,execute_status);

			if(args[1] == NULL) // 'command' > file.txt
			{
				break;
			}
			else if(!strcmp(args[1],";"))
			{
				shiftLeftArgs(args);
				shiftLeftArgs(args);
				executeRecursive(args,cmd_args);
			}
			else if(!strcmp(args[1],"&&"))
			{
				if(exit_status == 0)
				{
					shiftLeftArgs(args);
					shiftLeftArgs(args);
					executeRecursive(args,cmd_args);					
				}
			}
			break;
		case 5:
			exit_status = executePipe(args,cmd_args);
			break;
		case 6: /* Redirect with < and then with > */
			exit_status = executeRedirect(args,cmd_args,execute_status);
			if(args[3] == NULL)
			{
				i = 0; while(args[i] != NULL) shiftLeftArgs(args); 
				break; //empty the whole line and move on to the next one
			}
			else if(!strcmp(args[3],";"))
			{
				for(int i = 0; i < 4; i++) shiftLeftArgs(args);
		    	executeRecursive(args,cmd_args);
			}
			else if(!strcmp(args[3],"&&"))
			{
				if(exit_status == 0)
				{
					for(int i = 0; i < 4; i++) shiftLeftArgs(args);
					executeRecursive(args,cmd_args);					
				}
			}
			break;			
		default:
			printf("ERROR: Unexpected execute status %d.\n",execute_status);
			exit(EXIT_FAILURE);
	}
}
/*
 *******************************************************************************
 * executeRedirect() is a function which takes the cmd_args and the args and   *
 * what it does is that it with respect to the type of redirection that was    *
 * observed, executes the command and redirects either the input or the output *
 * or even both to where the user commanded.                                   *
 *******************************************************************************
 */
int executeRedirect(char **args, char **cmd_args, int redirect_mode)
{
	pid_t pid, wait_pid;
	int status, fd[2], i = 0;

	pid = fork();

	if(pid < 0) //Error
	{
		perror("fork");
		printf("Failed to make child.\n");
	}
	else if(pid == 0) //Child
	{
		if(redirect_mode == 3) // < redirection
		{	
			fd[0] = open(args[0],O_RDONLY);
			dup2(fd[0],STDIN_FILENO);
			close(fd[0]);
		}
		else if(redirect_mode == 4) // > redirection
		{
			fd[1] = creat(args[0],0644);
			dup2(fd[1],STDOUT_FILENO);
			close(fd[1]);
		}
		else if(redirect_mode == 6) // <> redirection
		{
			fd[0] = open(args[0],O_RDONLY);
			dup2(fd[0],STDIN_FILENO);
			close(fd[0]);
			fd[1] = creat(args[2],0644);
			dup2(fd[1],STDOUT_FILENO);
			close(fd[1]);
		}
		else 
		{
			printf("Not supported redirect mode.\n");
			exit(EXIT_FAILURE);
		}
		//fprintf(stdout, "executeRec Command %s: %s\n", cmd_args[0], strerror(errno));
		if(execvp(cmd_args[0], cmd_args) == -1)
		{
			//fprintf(stdout, "executeRec Command %s: %s\n", cmd_args[0], strerror(errno));
			perror("Command");
		}
		exit(EXIT_FAILURE);
	}
	else //Parent
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
 * executePipe() is a function which is responsible for pipeline commands. As  *
 * before, the cmd_args are the command before the pipelining that are about   *
 * be executed and args are the rest after the pipeline character.             *
 *******************************************************************************
 */
int executePipe(char **args, char **cmd_args)
{
	pid_t pid1, pid2, wait_pid;
	int status, fd[2], i =0;

	if(pipe(fd) < 0) /* fd[0]: reading end, fd[1]: writing end */
	{
		perror("pipe");
		printf("Pipe failed to create.\n");
		return EXIT_FAILURE;
	}

	pid1 = fork();

	if(pid1 < 0) //Error
	{
		perror("fork");
		printf("Failed to make child.\n");		
	}
	else if(pid1 == 0) //Child
	{
		close(fd[0]); //close reading end.
		dup2(fd[1],STDOUT_FILENO);

		if(execvp(cmd_args[0], cmd_args) == -1)
		{
			//fprintf(stdout, "executeRec Command %s: %s\n", cmd_args[0], strerror(errno));
			perror("CommandPipe");
		}
		exit(EXIT_FAILURE);
	}
	else //Parent
	{
		close(fd[1]); //close writing end
		do
		{
			wait_pid = wait(&status);
		} while(!WIFEXITED(status)); 

		pid2 = fork();

		if(pid2 < 0) //Error
		{
			perror("fork");
			printf("Failed to make child.\n");			
		}
		else if(pid2 == 0) //Child
		{
			close(fd[1]); //close writing end.
			dup2(fd[0],STDIN_FILENO);

			executeRecursive(args,cmd_args);
			exit(EXIT_SUCCESS);
			printf("Edw erxomai?\n");
		}
		else
		{
			close(fd[0]); //close reading end.
			do
			{
				wait_pid = wait(&status);
			} while(!WIFEXITED(status)); 

			return WEXITSTATUS(status);
		}
	}
}
