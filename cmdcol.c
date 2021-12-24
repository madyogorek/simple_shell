// cmdcol.c: functions related to cmdcol_t collections of commands.

#include "commando.h"
void cmdcol_add(cmdcol_t *col, cmd_t *cmd)
// Add the given cmd to the col structure. Update the cmd[] array and
// size field. Report an error if adding would cause size to exceed
// MAX_CMDS, the maximum number commands supported.
{
  //checking if we've reached the limit on the cmd table size
  if(col->size + 1 > MAX_CMDS)
  {
    printf("ERROR: too many commands\n");
    return;
  }
  col->cmd[col->size] = cmd;
  //increment size
  col->size++;
}

void cmdcol_print(cmdcol_t *col)
// Print all cmd elements in the given col structure.  The format of
// the table is
//
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 0    #17434       0    EXIT(0) 2239 ls -l -a -F
// 1    #17435       0    EXIT(0) 3936 gcc --help
// 2    #17436      -1        RUN   -1 sleep 2
// 3    #17437       0    EXIT(0)  921 cat Makefile
//
// Widths of the fields and justification are as follows
//
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 1234 #12345678 1234 1234567890 1234 Remaining
// left  left    right      right rigt left
// int   int       int     string  int string
//
// The final field should be the contents of cmd->argv[] with a space
// between each element of the array.
{
  printf("JOB  #PID      STAT   STR_STAT OUTB COMMAND\n");
  //looping through each row
  for(int i = 0; i < col->size; i++)
  {

    printf("%d    #%d         %d       %s   %d ", i, (int)(col->cmd[i]->pid), col->cmd[i]->status, col->cmd[i]->str_status, col->cmd[i]->output_size);
    //looping through last variable to print out all arguments
    for(int j = 0; col->cmd[i]->argv[j] != NULL; j++)
    {
      printf("%s ", col->cmd[i]->argv[j]);
    }
    printf("\n");
  }
}

void cmdcol_update_state(cmdcol_t *col, int block)
// Update each cmd in col by calling cmd_update_state() which is also
// passed the block argument (either NOBLOCK or DOBLOCK)
{
  for(int i = 0; i < col->size; i++)
  {

    cmd_update_state(col->cmd[i], block);
  }
}

void cmdcol_freeall(cmdcol_t *col)
// Call cmd_free() on all of the constituent cmd_t's.
{
  for(int i = 0; i < col->size; i++)
  {
    cmd_free(col->cmd[i]);
  }
}
