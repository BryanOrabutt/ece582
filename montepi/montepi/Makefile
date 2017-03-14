#
# Makefile
#
# Used by the "make" program to build programs or perform
# tasks based on a specified dependency list.
#
# This Makefile is used to build the montepi.c example
# program.
#
# bnoble - Tue Jan  9 12:05:39 CST 2001

#
# Variables are usually declared at the top.  Typically
# they specify which compiler/linker to use and any
# flags to be passed to the compiler/linker.
#

#
# GNU C-compiler
#
CC = gcc

#
# C compiler flags to always use
#
CFLAGS = -Wall

#
# For debugging with dbx or gdb.  Turn off for performance runs
#
#CFLAGS += -g

#
# The default dependency to "make" when the make command is
# issued with no arguments goes here.
#
all : montepi gsl_rng_save_states

#
# montepi executable
#
montepi : montepi.c
	$(CC) $(CFLAGS) -o montepi montepi.c -lgsl -lgslcblas -lpthread -lm

#
# montepi assembly file
#
montepi.s : montepi.c
	$(CC) $(CFLAGS) -S montepi.c

#
# gsl_rng_save_states executable
#
gsl_rng_save_states : gsl_rng_save_states.c
	$(CC) $(CFLAGS) $(LIBS) -o gsl_rng_save_states gsl_rng_save_states.c -lgsl -lgslcblas 

#
# To ANSI format the C program, issue: make astyle
#
astyle :
	astyle --pad-oper --indent=tab --style=ansi montepi.c
	astyle --pad-oper --indent=tab --style=ansi gsl_rng_save_states.c

#
# To make a PostScript file suitable for printing, issue: make pdf
pdf :
	enscript --media=Letter --fancy-header --highlight=c --font=Courier7 montepi.c -o - | ps2pdf - montepi.c.pdf
	enscript --media=Letter --fancy-header --highlight=c --font=Courier7 gsl_rng_save_states.c -o - | ps2pdf - gsl_rng_save_states.c.pdf

#
# To remove the executables and other fluff, issue: make clean
#
clean :
	rm -f montepi montepi.exe.stackdump montepi.s montepi.c.orig montepi.c.pdf
	rm -f gsl_rng_save_states gsl_rng_save_states.exe.stackdump gsl_rng_save_states.s gsl_rng_save_states.c.orig gsl_rng_save_states.c.pdf
