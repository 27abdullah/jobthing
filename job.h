#ifndef JOB_H
#define JOB_H

#include "helper.h"
#include "parsing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <csse2310a3.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>

#define INITIAL_JOB_LIST 8
#define READ_END 0
#define WRITE_END 1
#define SUCCESSFUL_EXIT 0
#define FAILED_EXEC_EXIT 99
#define COMMENT '#'
#define NUMBER_RESTARTS_POSITION 0
#define INPUT_FILE_POSITION 1 
#define OUTPUT_FILE_POSITION 2
#define COMMAND_POSITION 3

//Represents and holds all the information regarding a job's input or output.
//This includes pipes to jobThing and other files the job needs to access.
typedef struct {
    bool isPipe;
    int fd;
    int pipe[2];
    char* file;
} InOut;

//Represents a job (or task) that jobthing runs
typedef struct {
    int numRestarts;
    char* cmd;
    int pid;
    bool runnable;
    int jobNumber;
    InOut* in;
    InOut* out;
    FILE* wrappedOutput;
    int startCount;
    int inputReceived;
    bool killed;
    bool restart;
} Job;

//Represents the total of all the jobs jobthing is to run
typedef struct {
    Job** tasks;
    int numberJobs;
    int size;
} Jobs;

#endif //JOB_H

/* make_job()
 * ----------
 * Makes a job using the given parameters.
 *
 * jobTokens: the array of strings defining the job from the jobfile
 *
 * verbose: whether jobthing is in verbose mode
 *
 * jobCount: the number of current jobs
 *
 * Returns: a pointer to the made job struct using the paramters.
 */
Job* make_job(char** jobTokens, bool verbose, int jobCount);

/* free_tasks()
 * ------------
 * frees the memory associated with an array of jobs
 *
 * numberJobs: the number of jobs to be freed
 *
 * jobs: a pointer to an array of pointers to job structs.
 */
void free_tasks(int numberJobs, Job** jobs);

/* init_jobs()
 * -----------
 * Initialises a jobs struct. This includes, allocating size and setting up
 * variables.
 *
 * jobs: the pointer to the jobs struct that needs to be initialised.
 */
void init_jobs(Jobs* jobs);

/* get_io_fds()
 * ------------
 * Uses input/output information from jobfile listing to create and configure
 * a job's file descriptors and/or pipes.
 *
 * isInput: is true if the direction being setup is an input for the job. 
 * false if the direction is an output.
 *
 * ioPipe: the pipe belonging to the job that is to be setup.
 *
 * fd: a pointer to the file descriptor variable of the job struct
 *
 * ioFile: the string describing the input/output of the jobfile job listing.
 *
 * isPipe: a pointer that is configuered depending to reflect the ioFile.
 *
 * Returns: true if the ioFile describes a valid input/output configuration.
 * false otherwise.
 */
bool get_io_fds(bool isInput, int ioPipe[2], int* fd, char* ioFile, 
        bool* isPipe);

/* init_in_out()
 * ------------
 * Initialises the InOut struct
 *
 * inOut: a pointer to the InOut struct to be intialised.
 */
void init_in_out(InOut* inOut);

/* start_job()
 * -----------
 * Starts the specified job. This includes handling the piping and dup2 use
 * if necessary. Handles whether this call is a restart of a job or the first
 * time a job is being made.
 *
 * job: the job to be started
 *
 * totalWorkers: pointer to integer containing the value of the number of
 * total workers.
 *
 * isRestart: true if the function is being called for a restart of the job.
 * false if it is being called to start the job for the first time.
 *
 * verbose: whether jobthing is in verbose mode.
 *
 * Errors: function will return without spawning job if the job input or output
 * configuration specified is invalid. Will set job as unrunnable and return.
 */
void start_job(Job* job, int* totalWorkers, bool isRestart, bool verbose);

/* spawn_job()
 * -----------
 * Spawns a job. It handles the io dups and closing pipes if relevant.
 *
 * job: the job to spawn.
 *
 * Errors: will error with FAILED_EXEC_EXIT (99) if the exec fails and the 
 * child is not spawned
 */
void spawn_job(Job* job);

/* all_jobs_unrunnable()
 * --------------------
 * Determines whether all jobs are unrunnable.
 *
 * jobs: pointer to array containing jobs
 *
 * Returns: true if all jobs are unrunnable, false otherwise.
 */
bool all_jobs_unrunnable(Jobs* jobs);

/* close_job_fds()
 * ---------------
 * Closes the specified job's file descriptors for its pipes or io files.
 *
 * job: the job to have its fds closed
 */
void close_job_fds(Job* job);

/* close_all_runnable_fds()
 * ------------------------
 * Closes the file descriptors of all runnable job files
 *
 * jobs: pointer to array containing the jobs and some informaiton
 */
void close_all_runnable_fds(Jobs* jobs);

/* init_job()
 * ----------
 * Initilises the job struct.
 *
 * job: a pointer to the job to be initialised.
 */
void init_job(Job* job);

/* reap_process_job(Job* job, bool verbose)
 * ----------------------------------------
 * Attempts to reap the specified job and if successful will update that
 * job's stats in its struct. If the job does not need to be reaped, does 
 * nothing.
 *
 * job: the job to attempt to be reaped.
 *
 * verbose: whether the verbose mode is set
 */
void reap_process_job(Job* job, bool verbose);

/* restart_job()
 * -------------
 * Configures and restarts the specified job
 * 
 * job: the job to be restarted.
 *
 * verbose: whether the verbose mode is set
 *
 */
void restart_job(Job* job, bool verbose);

/* populate_jobs()
 * ---------------
 * Uses the jobfile specified in params to validate listed jobs, make them,
 * and then use them to populate.
 *
 * jobs: pointer to jobs struct that is to be populated
 *
 * params: pointer to struct containing command line parameters specified
 */
void populate_jobs(Jobs* jobs, Params* params);

/* read_process_input()
 * --------------------
 * Reads input from inputfile and then ignores it, handles it as a command,
 * or sends it to all jobs, depending on what it is.
 *
 * params: the parameters specified by the command line.
 *
 * inputFile: the file that the information needs to be read
 *
 * jobs: the jobs to iterate over and send input
 *
 * Returns: false if a command is read from the input file, true otherwise.
 * Errors: exits with SUCCESSFUL_EXIT (0) if EOF is read from input file.
 */
bool read_process_input(Params* params, FILE* inputFile, Jobs* jobs);

/* process_job_output()
 * --------------------
 * Reads one line of output from each runnable, piped and not-killed job and 
 * then prints it to standard out.
 *
 * jobs: pointer to array containing jobs to be read
 *
 * verbose: whether verbose mode is set
 */
void process_job_output(Jobs* jobs, bool verbose);

/* handle_command()
 * ----------------
 * Validates input for whether it is a command and then handle the command
 * appropriately
 *
 * input: the potential command
 *
 * jobs: pointer to array containing the jobs
 */
void handle_command(char* input, Jobs* jobs);

/* handle_sleep()
 * --------------
 * Causes jobthing to sleep if the input is in a valid format
 *
 * input: the sleep command
 */
void handle_sleep(char* input);

/* handle_signal()
 * ---------------
 * Sends the specified signal to the specified job in the input argument
 * if valid.
 *
 * input: the signal command
 *
 * jobs: pointer to array containing jobs
 */
void handle_signal(char* input, Jobs* jobs);
