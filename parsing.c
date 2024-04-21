#include "parsing.h"

bool correct_cmd_format(char* line) {
    //A command is correct if it doesn't start with a space, hence the line[0],
    //and is not empty.
    return strcmp(line, "") && line[0] != ' ';
}

void validate_commands(Params* params, int argc, char** argv) {
    if (argc < MIN_ARG_COUNT || argc > MAX_ARG_COUNT) {
        format_error(); 
    }

    //Tracks if there has been a -i argument 
    bool isI = false;
    char* jobFile = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i - 1], "-i") && !params->inputFile) {
            params->inputFile = open(argv[i], O_RDONLY);
        } else if (!strcmp(argv[i], "-i") && (i != argc - 1) && !isI) {
            //argc -1 as -i cannot be last argument
            isI = true;
            continue;
        } else if (!strcmp(argv[i], "-v") && !params->verbose) {
            if (params->verbose) {
                format_error();
            }
            params->verbose = true;
        } else if (!jobFile && (strlen(argv[i]) == 1 ||
                strncmp(argv[i], "-", 1))) {
            //Will identify anything that begins with a '-' as a command, but 
            //not just a '-' alone, per demo-jobthing behaviour
            jobFile = argv[i];
        } else {
            format_error();
        }
    }

    if (!jobFile) {
        format_error();
    }
    if (params->inputFile < 0) {
        fprintf(stderr, "Error: Unable to read input file\n");
        exit(INVALID_INPUTFILE_EXIT);
    } 
    if (!(params->jobFile = fopen(jobFile, "r"))) {
        fprintf(stderr, "Error: Unable to read job file\n");
        exit(INVALID_JOBFILE_EXIT);
    }
}

void format_error() {
    fprintf(stderr, "Usage: jobthing [-v] [-i inputfile] jobfile\n");
    exit(FORMAT_ERROR_EXIT);
}

void init_params(Params* params) {
    params->jobFile = NULL;
    params->inputFile = STDIN_FILENO;
    params->verbose = false;
}
