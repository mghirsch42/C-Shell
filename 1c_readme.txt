FILE	
	1c.c

AUTHOR
	MG Hirsch
	mghirsch@ou.edu
	University of Oklahoma

DATE
	November 2017

DESCRIPTION	
	This program mimics a UNIX shell: it reads in and executes a line of 
	keyboard input.
	Created for CS3113 - Introduction to Operating Systems

COMPILATION
	make		: Compiles and makes all executables.
	make -all	: Compiles and makes all executables.
	make -clean	: Deletes any previous executables associated with 
			  the code. Compiles and creates new executables.
	Note: You must be in the directory containing the source code to 
	compile the program in this fashion.
	
RUN
	./1c		: Runs the current executable.
	./1c f		: Runs the current executable with the batch file f.
	make run	: Compiles and makes all executables. Runs new
			  executable.

INTERNAL COMMANDS	
	cd <directory>	: Changes current working directory to <directory>. 
			  If no argument is passed, no action is taken.
			  Updates PATH variables as appropriate.
	clr		: Clears the terminal. 
	dir <directory>	: Writes all contents of <directory> to output. 
	echo <comment>	: Writes <comment> to the terminal. 
			  Note: This command is not compatible with I/O 
			  redirection. 
	environ		: Writes environment variables to the output.
	help		: Writes readme file to the terminal. 
			  Note: This command is not compatible with I/O 
			  redirection.
	pause		: Pauses all execution until the user presses the 
			  Return key.
	quit		: Exits the shell program.

REDIRECTION COMMANDS
	cmnd < f	: Redirects stdin to file f. Executes cmnd with f as 
			  input.
	cmnd args > f	: Redirects stdout to file f. Executes cmnd with args 
			  and writes output to f. If f does not exist, it is 
			  created. If f does exist, the file is truncated.
	cmnd args >> f	: Redirects stdout to file f. Executes cmnd with args 
			  and writes output to f. If f does not exist, it is 
			  created. If f does exist, the output is 
			  concatenated to the current file.	
	cmnd args &	: Shell execution will not wait for the termination
			  of cmnd args.
			  Note: "&" must be the last argument in the line.
	Note: Any commands not in the two previous lists are passed to the 
	operating system for execution.

KNOWN BUGS
	When a command is passed with an "&", the prompt requesting the next 
	command is printed immediately and the output of the current command
	is printed when execution is completed. Thus, if the next input is 
	not received before the previous command's completion, the prompt 
	will be	succeeded by the finished command's output. When this occurs, 
	a new prompt is not rewritten which could lead to ambiguity regarding
	the expected user input.

ANSWERS TO POLISHING QUESTIONS
	1. Check that arguments exist before accessing them.
		Yes. Any attempt to access and argument is wrapped in an if
		statement checking the existance of the argument.

	2. What happens when Ctrl-C is pressed during 
		- a command exection?
			program exits	
		- user entry of commands?
			program exits
	   Would SIGINT trapping control this better?
		Trapping SIGINT could definitely provide a more graceful 
		interruption of the program. On the lowest level, it could
		be indicated in the output that the user interrupted the
		program before the program is terminated. Possibly a more 
		practical implementation would be to stop the current command 
		execution within the process without halting the process 
		itself. This would give users a	way to interrupt the 
		execution without terminating the shell.

	3. The code should check the existance/permissions of input 
	   redirection files.
		Within the ckrdir method, the existance and permissions of
		redirection files are checked. If the file cannot be
		accessed, an error message is printed and execution of the
		command terminates. 

	4. The makefile works.
		Within the appropriate directory.
		make compiles all necessary files.
		make all compiles all necessary files.
		make clean deletes old executables.
		make run compiles and runs code.
