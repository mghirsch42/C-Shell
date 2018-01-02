/*
 * Filename: 
 * 	1c.c
 * Author: 
 * 	MG Hirsch
 * Date: 
 * 	November 2017
 * Description:
 * 	see readme
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>


#define MAX_BUFFER 1024
#define MAX_ARGS 64
#define SEPARATORS " \t\n"
#define README "readme"

struct shellstatus_st{
	int foreground;
	char *infile;
	char *outfile;
	char *outmode;
	FILE *in;
	FILE *out;
};

typedef struct shellstatus_st shellstatus;

extern char **environ;

void pprompt();
void ckrdir(char **, shellstatus *);
int set_lflag(int);
void redirin(shellstatus);
void redirout(shellstatus);
void stddir(shellstatus*);

int main (int argc, char **argv){
	char linebuf[MAX_BUFFER];	// line buffer
	char cmndbuf[MAX_BUFFER];	// command buffer
	char *args[MAX_ARGS];		// pointers to arg strings
	char **arg;			// working pointer thru args
	char *readmepath;		// readme pathname
	shellstatus status;		// status of shell
	FILE *input;			// current input file			

	/* Check for batch file
	   If there is one, open it and point input to it
	   Otherwise leave input as stdin */
	input = stdin;
	if(argv[1]) {
		input = fopen(argv[1], "r");
	}

	/* Set readmepath */
	readmepath = malloc(MAX_BUFFER);
	getcwd(readmepath, MAX_BUFFER-7);
	strcat(readmepath, "/readme");

	/* Loop over all input */
	while(!feof(input)) {
	
		/* Reset foreground */ 
		status.foreground = 1;

		/* Print prompt */
		if(status.foreground) {
			pprompt();
		}
		fflush(stdout);
		
		
		/* Get input from terminal */
		if(fgets(linebuf, MAX_BUFFER, input)) {	

			/* tokenize the input into args array */
			arg = args;
			*arg++ = strtok(linebuf, SEPARATORS);	// tokenize input
			while((*arg++ = strtok(NULL, SEPARATORS))); // last entry will be NULL

			if(args[0]) {	// if there's anything there
				
				/* initialize cmndbuf */
				cmndbuf[0] = 0;
					
				/* check for redirection symbols */
				ckrdir(args, &status);

				/**
				 * check for internal command 
				 */

				/* ECHO */
				if(!strcmp(args[0], "echo")) { 
					int i = 1;
					while(args[i]) {
						fputs(args[i], stdout);
						fputs(" ", stdout);
						i++;
					}
					fputs("\n", stdout);
				} else
				
				/* PAUSE */
				if(!strcmp(args[0], "pause")) {
					set_lflag(0);
					printf("Press Enter to continue...");
					while(getchar() != '\n');
					set_lflag(1);
					printf("\n");
				}else 

				/* HELP */
				if(!strcmp(args[0], "help")) {
					FILE *fptr;
					fptr = fopen(readmepath, "r");
					
					if(fptr==NULL) {
						perror("Error");
					}
					
					char *contents = malloc(MAX_BUFFER);
				
					while(fgets(contents, MAX_BUFFER, fptr) != NULL) {
						fputs(contents, stdout);
						free(contents);
						contents = malloc(MAX_BUFFER);
					}
					
					fclose(fptr);
				} else

				/* CLEAR */
				if (!strcmp(args[0], "clr")) {	// "clear" command
					strcat(cmndbuf, "clear");
				} else

				/* QUIT */
				if (!strcmp(args[0], "quit")){	// "quit" command
					break;			// break out of while loop
				} else
					
				/* EVIRON  */
				if(!strcmp(args[0], "environ")) {	// "environ" command
					char ** env = environ;
					while(*env) {
						printf("%s\n", *env++);	// step through the environment variables
					}
				} else

				/* DIR */
				if(!strcmp(args[0], "dir")){ 	// "dir" command

					strcpy(cmndbuf, "ls");

					if(args[1] == NULL) {
						args[1] = "-al";
						args[2] = ".";
					}else {
						args[2] = args[1];
						args[1] = "-al";
					}

				} else

				/* CD */
				if(!strcmp(args[0], "cd")) {
					if(args[1] != NULL) {

						char* dir = args[1]; 									
						if(chdir(dir)) {
							printf("%s\n", strerror(errno));
						} 
					}

					char cwd[MAX_BUFFER];
					getcwd(cwd, MAX_BUFFER);
					char* pwd = malloc(strlen("PWD=") + strlen(cwd) + 1);
					strcpy(pwd, "PWD=");
					strcat(pwd, cwd);
					if(putenv(pwd)) {
						printf("%s\n", strerror(errno));
					}
				}

				/* Pass command to OS */
				else {					
					arg = args;
					strcat(cmndbuf, *arg++);	
				}
				
				/* If we have commands to pass to the OS, execute them */
				if(cmndbuf[0]) {
		
					int pid = fork();
					switch(pid) {
						/* Error case */
						case -1:	
							printf("Error: forking error");	

						/* Child case */
						case 0:;
							
							/* Open redirect files */
							redirin(status);
							redirout(status);

							/* Execute command */
							execvp(cmndbuf, args);

							/* Close redirect files */
							if(status.in) fclose(status.in);
							if(status.out) fclose(status.out);
									
							exit(0);
				
						/* Parent case */
						default:	

							/* Wait if necessary */
							if(status.foreground) {
								wait(NULL);
							} 
					}
				}
			}
		}
	} // end while

	return 0;	
	
} // end main


/**
 * void pprompt()
 * Print the input prompt to the terminal.
 */
void pprompt() {
	char *p = malloc(MAX_BUFFER);
	getcwd(p, MAX_BUFFER);
	strcat(p, " ==> ");
	printf("%s", p);
}


/**
 * void ckrdir(char **args, shellstatus *stat)
 * Check args  for redirection symbols and update stat to reflect findings.
 */
void ckrdir(char **args, shellstatus *stat) {

	stat->infile = NULL;
	stat->outfile = NULL;
	stat->outmode = NULL;
		
	// Loop through each of the arguments
	while(*args) {
		
		// Check for input redirection
		if(!strcmp(*args, "<")) {
			
			*args = NULL; 		// delimit args
			if(*(++args)) {		// file exists

				if(access(*args, F_OK)) {
					printf("Error: file does not exist.");
				}
				else if(access(*args, R_OK)) {
					printf("Error: file not readable.");
				}
				else {
					stat->infile = *args;
				}
			
			} else {
			
				printf("Please provide a file name.");
				break;
			}
		}

		// Check for output redirection
		else if(!strcmp(*args, ">") || !strcmp(*args, ">>")) {
			if(!strcmp(*args, ">")) {
				stat->outmode = "w";
			}
			else {
				stat->outmode = "a";
			}
			
			*args = NULL;		// delimit args
			if(*(++args)) {		// file exists

				if(access(*args, W_OK)) {
					printf("Error: file does not exist.");
				}
				else if(access(*args, F_OK)) {
					printf("Error: file not readable.");	
				}
				else {
					stat->outfile = *args;
				}
			}
			else {
				printf("Please provide a file name.");
				break;
			}
		} // end if
		
		args++;
		
	}

	// Check for "&" indicating a no-wait status
	if(!strcmp(*(--args), "&")) {
		stat->foreground = 0;
		*args = NULL;
	}
}


/**
 * int set_lflag(int desc, int value)
 * Turns on and off keyboard echo.
 * If value = 0, turn echo off. If value = 1, turn echo on.
 * Return 1 when successful. Return 0 when unsuccessful.
 */
int set_lflag(int value) {

	struct termios settings;
	int result;

	result = tcgetattr(0, &settings);
  	if (result < 0)
    	{
      		perror ("Error");
	      	return 0;
    	}
	settings.c_lflag &= ~ECHO;
	if (value)
		settings.c_lflag |= ECHO;
		settings.c_iflag |= IUTF8;
 	result = tcsetattr (0, TCSANOW, &settings);
  	if (result < 0) {
      		perror ("Error");
      		return 0;
   	}
  	return 1;
}

/* 
 * void redirout(shellstatus status)
 * If status.outfile contains a file name, open that file as standard output.
 * If the file cannot be opened as standard output, print an error message.
 */
void redirout(shellstatus status) {
	
	status.out = stdout;

	if (status.outfile) {
		if(!(status.out = freopen(status.outfile, status.outmode, stdout))) {
			printf("Error: Cannot open %s for output.", status.outfile);
			status.out = stdout;
		}
	}
}	


/* 
 * void redirin(shellstatus status)
 * If status.infile contains a file name, open that file as standard input.
 * If the file cannot be opened as standard input, print an error message.
 */
void redirin(shellstatus status) {

	status.in  = stdin;

	if (status.infile) {
		if(!(status.in = freopen(status.infile, "r", stdin))) {
			printf("Error: Cannot open %s for input.", status.infile);
			status.in = stdin;
		}
	} 
}	
