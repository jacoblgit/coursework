// implementation for string manipulation functions

#include "string_helper.h"

// adapted from Aspenes' notes (4.10)
// returns string of line that's been read in
// return NULL pointer if no chars are read
#define INITIAL_LINE_LENGTH (40)

char* getLine(FILE* fp) {
    char* line;
    int size;
    int length;
    int c;

    size = INITIAL_LINE_LENGTH;
    line = malloc(size);
    
    length = 0;
    while((c = fgetc(fp)) != EOF && c != '\n') {
        if(length >= size - 1) {
            size *= 2;
            line = realloc(line, size);
        }

        line[length] = c;
        length++;
    }

    if (length == 0) {
        free(line);
        return NULL;
    }

    line[length] = '\0';

    return line; 
}