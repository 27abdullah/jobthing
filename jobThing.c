#ifndef JOBTHING_H
#define JOBTHING_H
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
#include "signals.h"
#include "job.h"
#include "helper.h"
#include "parsing.h"
#define SUCCESSFUL_EXIT 0
#endif //JOBTHING_H

/* operation()
 * -----------
 * Handles the main operation of jobthing after jobs have been spawned and 
 * need to be tracked and interacted with.
 *
 * jobs: pointer to array containing information on jobs and the jobs 
 * themselves
 *
 * params: the setup paramters specified by command line arguments
 *
 * Errors: exits with SUCCESSFUL_EXIT (0) if there are no more viable workers
 */
void operation(Jobs* jobs, Params* params);

/* dead_pipe_handler()
 * -------------------
 * Is called when jobthing tries to write to a pipe that is terminanted. It
 * indentifies that job as being killed.
 *
 * sig: the signal sent
 *
 * info: pointer to struct containting information about sending process
 *
 * ucontext: contains signal context information
 */
void dead_pipe_handler(int sig, siginfo_t* info, void* ucontext);

/* report_stats()
 * --------------
 * Reports statistics on all jobs specified in jobfile.
 * 
 * sig: the signal causing invocation of function *
 */
void report_stats(int sig);     

//Global variable for signal handler to access jobs
Jobs* sigHandlerJobs;

int main(int argc, char** argv) { 
    Params params;
    init_params(&params);
    validate_commands(&params, argc, argv);

    Jobs jobs;
    init_jobs(&jobs);
    sigHandlerJobs = &jobs;
    populate_jobs(&jobs, &params);

    int totalWorkers = 0;
    int numJobs = jobs.numberJobs;
    for (int i = 0; i < numJobs; i++) {
        start_job(jobs.tasks[i], &totalWorkers, false, params.verbose);
    }
    
    //Setup signal handlers
    struct sigaction reportStats;
    setup_sighandler(&reportStats, report_stats, SIGHUP);
    struct sigaction block;
    setup_sighandler(&block, block_signal, SIGINT);
    
    //deadPipe sigaction is setup seperately as it uses .sa_sigaction
    struct sigaction deadPipe;
    memset(&deadPipe, 0, sizeof(deadPipe));
    deadPipe.sa_sigaction = dead_pipe_handler;
    deadPipe.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;
    sigaction(SIGPIPE, &deadPipe, 0);

    usleep(1000000);
    operation(&jobs, &params); 
    return 0;
}

void operation(Jobs* jobs, Params* params) {
    FILE* inputFile = fdopen(params->inputFile, "r");
    while(true) {
        //Reap and report on jobs
        for (int i = 0; i < jobs->numberJobs; i++) {
            Job* job = jobs->tasks[i]; 
            if (!job->runnable) {
                continue;
            }
            reap_process_job(job, params->verbose);
        }

        //Restart jobs
        for (int i = 0; i < jobs->numberJobs; i++) {
            Job* job = jobs->tasks[i]; 
            if (!job->restart || !job->runnable) {
                continue;
            }
            restart_job(job, params->verbose);
        }

        if (waitpid(-1, NULL, WNOHANG) == -1 && all_jobs_unrunnable(jobs)) { 
            close_all_runnable_fds(jobs);
            fclose(inputFile);
            free_tasks(jobs->numberJobs, jobs->tasks);
            fprintf(stderr, "No more viable workers, exiting\n");
            exit(SUCCESSFUL_EXIT);
        }
        if (!read_process_input(params, inputFile, jobs)) {
            //Continue to top if a command is sent from the input file
            continue;
        } 
        usleep(1000000);
        process_job_output(jobs, params->verbose); 
    }
}

void dead_pipe_handler(int sig, siginfo_t* info, void* ucontext) {
    int pid = info->si_pid;
    int length = sigHandlerJobs->numberJobs;
    for (int i = 0; i < length; i++) {
        Job* job = sigHandlerJobs->tasks[i];
        if (job->pid == pid) {
            job->killed = true;
        }
    }
}

void report_stats(int sig) {
    int length = sigHandlerJobs->numberJobs;
    for (int i = 0; i < length; i++) {
        Job* job = sigHandlerJobs->tasks[i];
        fprintf(stderr, "%d:%d:%d\n", job->jobNumber, job->startCount, 
                job->inputReceived);
    }
}


