#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "kdtree.h"
#include "location.h"

location create_loc(double lat, double lon);
void print_helper(const location* loc, void* arg);
void loc_array_print(location* arr, int len);

int main() {
    printf("Hello world!\n");
    
    location loc1 = create_loc(10.0, 10.0);
    location loc2 = create_loc(20.0, 20.0);
    location loc3 = create_loc(30.0, 30.0);
    location loc4 = create_loc(40.0, 40.0);
    // location loc5 = create_loc(30.0, 30.0);
    location pts[] = {loc1, loc2, loc3};

    kdtree* kdt = kdtree_create(pts, 3);
    
    printf("contains point 4? %s (correct: false)\n", kdtree_contains(kdt, &loc4) ? "true" : "false");
    kdtree_add(kdt, &loc4);
    kdtree_add(kdt, &loc4);
    printf("contains point 4? %s (correct: true)\n", kdtree_contains(kdt, &loc4) ? "true" : "false");
    kdtree_remove(kdt, &loc4);
    printf("contains point 4? %s (correct: false)\n", kdtree_contains(kdt, &loc4) ? "true" : "false");
    kdtree_add(kdt, &loc4);
    printf("contains point 4? %s (correct: true)\n", kdtree_contains(kdt, &loc4) ? "true" : "false");

    // kdtree_range_for_each(kdt, &loc1, &loc1, print_helper, NULL);
    int len;
    location* range = kdtree_range(kdt, &loc1, &loc3, &len);
    loc_array_print(range, len);
    free(range);

    kdtree_destroy(kdt);

    printf("end of program!\n");
    return 0;
}

location create_loc(double lat, double lon) {
    location loc;
    loc.lat = lat;
    loc.lon = lon;
    
    assert(location_validate(&loc));
    return(loc);
}

void print_helper(const location* loc, void* arg)
{
    printf("(%5.3lf, %5.3lf)\n", loc->lat, loc->lon);
}

void loc_array_print(location* arr, int len)
{
    for (int i = 0; i < len; i++)
        printf("(%lf, %lf)\n", arr[i].lat, arr[i].lon);
}