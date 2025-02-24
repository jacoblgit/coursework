

// to do:
// make work with stdin

// Jacob Lessing
// CPSC 223 Fall 2022

// This program parses a GPX File, outputing
// location, and time data for each trkpt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// how many characters from stdin the program "remembers" as it scans
// 10 is sufficient as its larger than any attribute or tag name of interest
#define BUFFER_SIZE 10

int scan_for_attribute(char attribute[]);
int scan_for_start_tag(char element_type[]);
int print_attribute_value();
int print_element_text();

int scan_for_string(char target[], int ignore_quotes, int case_insensitive);

int main()
{    
    // repeatedly scan for additional trkpt elements to parse
    while (1) {
        // will close file and exit program if no more trkpt elements are found
        if (scan_for_start_tag("<trkpt")) {
            return 0;
        }

        // scans, parses, and prints children and fields of trkpt
        scan_for_attribute(" lat");
        print_attribute_value();
        printf(",");

        scan_for_attribute(" lon");
        print_attribute_value();
        printf(",");

        scan_for_start_tag("<ele");
        print_element_text();
        printf(",");

        scan_for_start_tag("<time");
        print_element_text();
        printf("\n");
    }
}


// ************************************ //
//          SCANNING FUNCTIONS          //
// ************************************ //


// pre: atribute should be atribute name WITHOUT appended =
// post: reads stdin stream until TARGET ATRIBUTE is found
// returns 0 if target is found
// returns 1 if end of file is reached without match
int scan_for_attribute(char attribute[]) {
    int returned_value;
    char curr_char;

    // 1: scan for attribute name, but ignore matches inside quotes
    // 0: make search case sensitive
    returned_value = scan_for_string(attribute, 1, 0);

    if (returned_value) return 1;   // end of file (EOF)
    else {
        if (fscanf(stdin, " %c", &curr_char) <= 0) return 1;    // (EOF)
        if (curr_char == '=') return 0;                         // target found
        else scan_for_attribute(attribute);                     // otherwise, search again
    }

    return 1;
}

// pre: takes element type preceeded by <
// post: scans stdin stream until desired tag is found
// returns 0 if target is found
// returns 1 if end of file is reached without match
int scan_for_start_tag(char element_type[]) {
    int returned_value;
    char curr_char;

    // 1: scan for attribute name, but ignore matches inside quotes
    // 1: make search case insensitive
    
    returned_value = scan_for_string(element_type, 1, 1);

    // checks that identified attribute name is followed by ' ' or '>'
    if (returned_value) return 1;   // end of file (EOF)
    else {
        if (fscanf(stdin, "%c", &curr_char) <= 0) return 1;   // (EOF)
        if (curr_char == ' ' || curr_char == '>') {
            // re-add char to stream so it can be used later
            if (curr_char == '>') ungetc('>', stdin);
            if (curr_char == ' ') ungetc(' ', stdin);
            return 0;                                          // target found
        }
        else scan_for_start_tag(element_type);                 // otherwise, search again
    }

    return 1;
}


// ************************************ //
//          PRINTING FUNCTIONS          //
// ************************************ //

// pre: attribute must be first thing in quotes
// post: prints next attribute value
// returns 0 if full value is sucessfully printed
// returns 1 if file ends before closing "/' is encountered
int print_attribute_value() {
    char curr_char;

    // value can be enclosed in " or '
    char enclosing_char;

    // used as a boolean to indicate a state:
    // 1 indicates stdin is inside an atribute value
    // 0 indicates stdin is outside any atribute value 
    int is_inside_value = 0;

    while (fscanf(stdin, "%c", &curr_char) > 0) {
        switch (is_inside_value) {
        case 0:
            // search for start of attribute value
            if (curr_char == '"' || curr_char == '\'') {
                is_inside_value = 1;
                enclosing_char = curr_char;
            }
            break;
        case 1:
            // print characters until final quote is reached
            if (curr_char == enclosing_char) return 0;
            else if (curr_char == ',') printf("&comma");
            else printf("%c", curr_char);
            
            break;
        }
    }

    // end of file reached
    return 1;

}

// pre: element has no children & stdin is in start tag of desired element
// post: prints the element text of the current element
// returns 0 if full value is sucessfully printed
// returns 1 if file ends before printing is complete
int print_element_text() {
    // >, outside of any string, marks end of start tag
    if (scan_for_string(">", 1, 0)) return 1;

    // print all characters until start of end tag <
    char curr_char;
    while (fscanf(stdin, "%c", &curr_char) > 0) {
        if (curr_char == '<') return 0;
        else if (curr_char == ',') printf("&comma");
        else printf("%c", curr_char);
    }

    // end of file reached without closing ">"
    return 1;
}


// ************************************ //
//            HELPER FUNCTIONS          //
// ************************************ //


// post: reads stdin stream until TARGET string is found
// returns 0 if target is found
// returns 1 if end of file is reached without match
// ignores characters in quotes when flag ignore_quotes == 1
// executes case-insensitive search when flag case_insensitive == 1
int scan_for_string(char target[], int ignore_quotes, int case_insensitive)
{
    // stores BUFFER_SIZE most recent characters from input stream

    // *** I wanted to initialize my buffer like this, but
    // the code didn't work as a I expected ***
    //char buffer[BUFFER_SIZE + 1];
    //buffer[BUFFER_SIZE] = '\0';
    char buffer[] = "xxxxxxxxxx";

    // most recently read char from input
    char curr_char;

    while (fscanf(stdin, "%c", &curr_char) > 0) {

        // checks if character is quote, and skips quote if ignore_quotes == 1
        if (ignore_quotes && (curr_char == '"' || curr_char == '\'')) {
            // flushes buffer
            for (size_t i = 0; i < BUFFER_SIZE; i++) {
                buffer[i] = '`'; // garbage character
            }
            
            // identifies if " or ' started quote
            char enclosing_char = curr_char;

            // skips characters until corresponding quote,
            // or end of file is reached
            while (fscanf(stdin, "%c", &curr_char) > 0) {
                if (curr_char == enclosing_char) break;
            }

            // if corresponding quote was found, execution of TARGET search continues
            if (curr_char == enclosing_char) continue;

            // otherwise, reached end of file without finding target
            else return 1;
        }

        // clear space in buffer for new char
        for (size_t i = 0; i < (BUFFER_SIZE - 1); i++) {
            buffer[i] = buffer[i + 1];
        }

        // forces character to lowercase if case insensitive is flagged
        if (case_insensitive) curr_char = tolower(curr_char);

        // append next input char to buffer
        buffer[BUFFER_SIZE - 1] = curr_char;

        // check if buffer contains TARGET
        if (strstr(buffer, target) != NULL) {
            // found TARGET
            return 0;
        }
    }

    // reached end of file without finding TARGET
    return 1;
}