#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "string_helper.h"
#include "gmap.h"
#include "string_key.h"     // can we assume this will be in the turn-in folder?

void array_print(int* arr, size_t size);
void string_destroy(void* str);
void value_destroy(const void* key, void* value, void* arg);


int main(int argc, char** argv)
{
    size_t num_of_battles;
    int* battle_values; 
    gmap* players;
    
    char* line;
    char* token;
    char* player_name;
    char* name1;
    char* name2;
    double score1;
    double score2;
    int* allocation1;
    int* allocation2;
    int* resource_alloc;
    size_t curr_name_len, max_name_len = 0;
    char c;
    size_t i;
    
    // count number of battles
    num_of_battles = 0;
    for (i = 1; i < argc; i++)
    {
        c = argv[i][0];
        if (!isdigit(c)) break;
        /* otherwise, c represents another battle */
        num_of_battles++;
    }

    // store battle point values
    battle_values = malloc(sizeof(int) * num_of_battles);
    for (i = 0; i < num_of_battles; i++)
    {
        battle_values[i] = atoi(argv[1 + i]);
    }
    // array_print(battle_values, num_of_battles);

    // initialize gmap
    players = gmap_create(duplicate, compare_keys, hash29, string_destroy);

    // read in player resource allocation and store in gmap
    while ((line = getLine(stdin)))
    {
        // printf("%s\n", line);

        // read player name
        token = strtok(line, ",");
        curr_name_len = strlen(token);
        player_name = malloc(curr_name_len + 1);
        strcpy(player_name, token);
        if(curr_name_len > max_name_len) max_name_len = curr_name_len;

        // printf("%s\n", player_name);

        // read player's resource allocation
        resource_alloc = malloc(sizeof(int) * num_of_battles);
        for (i = 0; i < num_of_battles; i++)
        {
            token = strtok(NULL, ",");
            resource_alloc[i] = atoi(token);
        }
        // array_print(resource_alloc, num_of_battles);

        // add player information to players gmap
        gmap_put(players, player_name, resource_alloc);

        free(player_name);
        free(line);
    }

    // read in and execute each "head to head" match-up
    name1 = malloc(max_name_len + 1);
    name2 = malloc(max_name_len + 1);
    while (scanf("%s %s", name1, name2) == 2)
    {

        // retrieve players' resource allocation
        allocation1 = (int*) gmap_get(players, name1);
        allocation2 = (int*) gmap_get(players, name2);

        if (allocation1 == NULL || allocation2 == NULL) {
            /* error condition */
            printf("unrecognized player name in matchups.\n");
            break;
        }

        // calculate match-up score
        score1 = 0;
        score2 = 0;

        for (int i = 0; i < num_of_battles; i++)
        {
            // game rules dictation that each player specifies an
            // allocation for each battle, so this element retrieval
            // will not cause an memory exception
            if (allocation1[i] > allocation2[i]) 
                score1 += battle_values[i];
            else if (allocation1[i] < allocation2[i])
                score2 += battle_values[i];
            else {
                /* battle is a tie */
                score1 += battle_values[i] / 2.0;
                score2 += battle_values[i] / 2.0;
            }
        }

        // print battle results
        printf("%s %.1lf - %s %.1lf\n",
            name1, score1,
            name2, score2);

    }
    free(name1);
    free(name2);

    // free remaining memory
    gmap_for_each(players, value_destroy, NULL);
    gmap_destroy(players);
    free(battle_values);

    return 0;
}

// takes a (non-NULL) pointer to a string
// and frees it
void string_destroy(void* str)
{
    free(str);
}

// frees an given integer array as means to free all gmap values
// used as a helper for the for_each function of gmap
void value_destroy(const void* key, void* value, void* arg)
{
    free(value);
}

// for debugging
void array_print(int* arr, size_t size)
{
    printf("[ ");
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("]\n");
}