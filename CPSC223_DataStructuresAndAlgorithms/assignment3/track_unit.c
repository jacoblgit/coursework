#include <stdio.h>
#include <stdlib.h>

#include "track.h"
#include "trackpoint.h"
#include "location.h"

location short_segment[] = {{41.3078680, -72.9342120},
			  {41.3078780, -72.9342340},
			  {41.3078810, -72.9342590}};

location parallel_short_segment[] = {{41.3078680, -72.8342120},
				     {41.3078780, -72.8342340},
				     {41.3078790, -72.8342260},
				     {41.3078810, -72.8342590}};

const location *two_segment[] = {short_segment, parallel_short_segment};
int two_segment_lengths[] = {3, 4};

int small_map_counts[][3] = {{1, 2, 3}, {4, 5, 6}};
int small_map_rows = 2;
int small_map_cols = 3;

track *make_track(const location **loc, int num_segs, int *num_pts, long t);

void create_destroy_track();
void empty_track_counts();
void add_and_get(const location **pts, int num_segs, int *num_pts, long t);
void add_and_get_single(const location *pts, int n);
void counts(const location **pts, int num_segs, int *num_pts, long t);
void add_out_of_order(int new_segments);
void start_segment_when_empty(const location **pts, int num_segs, int *num_pts);
void merge(int start_segments, int merge_start, int merge_end);
void copy_in_add();
void heatmap(int rows, int cols, int counts[][cols], double north, double west);
void free_heatmap(int **map, int rows);


int main(int argc, char **argv)
{
  if (argc < 2)
    {
      fprintf(stderr, "USAGE: %s test-number\n", argv[0]);
      return 1;
    }

  int test = atoi(argv[1]);
  switch (test)
    {
    case 1:
      create_destroy_track();
      break;

    case 2:
      empty_track_counts();
      break;

    case 3:
      copy_in_add();
      break;

    case 4:
      add_and_get_single(short_segment, sizeof(short_segment) / sizeof(location));
      break;

    case 5:
      add_and_get(two_segment, 2, two_segment_lengths, 2000);
      break;

    case 6:
      counts(two_segment, 2, two_segment_lengths, 2000);
      break;

    case 7:
      // generic merge
      merge(6, 2, 4);
      break;

    case 8:
      // merge including 1st segment
      merge(6, 0, 3);
      break;

    case 9:
      // merge including only 1 segment
      merge(6, 2, 3);
      break;

    case 10:
      heatmap(small_map_rows, small_map_cols, small_map_counts, 45.0, 10.0);
      break;

    case 11:
      heatmap(small_map_rows, small_map_cols, small_map_counts, 45.0, 179.0);
      break;

    default:
      fprintf(stderr, "%s: invalid test number %s\n", argv[0], argv[1]);
      return 1;
    }

  return 0;
}


track *make_track(const location **loc, int num_seg, int *num_pts, long t)
{
  track *trk = track_create();
  if (trk == NULL)
    {
      return NULL;
    }

  long time = t;
  for (int seg = 0; seg < num_seg; seg++)
    {
      if (seg > 0)
	{
	  track_start_segment(trk);
	}
      
      for (int i = 0; i < num_pts[seg]; i++)
	{
	  trackpoint *pt = trackpoint_create(loc[seg][i].lat, loc[seg][i].lon, time++);
	  if (pt != NULL)
	    {
	      track_add_point(trk, pt);
	      trackpoint_destroy(pt);
	    }
	  else
	    {
	      fprintf(stderr, "ERROR: failed to create point\n");
	      track_destroy(trk);
	      return NULL;
	    }
	}

    }
  return trk;
}


void create_destroy_track()
{
  track *trk = track_create();
  track_destroy(trk);
  printf("PASSED\n");
}


void empty_track_counts()
{
  track *trk = track_create();
  if (trk == NULL)
    {
      printf("ERROR: trackpoint_create returned NULL\n");
      return;
    }

  int segment_count;
  if ((segment_count = track_count_segments(trk)) != 1)
    {
      printf("ERROR: empty track has %d segments\n", segment_count);
      return;
    }

  int segment_size;
  if ((segment_size = track_count_points(trk, 0)) != 0)
    {
      printf("ERROR: empty segment has %d points\n", segment_size);
      return;
    }
  
  track_destroy(trk);
  printf("PASSED\n");
}


void add_and_get_single(const location *pts, int n)
{
  long start_time = 1000;

  int segment_lengths[] = {n};
  add_and_get(&pts, 1, segment_lengths, start_time);
}


void add_and_get(const location **pts, int num_segs, int *num_pts, long t)
{
  track *trk = make_track(pts, num_segs, num_pts, t);
  if (trk == NULL)
    {
      printf("ERROR: couldn't make track\n");
      return;
    }

  long time = t;
  for (int seg = 0; seg < num_segs; seg++)
    {
      for (int i = 0; i < num_pts[seg]; i++)
	{
	  trackpoint *pt = track_get_point(trk, seg, i);
	  if (pt == NULL)
	    {
	      printf("ERROR: couldn't get point\n");
	      track_destroy(trk);
	      return;
	    }
      
	  location gotten = trackpoint_location(pt);
	  if (gotten.lat != pts[seg][i].lat)
	    {
	      printf("ERROR: saved then retrieved latitide doesn't match -- %f %f\n", gotten.lat, pts[seg][i].lat);
	      trackpoint_destroy(pt);
	      track_destroy(trk);
	      return;
	    }
      
	  if (gotten.lon != pts[seg][i].lon)
	    {
	      printf("ERROR: saved then retrieved longitude doesn't match -- %f %f\n", gotten.lon, pts[seg][i].lon);
	      trackpoint_destroy(pt);
	      track_destroy(trk);
	      return;
	    }
	  
	  if (trackpoint_time(pt) != time)
	    {
	      printf("ERROR: saved then retrieved time doesn't match -- %ld %ld\n", trackpoint_time(pt), time);
	      trackpoint_destroy(pt);
	      track_destroy(trk);
	      return;
	    }
	  trackpoint_destroy(pt);
	  
	  time++;
	}
    }
  
  track_destroy(trk);
  printf("PASSED\n");
}


void counts(const location **pts, int num_segs, int *num_pts, long t)
{
  track *trk = make_track(pts, num_segs, num_pts, t);
  if (trk == NULL)
    {
      printf("ERROR: couldn't make track\n");
      return;
    }

  if (track_count_segments(trk) != num_segs)
    {
      printf("ERROR: number of segments %d is incorrect\n", track_count_segments(trk));
      track_destroy(trk);
      return;
    }
  else
    {
      int i = 0;
      while (i < num_segs && track_count_points(trk, i) == num_pts[i])
	{
	  i++;
	}
      if (i < num_segs)
	{
	  printf("ERROR: number of points %d on segment %d is incorrect\n", track_count_points(trk, i), i);
	  track_destroy(trk);
	  return;
	}
    }
  
  track_destroy(trk);
  printf("PASSED\n");
}


void merge(int start_segments, int merge_start, int merge_end)
{
  long start_time = 20000000L;
  
  track *trk = track_create();
  if (trk == NULL)
    {
      printf("ERROR: could not create track\n");
      return;
    }

  // build a track with known segments and points
  double lat = 0.0;
  double lon = 0.0;
  long time = start_time;
  for (int i = 0; i < start_segments; i++)
    {
      if (i > 0)
	{
	  track_start_segment(trk);
	}

      for (int j = 0; j < i + 1; j++)
	{
	  trackpoint *pt = trackpoint_create(lat, lon, time);
	  lat += 0.1;
	  lon += 0.1;
	  time += 10;
	  if (pt == NULL)
	    {
	      printf("ERROR: could not create trackpoint\n");
	      track_destroy(trk);
	      return;
	    }
	  track_add_point(trk, pt);

	  trackpoint_destroy(pt);
	}
    }

  // do the requested merge
  track_merge_segments(trk, merge_start, merge_end);

  // compute what should have happened
  int deleted_segments;
  if (merge_start >= 0 && merge_start < start_segments && merge_end > merge_start && merge_end <= start_segments)
    {
      deleted_segments = merge_end - merge_start - 1;
    }
  else
    {
      deleted_segments = 0;
    }
  
  int expected_segments = start_segments - deleted_segments;

  // compute expected points and check against what is on the track
  if (track_count_segments(trk) != expected_segments)
    {
      printf("ERROR: incorrect number of segments %d after merge\n", track_count_segments(trk));
      track_destroy(trk);
      return;
    }

  lat = 0.0;
  lon = 0.0;
  time = start_time;
  
  for (int i = 0; i < expected_segments; i++)
    {
      int expected_length = 0;
      if (i < merge_start || deleted_segments == 0)
	{
	  expected_length = i + 1;
	}
      else if (i == merge_start)
	{
	  for (int s = merge_start; s < merge_end; s++)
	    {
	      expected_length += (s + 1);
	    }
	}
      else
	{
	  expected_length = i + deleted_segments + 1;
	}
      
      if (track_count_points(trk, i) != expected_length)
	{
	  printf("ERROR: segment %d has size %d after merge\n", i, track_count_points(trk, i));
	  track_destroy(trk);
	  return;
	}

      for (int j = 0; j < expected_length; j++)
	{
	  trackpoint *pt = track_get_point(trk, i, j);
	  if (pt == NULL)
	    {
	      printf("ERROR: segment %d point %d is NULL after merge\n", i, j);
	      track_destroy(trk);
	      return;
	    }

	  location loc = trackpoint_location(pt);
	  if (loc.lat != lat || loc.lon != lon)
	    {
	      printf("ERROR: got point %f %f from segment %d point %d after merge\n", loc.lat, loc.lon, i, j);
	      track_destroy(trk);
	      trackpoint_destroy(pt);
	      return;
	    }

	  if (trackpoint_time(pt) != time)
	    {
	      printf("ERROR: got time %ld from segment %d point %d after merge\n", trackpoint_time(pt), i, j);
	      track_destroy(trk);
	      trackpoint_destroy(pt);
	      return;
	    }

	  trackpoint_destroy(pt);
	  lat += 0.1;
	  lon += 0.1;
	  time += 10;
	}
    }

  track_destroy(trk);
  printf("PASSED\n");
}

void copy_in_add()
{
  track *trk = track_create();

  if (trk == NULL)
    {
      printf("ERROR: couldn't make track\n");
      return;
    }

  trackpoint *pt = trackpoint_create(47.5, -125.0, 1000);
  track_add_point(trk, pt);

  trackpoint_destroy(pt);

  // make a new point to increase probablity we overwrite the old point
  pt = trackpoint_create(0.0, 0.0, 0);
  trackpoint_destroy(pt);

  pt = track_get_point(trk, 0, 0);
  if (pt == NULL)
    {
      printf("ERROR: could not retrieve point\n");
      track_destroy(trk);
      return;
    }

  location loc = trackpoint_location(pt);
  if (loc.lat != 47.5 || loc.lon != -125.0 || trackpoint_time(pt) != 1000)
    {
      printf("ERROR: trackpoint in track affected by destruction of original\n");
      track_destroy(trk);
      trackpoint_destroy(pt);
      return;
    }

  track_destroy(trk);
  trackpoint_destroy(pt);
  printf("PASSED\n");
}

void heatmap(int rows, int cols, int counts[][cols], double north, double west)
{
  double cell_width = 1.0;
  double cell_height = 1.0;
  long time = 1000;
  
  track *trk = track_create();

  if (trk == NULL)
    {
      printf("ERROR: couldn't make track\n");
      return;
    }

  bool made_north = false;
  bool made_west = false;
  
  for (int r = 0; r < rows; r++)
    {
      for (int c = 0; c < cols; c++)
	{
	  for (int p = 0; p < counts[r][c]; p++)
	    {
	      // compute lat and lon for point to add so all are distinct
	      // and there is always a point on the north edge and
	      // a point on the west edge, otherwise all points are
	      // not on the borders
	      double lat;
	      double lon;
	      if (p == 0 && r == 0 && c == 0)
		{
		  // first point in NW cell goes on NW corner
		  lat = north;
		  lon = west;
		  made_north = true;
		  made_west = true;
		}
	      else if (p == 0 && r == 0 && !made_north)
		{
		  // first point on top row goes on top if there wasn't
		  // already something in the NW corner of NW cell
		  lat = north;
		  lon = west + cell_width * c + (p + 1) * (cell_width / (counts[r][c] + 1));
		  made_north = true;
		}
	      else if (p == 0 && c == 0 && !made_west)
		{
		  // first point in first column goes on west edge if
		  // there wasn't already something in NW corner of NW cell
		  lat = north - cell_height * r - (p + 1) * (cell_height / (counts[r][c] + 1));
		  lon = west;
		  made_west = true;
		}
	      else
		{
		  // other points go diagonally through cell
		  lat = north - cell_height * r - (p + 1) * (cell_height / (counts[r][c] + 1));
		  lon = west + cell_width * c + (p + 1) * (cell_width / (counts[r][c] + 1));
		}

	      if (lon >= 180.0)
		{
		  // normalize longitude
		  lon -= 360.0;
		}
	      
	      trackpoint *pt = trackpoint_create(lat, lon, time);
	      track_add_point(trk, pt);
	      trackpoint_destroy(pt);
	      time++;
	    }
	}
    }

  int **map;
  int map_rows;
  int map_cols;
  track_heatmap(trk, 1.0, 1.0, &map, &map_rows, &map_cols);

  if (map == NULL)
    {
      printf("ERROR: couldn't make heatmap\n");
      track_destroy(trk);
      return;
    }

  if (map_rows != rows || map_cols != cols)
    {
      printf("ERROR: heatmap dimensions %d %d incorrect\n", map_rows, map_cols);
      free_heatmap(map, map_rows);
      track_destroy(trk);
      return;
    }

  for (int r = 0; r < map_rows; r++)
    {
      if (map[r] == NULL)
	{
	  printf("ERROR: heatmap row %d is NULL\n", r);
	  free_heatmap(map, map_rows);
	  track_destroy(trk);
	  return;
	}
      
      for (int c = 0; c < map_cols; c++)
	{
	  if (map[r][c] != counts[r][c])
	    {
	      printf("ERROR: heatmap entry %d %d is incorrect %d\n", r, c, map[r][c]);
	      free_heatmap(map, map_rows);
	      track_destroy(trk);
	      return;
	    }
	}
    }

  free_heatmap(map, map_rows);
  track_destroy(trk);
  printf("PASSED\n");
}

void free_heatmap(int **map, int rows)
{
  for (int r = 0; r < rows; r++)
    {
      free(map[r]);
    }
  free(map);
}
