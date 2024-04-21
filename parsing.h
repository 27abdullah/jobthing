#ifndef PARSING_H
#define PARSING_H
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

#define INVALID_INPUTFILE_EXIT 3
#define INVALID_JOBFILE_EXIT 2
#define FORMAT_ERROR_EXIT 1
#define MIN_ARG_COUNT 2
#define MAX_ARG_COUNT 5

//Contains all the jobThing parameter information specified by
//the command line arguments
typedef struct {
    FILE* jobFile;
    int inputFile;
    bool verbose;
} Params;

#endif //PARSING_H


/* init_params()
 * -------------
 * Initialises the params struct
 *
 * params: a pointer to the params struct to be initialised.
 */
void init_params(Params* params);

/* format_error()
 * --------------
 * Performs the required tasks after a format error is found.
 * Errors: exits the program with FORMAT_ERROR_EXIT (1)
 */
void format_error();

/* validate_commands()
 * -------------------
 * Validates the command line input and then uses it to populate the params
 * struct.
 *
 * params: a pointer to the params struct that is to be populated.
 *
 * argc: the number of arguments in argv
 *
 * argv: the array of command line inputs.
 *
 * Errors: will exit if inputFile cannot be opened with 
 * INVALID_INPUTFILE_EXIT (3) or if the jobFile cannot be read with 
 * INVALID_JOBFILE_EXIT(2).
 */
void validate_commands(Params* params, int argc, char** argv);

/* correct_cmd_format()
 * --------------------
 * Identifies if a command is in a correct format;
 *
 * line: the command to be checked
 *
 * Returns: true if the command is in the correct format and false if it is 
 * not.
 */
bool correct_cmd_format(char* line);
