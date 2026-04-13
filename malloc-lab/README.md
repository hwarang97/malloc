#####################################################################
# CS:APP Malloc Lab
# Handout files for students
#
# Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
# May not be used, modified, or copied without permission.
#
######################################################################

***********
Main Files:
***********

mm.{c,h}	
	Your solution malloc package. mm.c is the file that you
	will be handing in, and is the only file you should modify.

mdriver.c	
	The malloc driver that tests your mm.c file

short{1,2}-bal.rep
	Two tiny tracefiles to help you get started. 

Makefile	
	Builds the driver

**********************************
Other support files for the driver
**********************************

config.h	Configures the malloc lab driver
fsecs.{c,h}	Wrapper function for the different timer packages
clock.{c,h}	Routines for accessing the Pentium and Alpha cycle counters
fcyc.{c,h}	Timer functions based on cycle counters
ftimer.{c,h}	Timer functions based on interval timers and gettimeofday()
memlib.{c,h}	Models the heap and sbrk function

*******************************
Building and running the driver
*******************************
To build the driver, type "make" to the shell.

To run the driver on a tiny test trace:

	unix> mdriver -V -f short1-bal.rep

The -V option prints out helpful tracing and summary information.

To get a list of the driver flags:

	unix> mdriver -h


*******************************
Implicit Linked List Performence
*******************************
trace  valid  util     ops      secs  Kops   
 0       yes   99%    5694  0.005256  1083   
 1       yes  100%    5848  0.005171  1131   
 2       yes   99%    6648  0.007401   898   
 3       yes  100%    5380  0.005204  1034   
 4       yes  100%   14400  0.000088164009   
 5       yes   92%    4800  0.006784   708   
 6       yes   92%    4800  0.006461   743   
 7       yes   55%   12000  0.112502   107   
 8       yes   51%   24000  0.173027   139   
 9       yes   30%   14401  0.046062   313   
10       yes   34%   14401  0.001918  7507   
Total          77%  112372  0.369872   304   
   
Perf index = 46 (util) + 20 (thru) = 67/100
