#include "helper.h"

int char_occurrences(char* line, char c) {
    int count = 0;
    int length = strlen(line);
    for (int i = 0; i < length; i++) {
        if (line[i] == c) {
            count++;
        }
    }
    return count;
}

bool is_non_neg_int(char* line) {
    int length = strlen(line);
    for (int i = 0; i < length; i++) {
        if (!isdigit(line[i])) {
            return false;
        }
    }
    return true;
}

int extract_validate_int(char* line, char* name) {
    char* pEnd;
    int value = strtol(line, &pEnd, 10);
    if (value == 0 || *pEnd != '\0') {
        printf("Error: Invalid %s\n", name);
        return -1;
    }
    return value;
}

