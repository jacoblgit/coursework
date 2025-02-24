#include "trackpoint.h"
#include "location.h"
#include "segment.h"
#include "list.h"

struct _segment {
    list* points;
    double length;
};

// points list helper functions
void* tp_copy_helper(const void* pt);
void tp_print_helper(FILE* out, const void* pt);
void tp_destroy_helper(void* pt);

void seg_merge_helper(const void* pt, size_t index, void* new_seg);

// segment functions

segment* seg_create() {
    segment* seg = malloc(sizeof(segment));

    seg->points = list_create(tp_copy_helper, tp_print_helper, tp_destroy_helper);
    seg->length = 0.;

    return seg;
}

void seg_destroy(segment* seg) {
    list_destroy(seg->points);
    free(seg);
}

int seg_count_points(const segment* seg) {
    return list_size(seg->points);
}

void seg_add_point(segment* seg, const trackpoint* pt) {
    list_add(seg->points, pt);

    // adds new leg to total length distance
    int num_points = seg_count_points(seg);
    if (num_points >= 2) {
        // num_points - 1 is index of last point, so
        // num_points - 2 is index of second to last point
        trackpoint* prev = (trackpoint*) list_get(seg->points, num_points - 2);
        
        location loc_prev = trackpoint_location(prev);
        location loc_curr = trackpoint_location(pt);
        seg->length += location_distance(&loc_prev, &loc_curr);
    }
}

trackpoint* seg_get_point(const segment* seg, int i) {
    return (trackpoint*) list_get((seg->points), i);
}

double seg_get_length(const segment* seg) {
    return seg->length;
}

segment* seg_merge(const segment** segs, int num_of_segs) {
    segment* merged = seg_create();

    for (int i = 0; i < num_of_segs; i++) {
        list_for_each(segs[i]->points, seg_merge_helper, merged);
    }

    return merged;
}

void seg_sort(segment *seg, int (*compare)(const void *, const void *, const void *), const void *arg) {
    list_sort(seg->points, compare, arg);
}

// copies given pt from its current segment to the given new segment
void seg_merge_helper(const void* pt, size_t index, void* new_seg) {
    seg_add_point((segment*) new_seg, pt);
}


void seg_print(FILE* out, const segment* seg) {
    fprintf(stdout, " ");
    list_print(seg->points, out);
}


// points list helper functions

void* tp_copy_helper(const void* pt) {
    return trackpoint_copy((trackpoint*) pt);
}

void tp_print_helper(FILE* out, const void* pt) {
    trackpoint* trkpnt = (trackpoint*) pt;
    fprintf(out, "%.4lf %.4lf %li\n",
        trackpoint_location(trkpnt).lat,
        trackpoint_location(trkpnt).lon,
        trackpoint_time(trkpnt));
}

void tp_destroy_helper(void* pt) {
    trackpoint_destroy((trackpoint*) pt);
}

