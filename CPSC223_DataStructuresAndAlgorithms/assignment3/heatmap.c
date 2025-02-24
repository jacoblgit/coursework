#include "track.h"
#include "trackpoint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

char peak(FILE *in);
void int_array2D_destroy(int **arr, int rows);

int main(int argc, char **argv)
{
    double cell_width, cell_height;
    track *trk;
    int **map;
    int rows, cols;
    char* symbols;
    int symbol_range;
    int num_symbols;

    cell_width = atof(argv[1]);
    cell_height = atof(argv[2]);
    symbols = argv[3];
    symbol_range = atoi(argv[4]);

    // generate track with given trackpoint data
    trk = track_create();

    char c;
    while ((c = peak(stdin)) != EOF)
    {
        double lat, lon;
        long time;
        trackpoint *pt;

        // create new segement, if needed
        if (c == '\n')
            track_start_segment(trk);

        fscanf(stdin, "%lf %lf %ld", &lat, &lon, &time);
        pt = trackpoint_create(lat, lon, time);
        track_add_point(trk, pt);

        // clears '\n' at end of line
        fgetc(stdin);

        trackpoint_destroy(pt);
    }

    // track_print(trk);
    // printf("********************\n\n");

    track_heatmap(trk, cell_width, cell_height, &map, &rows, &cols);

    // testing
    // printf("%d, %d\n", rows, cols);

    // for (int i = 0; i < rows; i++)
    // {
    //     for (int j = 0; j < cols; j++)
    //     {
    //         printf("%d ", map[i][j]);
    //     }
    //     putchar('\n');
    // }

    // printing heatmap
    num_symbols = strlen(symbols);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            char symbol;
            int which_symbol;

            which_symbol = fmin((map[i][j] / symbol_range), num_symbols - 1);
            symbol = symbols[which_symbol];

            putchar(symbol);
        }
        putchar('\n');
    }

    int_array2D_destroy(map, rows);
    track_destroy(trk);
}

// peaks at next char in stream.
// returns EOF if cannot read next char
char peak(FILE *in)
{
    char c = getc(in);
    return ungetc(c, in);
}

// deallocates a 2d array
void int_array2D_destroy(int **arr, int rows)
{
    for (int i = 0; i < rows; i++)
    {
        free(arr[i]);
    }
    free(arr);
}