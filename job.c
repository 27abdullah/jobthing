#include "job.h"

void populate_jobs(Jobs* jobs, Params*  params) {
    char* buffer;
    while ((buffer = read_line(params->jobFile))) { 
        if (buffer[0] == COMMENT || strlen(buffer) == 0) {
            free(buffer);
            continue;
        }
       
        //Create duplicate of buffer as split_line() changes the string.
        char* tempBuffer = strdup(buffer);
        char** jobTokens = split_line(tempBuffer, ':');

        //Checks for valid format
        if (char_occurrences(buffer, ':') != 3 || 
                (!is_non_neg_int(jobTokens[NUMBER_RESTARTS_POSITION]) && 
                strcmp(jobTokens[NUMBER_RESTARTS_POSITION], "")) ||
                !correct_cmd_format(jobTokens[COMMAND_POSITION])) {
            if (params->verbose) {
                fprintf(stderr, "Error: invalid job specification: %s\n",
                        buffer);
            }
            free(buffer);
            free(tempBuffer);
            free(jobTokens);
            continue;
        }
        
        //Checks for space in jobs array
        if (jobs->numberJobs + 1 >= jobs->size) {
            jobs->size *= 2;
            jobs->tasks = realloc(jobs->tasks, sizeof(Job) * jobs->size);
        }

        jobs->tasks[jobs->numberJobs] = make_job(jobTokens, params->verbose,
                jobs->numberJobs);
        jobs->numberJobs++;
        free(buffer);
        free(jobTokens);
        free(tempBuffer);
    }
    fclose(params->jobFile);
}

void restart_job(Job* job, bool verbose) {
    job->killed = false;
    job->restart = false;
    init_in_out(job->out);
    init_in_out(job->in);
    start_job(job, NULL, true, verbose);
}

void reap_process_job(Job* job, bool verbose) {
    int status;
    switch (waitpid(job->pid, &status, WNOHANG)) {
        case 0:
            return;
        case -1:
            //Accounts for error
            return;
        default:
            if (WIFEXITED(status)) {
                printf("Job %d has terminated with exit code %d\n", 
                        job->jobNumber, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Job %d has terminated due to signal %d\n", 
                        job->jobNumber, WTERMSIG(status)); 
            }
            fflush(stdout); 
            close_job_fds(job);

            //Update variables tracking job state 
            if (--(job->numRestarts) == 0) {
                job->runnable = false;
            } else {
                job->restart = true; 
            }
    }
}

void init_job(Job* job) {
    job->startCount = 0;
    job->inputReceived = 0;
    job->killed = false;
    job->restart = false;
}

void close_all_runnable_fds(Jobs* jobs) {
    int numberJobs = jobs->numberJobs;
    for (int i = 0; i < numberJobs; i++) {
        Job* job = jobs->tasks[i];
        if (job->runnable) {
            close_job_fds(job);
        }
    }
}

void close_job_fds(Job* job) {
    close(job->in->fd);
    if (job->out->isPipe) {
        fclose(job->wrappedOutput);
    } else {
        close(job->out->fd);
    }
}

bool all_jobs_unrunnable(Jobs* jobs) {
    for (int i = 0; i < jobs->numberJobs; i++) {
        if (jobs->tasks[i]->runnable && !jobs->tasks[i]->killed) {
            return false;
        }
    }
    return true;
}

void spawn_job(Job* job) {
    InOut* in = job->in;
    InOut* out = job->out;
    
    //Handles input
    dup2(in->fd, STDIN_FILENO);
    if (in->isPipe) {
        close(in->pipe[READ_END]);
        close(in->pipe[WRITE_END]);
    }
    
    //Handles output
    dup2(out->fd, STDOUT_FILENO);
    if (out->isPipe) {
        close(out->pipe[READ_END]);
        close(out->pipe[WRITE_END]);
    }

    int numTokens;
    execvp(strtok(job->cmd, " "), split_space_not_quote(job->cmd, &numTokens));
    _exit(FAILED_EXEC_EXIT);
}

void start_job(Job* job, int* totalWorkers, bool isRestart, bool verbose) {
    InOut* in = job->in;
    InOut* out = job->out;
        
    //Sets up fds for input and output for each job. If invalid input or ouput
    //configuration specified, job will be set to unrunnable and job is not
    //run.
    if (!(get_io_fds(true, in->pipe, &(in->fd), job->in->file, 
            &(in->isPipe)) && get_io_fds(false, out->pipe, &(out->fd), 
            job->out->file, &(out->isPipe)))) {
        job->runnable = false;
        return;
    }
    job->runnable = true;
    job->startCount++;

    if ((job->pid = fork())) {
        //Sets up input and output for job
        if (in->isPipe) {
            close(in->pipe[READ_END]);
            in->fd = in->pipe[WRITE_END];
        }
        if (out->isPipe) {  
            close(out->pipe[WRITE_END]);
            out->fd = out->pipe[READ_END];
            job->wrappedOutput = fdopen(out->fd, "r");
        } 
        
        if (!isRestart) {
            job->jobNumber = ++(*totalWorkers);
        }

        if (verbose) {
            isRestart ? printf("Restarting worker %d\n", job->jobNumber) : 
                printf("Spawning worker %d\n", *totalWorkers);
        }
            
    } else {
        spawn_job(job); 
    }
}

void init_in_out(InOut* inOut) {
    inOut->isPipe = false;
}

bool get_io_fds(bool isInput, int ioPipe[2], int* fd, char* ioFile, 
        bool* isPipe) {
    if ((*isPipe = !strcmp(ioFile, ""))) {
        //If the io file is empty direct it to jobthing
        pipe(ioPipe);
        *fd = isInput ? ioPipe[READ_END] : ioPipe[WRITE_END];
    } else {
        //If an io file is specified and valid, set it up
        *fd = isInput ? open(ioFile, O_RDONLY) : open(ioFile, O_WRONLY | 
                O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
        if (*fd == -1) {
            fprintf(stderr, "Error: unable to open \"%s\" for %s\n", 
                        ioFile, isInput ? "reading" : "writing");
            return false;
        }
    }   
    return true;
}

Job* make_job(char** jobTokens, bool verbose, int jobCount) {
    Job* job = malloc(sizeof(Job));
    if (!strcmp(jobTokens[0], "")) {
        job->numRestarts = 0;
    } else {
        job->numRestarts = atoi(jobTokens[NUMBER_RESTARTS_POSITION]);
    }

    job->cmd = strdup(jobTokens[COMMAND_POSITION]); 
    init_job(job);
    
    //Setup job input and output functionality
    job->in = malloc(sizeof(InOut));
    job->out = malloc(sizeof(InOut));
    init_in_out(job->out);
    init_in_out(job->in);
    job->in->file = strdup(jobTokens[INPUT_FILE_POSITION]);
    job->out->file = strdup(jobTokens[OUTPUT_FILE_POSITION]);

    if (verbose) {
        printf("Registering worker %d:", jobCount + 1);
        
        //Prints the command used to generate worker without quotes ""
        int numWords;
        char* cmd = strdup(job->cmd);
        char** cmdTokens = split_space_not_quote(cmd, &numWords);
        for (int i = 0; i < numWords; i++) {
            printf(" %s", cmdTokens[i]);
        }

        free(cmd);
        free(cmdTokens);
        printf("\n");
    }
    return job;
}

void free_tasks(int numberJobs, Job** jobs) {
    for (int i = 0; i < numberJobs; i++) {
        free(jobs[i]->cmd);
        free(jobs[i]->in->file);
        free(jobs[i]->out->file);
        free(jobs[i]->in);
        free(jobs[i]->out);
        free(jobs[i]);
    }
    free(jobs);
}

void init_jobs(Jobs* jobs) {
    jobs->numberJobs = 0;
    jobs->size = INITIAL_JOB_LIST;
    jobs->tasks = malloc(sizeof(Job) * jobs->size);
}

void process_job_output(Jobs* jobs, bool verbose) {
    for (int i = 0; i < jobs->numberJobs; i++) {
        Job* job = jobs->tasks[i];
        if (!job->runnable || !job->out->isPipe || job->killed) {
            continue;
        }
        fflush(job->wrappedOutput); 

        char* output = read_line(job->wrappedOutput);
        if (output) {
            printf("%d->'%s'\n", job->jobNumber, output);
            free(output);
        } else if (verbose) {
            fprintf(stderr, "Received EOF from job %d\n", job->jobNumber);
        }
    }
}

bool read_process_input(Params* params, FILE* inputFile, Jobs* jobs) {
    char* input = read_line(inputFile);
    if (input == NULL) {
        free(input);
        close_all_runnable_fds(jobs);
        fclose(inputFile);
        free_tasks(jobs->numberJobs, jobs->tasks);
        exit(SUCCESSFUL_EXIT);
    } else if (input[0] == '*') {
        handle_command(input, jobs);
        usleep(1000000);
        free(input);
        return false;
    } else {
        for (int i = 0; i < jobs->numberJobs; i++) {
            Job* job = jobs->tasks[i];
            if (!job->runnable || !job->in->isPipe) {
                continue;
            }
            job->inputReceived++;
            write(job->in->fd, input, strlen(input));
            //Write a new line to flush pipe
            write(job->in->fd, "\n", 1);
            printf("%d<-'%s'\n", job->jobNumber, input);
        }
    }     
    free(input);
    return true;
}

void handle_command(char* input, Jobs* jobs) {
    char* cmd = strdup(input);
    cmd = strtok(cmd, " ");
    if (!strcmp(cmd, "*signal")) {
        handle_signal(input, jobs);
    } else if (!strcmp(cmd, "*sleep")) {
        handle_sleep(input);
    } else {
        printf("Error: Bad command '%s'\n", input);
    }
    free(cmd);
}

void handle_sleep(char* input) {
    char* inputDup = strdup(input);
    int numArgs;
    char** cmdTokens = split_space_not_quote(inputDup, &numArgs);   
    
    if (numArgs != 2) {
        printf("Error: Incorrect number of arguments\n");
        return;
    }

    int time = extract_validate_int(cmdTokens[1], "duration");
    
    //Ignore invalid values
    if (time == -1) {
        return;
    }
    free(cmdTokens);
    free(inputDup);
    usleep(time * 1000);
}

void handle_signal(char* input, Jobs* jobs) {
    char* inputDup = strdup(input);
    int numArgs;
    char** cmdTokens = split_space_not_quote(inputDup, &numArgs);

    if (numArgs != 3) {
        printf("Error: Incorrect number of arguments\n");
        return;
    }

    int jobNum, signal;
    //Ignore invalid values assigned as -1.
    //jobNum and signal are 2nd and 3rd arguments in cmdTokens by assign spec.
    if (((jobNum = extract_validate_int(cmdTokens[1], "job")) == -1)) {
        return;
    }
    if (((signal = extract_validate_int(cmdTokens[2], "signal")) == -1)) {
        return;
    }

    int jobPid;
    bool isValidJob = false;
    for (int i = 0; i < jobs->numberJobs; i++) {
        Job* job = jobs->tasks[i];
        if (job->runnable && job->jobNumber == jobNum) {
            isValidJob = true;
            jobPid = job->pid;
            break;
        }
    }

    if (!isValidJob) {
        printf("Error: Invalid job\n");
        return;
    }

    if (signal < 1 || signal > 31) {
        printf("Error: Invalid signal\n");
        return;
    }
    kill(jobPid, signal);
}
