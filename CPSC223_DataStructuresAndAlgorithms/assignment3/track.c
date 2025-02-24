#include <math.h>
#include <stdlib.h>
#include "track.h"
#include "segment.h"
#include "list.h"

struct track
{
    list *segments;
    // int num_of_segments;
};

static segment *track_get_seg(const track *trk, int i);

// segments list helper functions
void *seg_copy_helper(const void *seg);
void seg_print_helper(FILE *out, const void *seg);
void seg_destroy_helper(void *seg);

void get_lengths_helper(const void *seg, size_t index, void *lengths);
int pt_compare(const void *ele1, const void *ele2, const void *arg);
static void track_bounds(const track *trk, double *west, double *east, double *north, double *south);

// track functions

track *track_create()
{
    track *trk = malloc(sizeof(track));

    trk->segments = list_create(seg_copy_helper, seg_print_helper, seg_destroy_helper);
    list_add(trk->segments, seg_create());

    // trk->num_of_segments = 1;

    return trk;
}

void track_destroy(track *trk)
{
    list_destroy(trk->segments);
    free(trk);
}

int track_count_segments(const track *trk)
{
    return list_size(trk->segments);
    // return trk->num_of_segments;
}

int track_count_points(const track *trk, int i)
{
    return seg_count_points(track_get_seg(trk, i));
}

// returns COPY of given point in this track.
// caller takes ownership of the returned track point
trackpoint *track_get_point(const track *trk, int i, int j)
{
    return trackpoint_copy(seg_get_point(track_get_seg(trk, i), j));
}

// Caller takes ownership of returned array
double *track_get_lengths(const track *trk)
{
    double *lengths = malloc(sizeof(double) * track_count_segments(trk));
    list_for_each(trk->segments, get_lengths_helper, lengths);
    return lengths;
}

// adds a COPY of the given point to the last segment of this track
void track_add_point(track *trk, const trackpoint *pt)
{
    segment *last_seg = track_get_seg(trk, track_count_segments(trk) - 1);
    // seg_add_point makes copy
    seg_add_point(last_seg, pt);
}

void track_start_segment(track *trk)
{
    list_add(trk->segments, seg_create());
}

void track_merge_segments(track *trk, int start, int end)
{

    // make array of pointers to segments to merge
    int num_segs_to_merge = end - start;

    // if less than two segments where selected, do not execute a merge
    if (num_segs_to_merge < 2)
        return;

    const segment **segs_to_merge = malloc(sizeof(*segs_to_merge) * num_segs_to_merge);

    for (int i = 0; i < num_segs_to_merge; i++)
    {
        segs_to_merge[i] = (segment *)list_get(trk->segments, start + i);
    }

    // generates new, merged segment
    segment *merged = seg_merge(segs_to_merge, num_segs_to_merge);

    // destroys old segments that are now apart of the new merged one
    list_destroy_range(trk->segments, start, end);

    // inserts new merged segment at start index
    // track takes ownershp of new segment, but does NOT make copy of it
    list_add_at_index(trk->segments, merged, start);

    // free all temporary variables
    free(segs_to_merge);
}

// NEED TO FINISH
void track_heatmap(const track *trk, double cell_width, double cell_height,
                   int ***map, int *rows, int *cols)
{
    double west, east, north, south;
    int num_row, num_col;
    
    track_bounds(trk, &west, &east, &north, &south);
    // printf("bounds: W: %lf E: %lf N: %lf S: %lf\n", west, east, north, south);

    num_row = ceil((north - south) / cell_height);
    num_col = ceil(fmod((east - west + 360), 360) / cell_width);

    // allocate empty 2D array as heatmap
    int** hmap;
    hmap = malloc(sizeof(int*) * num_row);
    for (int i = 0; i < num_row; i++) {
        int* curr_row = calloc(num_col, sizeof(int));
        hmap[i] = curr_row;
    }
    


    // iterate through all trackpoints
    // increment the cell in hmap corresponding the the location of the trackpoint
    int num_segs = track_count_segments(trk);
    for (int curr_seg = 0; curr_seg < num_segs; curr_seg++) {
        int num_pts = track_count_points(trk, curr_seg);

        for (int curr_pt = 0; curr_pt < num_pts; curr_pt++) {
            trackpoint *pt = track_get_point(trk, curr_seg, curr_pt);
            location loc = trackpoint_location(pt);

            double deg_south_of_north_edge = north - loc.lat;
            double deg_east_of_west_edge = fmod(loc.lon - west + 360, 360);
            
            int hmap_row = fmin(floor(deg_south_of_north_edge / cell_height), num_row - 1);


            int hmap_col = floor(deg_east_of_west_edge / cell_width);
            if (fmod(deg_east_of_west_edge, cell_width) == 0. && (hmap_col > 0)) hmap_col--;
            
            // printf("(%lf, %lf) -> (%d, %d)\n", loc.lat, loc.lon, hmap_row, hmap_col);
            
            hmap[hmap_row][hmap_col]++;

            free(pt);
        }
    }

    *rows = num_row;
    *cols = num_col;
    *map = hmap;

    return;
}

// LOCAL FUNCTIONS

/**
 * Determines: 1) the latitude of the northernmost and southernmost track points in the given track; and 2)
 * the meridian of longitude at the western edge of the smallest spherical wedge bounded by two meridians
 * that contains all the points in the track (the "western edge" for a nontrivial wedge being the one that,
 * when you move east from it along the equator, you stay in the wedge).  When there are multiple such wedges,
 * this function finds the one with the lowest normalized (adjusted to the range -180 (inclusive)
 * to 180 (exclusive)) longitude.
 *
 * @param trk a pointer to a valid, non-empty track
 * @param west a pointer to a double in which to record the western edge of the containing wedge
 * @param east a pointer to a double in which to record the eastern edge of the containing wedge
 * @param north a pointer to a double in which to record the latitude of the northernmost point
 * @param south a pointer to a double in which to record the latitude of the southernmost point
 */
static void track_bounds(const track *trk, double *west, double *east, double *north, double *south)
{
    double min_lat, max_lat;
    double min_sector_size;
    double min_sector_start, min_sector_end;
    int num_pts;

        // copies every trackpoint in trk to new segment, pts.
    segment *pts = seg_create();

    int num_segs = track_count_segments(trk);
    for (int curr_seg = 0; curr_seg < num_segs; curr_seg++)
    {
        int num_pts = track_count_points(trk, curr_seg);

        for (int curr_pt = 0; curr_pt < num_pts; curr_pt++)
        {
            trackpoint *pt = track_get_point(trk, curr_seg, curr_pt);
            seg_add_point(pts, pt);
            free(pt);
        }
    }

    // **********************************************************************
    // identifies max and min latitudes
    // **********************************************************************

    min_lat = 90.0; // lat can range from -90 to 90
    max_lat = -90.0;

    num_pts = seg_count_points(pts);
    for (int i = 0; i < num_pts; i++)
    {
        trackpoint *pt = seg_get_point(pts, i);
        double curr_lat = trackpoint_location(pt).lat;

        if (curr_lat < min_lat)
            min_lat = curr_lat;
        if (curr_lat > max_lat)
            max_lat = curr_lat;
    }

    // **********************************************************************
    // identify smallest sector that contains all points
    // **********************************************************************

    // sorts segments array from west to east, starting at longitude 0
    seg_sort(pts, pt_compare, NULL);

    // makes a sorted list of longitude, without repeated longitudes.
    // repeated longitudes mess-up the calculation of the smallest sector
    double *longitudes = malloc((sizeof(double) * num_pts));
    double prev_lon = 361;
    int seg_index = 0, lon_list_index = 0;

    // seg_print(stdout, pts);
    // printf("********************\n\n");

    while (seg_index < num_pts)
    {
        trackpoint *curr_pt = seg_get_point(pts, seg_index);
        double curr_lon = trackpoint_location(curr_pt).lon;

        if (curr_lon != prev_lon)
        {
            longitudes[lon_list_index] = curr_lon;
            // printf("%lf\n", curr_lon);
            lon_list_index++;
        }

        prev_lon = curr_lon;
        seg_index++;
    }

    // printf("********************\n\n");

    int num_unique_lon = lon_list_index;

    if (num_unique_lon == 1)
    {
        min_sector_start = longitudes[0];
        min_sector_end = min_sector_start;
        min_sector_size = 0;
    }
    else if (num_unique_lon > 1)
    {

        // iterate through each longitude, calculating how large the sector
        // would be if that longitude was the western most longitude of the sector
        // identifies and stores starting longitude of smallest sector
        min_sector_size = 361.;
        min_sector_start = 361.;
        min_sector_end = 361.;
        for (int index_of_start = 0; index_of_start < num_unique_lon; index_of_start++)
        {
            double curr_start = longitudes[index_of_start];

            // pts array is storted from west to east, so
            // (index_of_start - 1) will always be the
            // corresponding end point of the sector that starts at the
            // index_of_start point. The modulus allows for treating
            // the array as a loop to avoid a special edge case
            int index_of_end = (index_of_start + num_unique_lon - 1) % num_unique_lon;
            double curr_end = longitudes[index_of_end];

            // adding 360 then taking mod 360 allows for calculating distance
            // around edge where longitudes go from positive to negative
            double curr_sector_size = fmod(curr_end - curr_start + 360, 360);

            if (curr_sector_size < min_sector_size)
            {
                min_sector_size = curr_sector_size;
                min_sector_start = curr_start;
                min_sector_end = curr_end;
            }
        }
    }
    else
    {
        printf("error in number of unique longitudes\n");
    }
    free(longitudes);

    // printf("min lat: %lf, max lat: %lf\n\n", min_lat, max_lat);

    // printf("min sector size: %lf, min sector start: %lf\n",
    //        min_sector_size,
    //        min_sector_start);
    // printf("********************\n\n");

    *west = min_sector_start;
    *east = min_sector_end;
    *north = max_lat;
    *south = min_lat;

    seg_destroy(pts);
    pts = NULL;
}

// comparison function that orders points from East to West
int pt_compare(const void *ele1, const void *ele2, const void *arg)
{
    trackpoint *pt1 = (trackpoint *)ele1;
    trackpoint *pt2 = (trackpoint *)ele2;

    double lon1 = trackpoint_location(pt1).lon;
    double lon2 = trackpoint_location(pt2).lon;

    // adjusts longitudes to be a positive
    // # degrees east of 0 (ie -179 would map to 181)
    lon1 = fmod(lon1 + 360, 360);
    lon2 = fmod(lon2 + 360, 360);

    if (lon1 < lon2)
        return (-1); // first is west of second
    else if (lon1 > lon2)
        return (1); // second is west of first
    else
        return (0);
}

// returns pointer to the segment at index
// i in the given track. list retains ownership
static segment *track_get_seg(const track *trk, int i)
{
    return (segment *)list_get(trk->segments, i);
}

void track_print(const track *trk)
{
    list_print(trk->segments, stdout);
}

// segments list helper functions

// does NOT make local copy of segment
// caller retains ownership of segment
void *seg_copy_helper(const void *seg)
{
    return seg;
}

void seg_print_helper(FILE *out, const void *seg)
{
    seg_print(out, seg);
    putc('\n', out);
}

void seg_destroy_helper(void *seg)
{
    seg_destroy(seg);
}

// fills an array, lengths, with the length of each segment in the track
// lengths must be of type double, and longer than the total number of segments
void get_lengths_helper(const void *seg, size_t index, void *lengths)
{
    ((double *)lengths)[index] = seg_get_length(seg);
}