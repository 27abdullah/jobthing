# jobthing
Creates and manages processes according to a job specification file. Completed for the course CSSE2310 (a3).

Written to specification "CSSE2310-7231 A3 specification - 2022 sem2 v1.4.pdf" which was supplied by the CSSE2310 course. The full spec is provided in the repository. A summary of the spec is provided below.

## Introduction
You are to create a program called jobthing, which creates and manages processes according to a job specification file, and must monitor and maintain the status and input/output requirements of those processes. The assignment will also test your ability to code to a programming style guide and to use a revision control system appropriately.

## jobthing basic behaviour
jobthing reads the job specification file provided on the command line, spawning child processes and executing programs as required. In general, jobthing is required to maintain a constant process state, regardless of what happens to those child processes and programs. For example, if a child process is killed or terminates somehow, then, unless otherwise specified, jobthing is required to notice this, and re-spawn the job as required, up to the maximum number of retries specified for each job.

Depending on the contents of the jobfile, each job created by jobthing may have its stdin and stdout connected to a pipe (back to jobthing), or to a file on the filesystem. Once jobthing has created the initial set of jobs, it is to take input either from stdin, or from the file specified with the -i inputfile commandline argument, one line at a time. By default each input line should be sent to each job to which jobthing has a pipe connection, however a line starting with the asterisk character ‘*’ will be interpreted as a command. After sending the input text to each job, jobthing will then attempt to read a line of input from each job (again, only those to which jobthing is connected by a pipe). Output received from jobs is emitted to jobthing’s standard output. Upon reading EOF from the input (stdin or the input file), jobthing shall terminate.
