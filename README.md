# simple_shell

Command line shells allow one to access the capabilities of a computer using simple, interactive means. Type the name of a program and the shell will bring it into being, run it, and show output. Familiarizing yourself with the basics of shell job management will make work on terminal-only machines much more palatable.

The goal of this project is to write a simple, quasi-command line shell called commando. The shell will be less functional in many ways from standard shells like bash (default on most Linux machines), but will have some properties that distinguish it such as the ability to recall output for any child process. Like most interesting projects, commando uses a variety of system calls together to accomplish its overall purpose. Most of these will be individually discussed in lecture but the interactions between them is what inspires real danger and romance.

## Completing commando will educate an implementer on the following systems programming topics.

Basic C Memory Discipline: A variety of strings and structs are allocated and de-allocated during execution which will require attention to detail and judicious use of memory tools like Valgrind.
fork() and exec(): Text entered that is not recognized as a built-in is treated as an command (external program) to be executed. This spawns a child process which executes the new program.
Pipes, dup2(), read(): Rather than immediately print child output to the screen, child output is redirected into pipes and then retrieved on request by commando.
wait() and waitpid(), blocking and nonblocking: Child processes usually take a while to finish so the shell will check on their status every so often
