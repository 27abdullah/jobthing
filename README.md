# jobthing 
`jobthing` is a process management tool that reads job configurations from a file and spawns processes according to the provided specifications. It can manage multiple processes, handle inter-process communication, and react to specific system signals. `jobthing` can optionally connect process input/output to files or pipes, allowing for complex workflows.
## Command Line Arguments 


```Copy code
./jobthing [-v] [-i inputfile] jobfile
```
 
- **`jobfile`** : (Mandatory) The name of the job specification file.
 
- **`-v`** : (Optional) Enables verbose mode, providing additional debug and status information.
 
- **`-i inputfile`** : (Optional) Specifies an input file for `jobthing` and its processes. If not provided, input is taken from stdin.

Invalid combinations or incorrect arguments will result in a usage message:


```Copy code
Usage: jobthing [-v] [-i inputfile] jobfile
```
If the specified input file (`-i`) or jobfile cannot be read, an error message is displayed and the program exits with a specific return code: 
- Return code `1`: Invalid command line arguments.
 
- Return code `2`: Job file cannot be opened.
 
- Return code `3`: Input file cannot be opened.

### Process Creation and Management 
`jobthing` reads the job specification file, spawns child processes, and executes the commands defined. It ensures process management is maintained even if some processes terminate unexpectedly. Based on the job configuration, `jobthing` may re-launch processes up to a specified number of times or indefinitely.
## Job Specification Format 

The job specification file consists of one line per job with the following format:


```Copy code
numrestarts:input:output:cmd [arg1 arg2 ...]
```

Where:
 
- **`numrestarts`** : Specifies the number of times a job should be restarted if it terminates. `0` indicates infinite restarts, and `1` means no restarts.
 
- **`input`** : If empty, the job receives input from a pipe connected to `jobthing`. Otherwise, the named file is opened for input.
 
- **`output`** : If empty, the job sends output to a pipe connected to `jobthing`. Otherwise, the named file is opened for output.
 
- **`cmd [arg1 arg2 ...]`** : The command to be executed along with its arguments.

## Example Job Configurations 


```Copy code
# A job running cat with stdin/stdout connected to jobthing, launched once.
1:::cat

# A job running cat, stdin from /etc/services, stdout to foo.out, launched once.
1:/etc/services:foo.out:cat

# A job running cat, restarted up to 5 times on failure.
5:::cat

# A job running cat, relaunched indefinitely upon termination.
0:::cat
```

## Verbose Mode 
If verbose mode is enabled (`-v`), the following additional information is displayed: 
- For each valid job specification read from the jobfile, `jobthing` emits:

```Copy code
Registering worker N: cmd arg1 arg2 ...
```
Where `N` is the job number, and `cmd`, `arg1`, `arg2`, etc., are the command and its arguments.

## Input and Command Handling 
Once the jobs are launched, `jobthing` reads input either from stdin or the provided input file. By default, each line is sent to all jobs connected by a pipe. Lines starting with `*` are treated as commands to control the behavior of the program or report statistics.
## Signals and Job Monitoring 
`jobthing` monitors its child processes and handles specific signals. If a child process terminates, `jobthing` checks whether the job should be restarted based on the number of allowed restarts specified in the jobfile. For terminated jobs, `jobthing` logs:

```Copy code
Job N has terminated with exit code M
Job N has terminated due to signal S
```

## Example Verbose Output 


```Copy code
Registering worker 1: cat
Registering worker 2: tee logfile.txt
Spawning worker 1
Spawning worker 2
```

## Error Handling 
Errors related to file operations are logged to `stderr` and cause the affected job to be marked as invalid and unrunnable. Errors include:
- Failure to open input/output files.

- Invalid job specifications in the jobfile.

## Shutdown 

The program terminates when EOF is encountered on the input stream (either stdin or the input file) or when all jobs are marked invalid and no further processes can be managed.
