#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "commando.h"

int main(int argc, char *argv[]){
  setvbuf(stdout, NULL, _IONBF, 0); //Turn off output buffering

  //by default, echo is set to false, turn on iff user entered that on command line
  int echo = 0;
  if(argc > 1){

	if(strcmp(argv[1], "--echo")==0 || (getenv("COMMANDO_ECHO") != NULL)){
  	echo = 1;
	}
  }




  //create cmdcol_t structure
  cmdcol_t cmd_table;// = malloc(sizeof(cmd_table));

  cmd_table.size=0;

  cmdcol_t *cmdcol = &cmd_table;

  //Enter the main while loop for commando
  while(1){
	printf("@> ");

	//create a buffer to store user's input, call it using fget
	char buf[MAX_LINE];
  //checking for end of input
	char *fgot = fgets(buf, MAX_LINE, stdin);
  if(fgot == NULL)
  {
    printf("\n");
    printf("End of input");
    break;
  }



	//print out the string if echoing is enabled
	if(echo){
  	printf("%s", buf);
	}

	//next, parse the entered command into tokens that can be read
	char *tokens[ARG_MAX];
	int num_tokens;

	parse_into_tokens(buf, tokens, &num_tokens);

	//if nothing was entered, then go into the next iteration of the loop so the user can enter more commands
	if(num_tokens == 0){
  	continue;
	}
	//compare the first token to built in functions, if it doesn't match any, create new cmd_t struct
	if(strncmp(tokens[0], "list", 4) == 0){
  	cmdcol_print(cmdcol);
	}
	else if(strncmp(tokens[0], "help", 4) == 0){
  	printf("COMMANDO COMMANDS\n");
  	printf("help           	: show this message\n");
  	printf("exit           	: exit the program\n");
  	printf("list           	: list all jobs that have been started giving information on each\n");
  	printf("pause nanos secs   : pause for the given number of nanseconds and seconds\n");
  	printf("output-for int 	: print the output for given job number\n");
  	printf("output-all     	: print output for all jobs\n");
  	printf("wait-for int   	: wait until the given job number finishes\n");
  	printf("wait-all       	: wait for all jobs to finish\n");
  	printf("command arg1 ...   : non-built-in is run as a job");
printf("\n");
	}
	else if(strncmp(tokens[0], "pause", 5) == 0){
  	long nanos = atoi(tokens[1]);
  	int secs = atoi(tokens[2]);
  	pause_for(nanos, secs);
	}
	else if(strncmp(tokens[0], "output-for", 10) == 0){
  	int index = atoi(tokens[1]); //this is where the user entered the cmd they want to view output for
  	cmd_t *curr = cmdcol->cmd[index];
  	printf("@<<< Output for %s[#%d] (%d bytes):\n", curr->name, curr->pid, curr->output_size);

    printf("----------------------------------------\n");
  	cmd_print_output(curr);
    printf("----------------------------------------\n");
	}
	else if(strncmp(tokens[0], "output-all", 10) == 0){
  	cmd_t *curr;

  	for(int index = 0; index < cmdcol->size; index++){
    	curr = cmdcol->cmd[index];

      printf("@<<< Output for %s[#%d] (%d bytes):\n", curr->name, curr->pid, curr->output_size);
      printf("----------------------------------------\n");
    	cmd_print_output(curr);
      printf("----------------------------------------\n");
  	}

	}
	else if(strncmp(tokens[0], "exit", 4) == 0){
  	break;
	}
	else if(strncmp(tokens[0], "wait-for", 8) == 0){
  	int index = atoi(tokens[1]);
  	cmd_update_state(cmdcol->cmd[index], DOBLOCK);
	}
	else if(strncmp(tokens[0], "wait-all", 8) == 0){
  	if(cmdcol->size > 0){
    	cmdcol_update_state(cmdcol, DOBLOCK);
  	}
	}
	else{ // if none of the built-ins match, create new cmd structure, set tokens to argv, start it running
  	cmd_t *cmd = cmd_new(tokens);

  	//add new cmd to cmd_col strucutre
  	cmdcol_add(cmdcol, cmd);
  	//start new command
  	cmd_start(cmd);
	}

	//update all child processes
	if(cmdcol->size > 0){
  	cmdcol_update_state(cmdcol, NOBLOCK);
	}

  }
  //outside of loop, before exiting the program, free all allocated memory
  cmdcol_freeall(cmdcol);

}
