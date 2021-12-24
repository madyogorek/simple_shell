//Chris Weinberger - weinb198 and Madelyn Ogorek - ogore014

#include "commando.h"


// cmd.c: functions related the cmd_t struct abstracting a
// command. Most functions maninpulate cmd_t structs.


// Allocates a new cmd_t with the given argv[] array. Makes string
// copies of each of the strings contained within argv[] using
// strdup() as they likely come from a source that will be
// altered. Ensures that cmd->argv[] is ended with NULL. Sets the name
// field to be the argv[0]. Sets finished to 0 (not finished yet). Set
// str_status to be "INIT" using snprintf(). Initializes the remaining
// fields to obvious default values such as -1s, and NULLs.
cmd_t *cmd_new(char *argv[]){
  cmd_t *cmd = malloc(sizeof(cmd_t)); //creates the new command struct we need



  strcpy(cmd->name, argv[0]);

  //set the finished field to 0
  cmd->finished = 0;

  //sets the str_status field
  char *init = "INIT";
  snprintf(cmd->str_status, 5, "%s", init);

  //sets the argv field by copying over the strings from argv argument

  int i;
  for(i = 0; argv[i] != NULL; i++){
	cmd->argv[i] = strdup(argv[i]);
  }
  cmd->argv[i] = NULL;

  //set the remaining fields to default values like -1 and NULL
  cmd->pid = -1;
  cmd->out_pipe[0] = -1; //make sure outpipe array should be null
  cmd->out_pipe[1] = -1;
  cmd->status = -1;
  cmd->output = NULL;
  cmd->output_size = -1;

  return cmd;
}

// Deallocates a cmd structure. Deallocates the strings in the argv[]
// array. Also deallocats the output buffer if it is not
// NULL. Finally, deallocates cmd itself.
void cmd_free(cmd_t *cmd){
  //first free the strings in argv structure
  for(int i = 0; cmd->argv[i] != NULL; i++){
	free(cmd->argv[i]);
  }

  //free the output buffer if it isn't NULL
  if(cmd->output != NULL){
	free(cmd->output);
  }

  //free the cmd_t struct
  free(cmd);
  return; //not sure if the return is required here
}

// Forks a process and starts executes command in cmd in the process.
// Changes the str_status field to "RUN" using snprintf().  Creates a
// pipe for out_pipe to capture standard output.  In the parent
// process, ensures that the pid field is set to the child PID. In the
// child process, directs standard output to the pipe using the dup2()
// command. For both parent and child, ensures that unused file
// descriptors for the pipe are closed (write in the parent, read in
// the child).
void cmd_start(cmd_t *cmd){
  //first, pipe the child process' out_pipe field
  pipe(cmd->out_pipe);

  //change the cmd_t start status to RUN
  char *str_status = "RUN";
  snprintf(cmd->str_status, 4, "%s", str_status);

  //fork a new process and store PID in cmd struct
  pid_t pid = fork();

  if(pid == 0){ //child process
	//changes child's printing to go into pipe we created earlier instead of terminal
	// does this by using dup2() -> second file pointer is overwritten by the first
	dup2(cmd->out_pipe[PWRITE], STDOUT_FILENO);

	//child closes the read end of the pipe
	close(cmd->out_pipe[PREAD]);

	//finally, child calls execv to switch to program in argv field
	char *child_cmd = cmd->argv[0]; //if this gives an error, use special functions

	execvp(child_cmd, cmd->argv);

  } else { //parent process
	//parent closes the write end of the pipe
	close(cmd->out_pipe[PWRITE]);

	//ensure that pid field is set to child's PID
	cmd->pid = pid;
  }

  return; //not sure if the return is required here
}

// If the finished flag is 1, does nothing. Otherwise, updates the
// state of cmd.  Uses waitpid() and the pid field of command to wait
// selectively for the given process. Passes block (one of DOBLOCK or
// NOBLOCK) to waitpid() to cause either non-blocking or blocking
// waits.  Uses the macro WIFEXITED to check the returned status for
// whether the command has exited. If so, sets the finished field to 1
// and sets the cmd->status field to the exit status of the cmd using
// the WEXITSTATUS macro. Calls cmd_fetch_output() to fill up the
// output buffer for later printing.
//
// When a command finishes (the first time), prints a status update
// message of the form
//
// @!!! ls[#17331]: EXIT(0)
//
// which includes the command name, PID, and exit status.
void cmd_update_state(cmd_t *cmd, int block){
  //if the cmd has finished, then no further action can be taken, so return immediately
  if(cmd->finished == 1){
	return;
  }

  //wait for child of cmd, using pid field which stores the child's PID
  int status;
  int res = waitpid(cmd->pid, &status, block); //block argument depends on what is passed into the function

  if (res == -1){ //error case - writeup says this is untested
	return;
  }
  else if (res == 0){ // no status change, exit out of update function
	return;
  }
  else { //res = pid of child, meaning there was a state change

	//handle the state change using macros associated with waitpid
	if(WIFEXITED(status)){ //if this evaluates to true, the child has exited properly
  	//update status field with the exit status of child
  	cmd->status = WEXITSTATUS(status);

  	//change str_status to exit;
  	snprintf(cmd->str_status, 9, "EXIT(%d)", cmd->status); //this may need to have a num attached as well

  	//set finish field to 1
  	cmd->finished = 1;

  	//calls cmd_fetch_output() to print to screen
  	cmd_fetch_output(cmd);

  	//prints message containing command name, pid, and exit status
  	printf("@!!! %s[#%d]: EXIT(%d)\n", cmd->name, cmd->pid, cmd->status);
	}
  }
  return;
}

// Reads all input from the open file descriptor fd. Stores the
// results in a dynamically allocated buffer which may need to grow as
// more data is read.  Uses an efficient growth scheme such as
// doubling the size of the buffer when additional space is
// needed. Uses realloc() for resizing.  When no data is left in fd,
// sets the integer pointed to by nread to the number of bytes read
// and return a pointer to the allocated buffer. Ensures the return
// string is null-terminated. Does not call close() on the fd as this
// is done elsewhere.
char *read_all(int fd, int *nread){
  //int *buffer = malloc(1024);
  // while(read(fd, buffer, 128) > 0){ //read from the file descriptor as long as there are bytes to read
  //
  // }
  //set up initial buffer of 1024 bytes and position before entering the loop
  int cur_pos = 0; //set a current position to keep track of where we are in the file
  int max_size = 1024;
  char *input = malloc(max_size*sizeof(char) + 1); //1 extra byte so we can null-terminate it

  while(1){ //loop until there is no more data to read
	//conditional to check if the buffer needs more space... if so, double max_size and reallcoate
	if(cur_pos >= max_size){
  	max_size = max_size * 2;
  	input = realloc(input, max_size);
	}
	if(input == NULL){
  	printf("error, could not find more space to allocate to buffer\n");
	}

	//calculate the maximum read size by account for the current position in the data
	int max_read = max_size - cur_pos;
	//read maximum amount of data possible without causing an error
	int bytes_read = read(fd, input+cur_pos, max_read);

	//check if nread==0, indicating we are at the end of the input
	if(bytes_read <= 0){
  	*nread = cur_pos;
  	//position of the next open index is equal to cur_pos... set that to null-term then exit
  	input[cur_pos] = '\0';
  	return input;
	}
	// else if (bytes_read == -1){
	//   perror("read failed");
	// }

	//adjust curr_pos by the number of bytes read
	cur_pos += bytes_read;
  }
}

// If cmd->finished is zero, prints an error message with the format
//
// ls[12341] not finished yet
//
// Otherwise retrieves output from the cmd->out_pipe and fills
// cmd->output setting cmd->output_size to number of bytes in
// output. Makes use of read_all() to efficiently capture
// output. Closes the pipe associated with the command after reading
// all input.
void cmd_fetch_output(cmd_t *cmd){
  //check if the child process is finished. If it isn't print an error message
  if(cmd->finished == 0){
	printf("%s[%d] not finished yet\n", cmd->name, (int) cmd->pid);
	return;
  }

  int num_read = 0;
  char *buffer = read_all(cmd->out_pipe[PREAD], &num_read);

  //printf("fetching here...\n");
  //printf("buffer: %s \n", buffer);

  //cmd->output = buffer, size_bytes = num_read
  cmd->output = buffer;
  //printf("cmd->output: %s", cmd->output);
  //printf("pid: %d\n", cmd->pid);
  cmd->output_size = num_read;
  //printf("num-read: %d", cmd->output_size);

  //close the pipe that was read from
  close(cmd->out_pipe[PREAD]);
  return;
}

// Prints the output of the cmd contained in the output field if it is
// non-null. Prints the error message
//
// ls[17251] : output not ready
//
// if output is NULL. The message includes the command name and PID.
void cmd_print_output(cmd_t *cmd){
  //prints error message if the output is NULL
  if(cmd->output == NULL){
	printf("%s[#%d]  : output not ready\n", cmd->name, (int) cmd->pid);
	return;
  }

  //writes the output buffer to the screen

  write(STDOUT_FILENO, cmd->output, cmd->output_size);
  return;
}
