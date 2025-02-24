#include "gmap.h"
#include "string_key.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void string_destroy(void* str);


int main(int argc, char* argv[])
{
    // char str[] = "this, is, a, string";
    // char* token;

    // token = strtok(str, ",");
    // printf("%s\n", token);

    // while((token = strtok(NULL, ",")) != NULL)
    //     printf("%s\n", token);

    gmap* map;
    map = gmap_create(duplicate, compare_keys, hash29, string_destroy);

    char key1[] = "key1";
    int val1[] = {1, 2 , 3};

    gmap_put(map, key1, val1);

    gmap_destroy(map);

    printf("program ran to completion.\n");
}

// takes a pointer to a string
// (not NULL) and frees it
void string_destroy(void* str)
{
    free(str);
}