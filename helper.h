#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#endif //HELPER_H

/* char_occurrences()
 * -----------------
 * Calculates the number of occurrences of a character in a string.
 *
 * line: the line to be used to calculate the char occurences.
 *
 * char: the char that is being counted.
 *
 * Returns: the number of occurences of the char in the line.
 */
int char_occurrences(char* line, char c);

/* is_non_neg_int()
 * ----------------
 * Determines if the input is a non-negative integer 
 *
 * line: the line to be checked
 *
 * Returns: true if the line is a non-negative integer. false if it is not.
 */
bool is_non_neg_int(char* line);

/* extract_validate_int()
 * ----------------------
 * Takes in a string and then checks to see if it is a valid integer. If
 * it contains non-numerical chars e.g., 'a', it is invalid. Leading white
 * spaces are okay.
 *
 * line: the line that may contain a number
 *
 * name: the name of the value
 *
 * Returns: the value contained in the line or -1 if the line is invalid.
 * Errors: returns -1 on invalid line i.e., not an int
 */
int extract_validate_int(char* line, char* name);
