 #Makefile to make the file hehe lol
 #to use, type
 #	> make
 #Which will create the commando program. Alternatively
 #	> make commando
 #will also create the commando program.
include test_Makefile
 #To create and run the test program use
 #	> make test

CFLAGS	=	-Wall	-g											#variable holding options to the c compiler
CC	=	gcc	$(CFLAGS)										#variable holding the compilation command

commando	:	commando.o	cmd.o	cmdcol.o	util.o	commando.h
#commando is a program, depends on 4 files, is default target
	$(CC)	-o	commando	commando.o	cmd.o	cmdcol.o	util.o
#when the other files are ready, compile commando
	@echo	commando is ready							#issue a report that the program is ready

commando.o	:	commando.c	commando.h		#commando.o depends on 2 source files
	$(CC)	-c	commando.c						#compile only, don't link yet

cmd.o	:	cmd.c	commando.h
	$(CC)	-c	cmd.c

cmdcol.o	:	cmdcol.c	commando.h
	$(CC)	-c	cmdcol.c

util.o	:	util.c	commando.h
	$(CC)	-c	util.c


#put testing shit here

clean:
	@echo	Cleaning up object files
	rm	-f	*.o	commando
