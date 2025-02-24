#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "kdtree.h"
#include "location.h"

void unit_test_remove(size_t n, bool readd);
void unit_test_add(size_t n);
void unit_test_build(size_t n);
void unit_test_contains(size_t n);
void unit_test_for_each(size_t n, double sw_lat, double sw_lon, double ne_lat, double ne_lon, bool add_borders);
void unit_test_range(size_t n, double sw_lat, double sw_lon, double ne_lat, double ne_lon);

void unit_test_add_time_random(size_t n, int on, double lat_scale, double lon_scale);
void unit_test_range_time(size_t n, int on, double lat_scale, double lon_scale);


/**
 * Prints the given point to standard output using the given format string
 *
 * @param l a pointer to a location, non-NULL
 * @param a a pointer to a printf format string suitable for two doubles
 */
void unit_print_point_formatted(const location *l, void *a);


/**
 * Prints the given point to standard output in the default format.
 *
 * @param l a pointer to a location, non-NULL
 */
void unit_print_point(const location *, void *);


/**
 * Determines if two points are close in either latitude or longitude to
 * each other.  "Close" means within the given parameter.
 *
 * @param l1 a pointer to a location, non-NULL
 * @param l2 a pointer to a location, non-NULL
 * @param close a positive number
 */
bool unit_not_close(const location *l1, const location *l2, double close);


static location unit_test_points[] =
  {
   {24.904359601287595, -164.679680919231197},
   {22.1889723646466788, -157.4889113459804237},
   {22.9975176700462036, -156.5980384054300085},
   {19.352792614743985, -162.739619141829996},
   {19.71197032632596, -161.0682580028004942},
   {24.796794566495814, -157.0503041973512346},
   {22.858083728266023, -158.3873640532406428},
   {15.356712575011505, -157.8071684212029604},
   {23.4204901549550186, -162.370437601685534},
   {23.8584532617037777, -157.5370774004888363},
   {22.588669770893219, -157.028378147416816},
   {19.0969835095968481, -160.17803538487762616},
   {22.2147131873924506, -158.355708807669218},
   {16.697782049411713, -164.92397630464659},
   {20.3756508986791074, -162.999269739764995},
   {17.079064596253004, -162.023667312991769},
   {18.8630989313068347, -163.993546207602046},
   {18.0848192792889675, -162.654559728282388},
   {21.8331905048543264, -161.9598511838378752},
   {15.070625181945118, -163.9570156703153074}
  };
size_t unit_test_count = sizeof(unit_test_points) / sizeof(location);


int main(int argc, char **argv)
{
  int test = 0;

  if (argc > 1)
    {
      test = atoi(argv[1]);
    }

  switch (test)
    {
    case 1:
      unit_test_add(0);
      break;

    case 2:
      unit_test_add(1);
      break;

    case 3:
      unit_test_contains(1);
      break;
      
    case 4:
      unit_test_add(unit_test_count / 2);
      break;

    case 5:
      unit_test_contains(unit_test_count / 2);
      break;
      
    case 6:
      unit_test_for_each(unit_test_count, -90.0, -180.0, 90.0, 180.0, false);
      break;

    case 7:
      unit_test_for_each(unit_test_count, 22.0, -158.0, 23.0, -157.0, false);
      break;

    case 8:
      unit_test_range(unit_test_count, -90.0, -180.0, 90.0, 180.0);
      break;

    case 9:
      unit_test_range(unit_test_count, 22.0, -158.0, 23.0, -157.0);
      break;

    case 10:
      unit_test_range(unit_test_count, 30.0, -160.0, 31.0, -159.0);
      break;

    case 11:
      unit_test_for_each(unit_test_count, 22.0, -158.0, 23.0, -157.0, true);
      break;

    case 12:
      unit_test_remove(1, false);
      break;

    case 13:
      unit_test_remove(unit_test_count / 2, false);
      break;

    case 14:
      unit_test_remove(unit_test_count / 2, true);
      break;

    case 15:
      unit_test_build(unit_test_count);
      break;

    case 16:
      if (argc > 3)
	{
	  size_t n = atoi(argv[2]);
	  int on = atoi(argv[3]);
	  if (n > 0)
	    {
	      unit_test_add_time_random(n, on, 1.0, 1.0);
	    }
	}
      break;

    case 17:
      if (argc > 3)
	{
	  size_t n = atoi(argv[2]);
	  int on = atoi(argv[3]);
	  if (n > 0)
	    {
	      unit_test_range_time(n, on, 1.0, 1.0);
	    }
	}
      break;

    default:
      fprintf(stderr, "USAGE: %s test-number\n", argv[0]);
      return 1;
    }

  return 0;
}


void unit_print_point(const location *loc, void *a)
{
  printf("%f %f\n", loc->lat, loc->lon);
}


void unit_test_add(size_t n)
{
  // create an empty tree
  kdtree *t = kdtree_create(NULL, 0);

  if (t == NULL)
    {
      printf("FAILED -- could not create tree\n");
      return;
    }

  // add the points to it
  for (size_t i = 0; i < n; i++)
    {
      if (!kdtree_add(t, &unit_test_points[i]))
	{
	  printf("FAILED -- could not add point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
      
      if (kdtree_add(t, &unit_test_points[i]))
	{
	  printf("FAILED -- added duplicate point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }

  // destroy the tree
  kdtree_destroy(t);
  printf("PASSED\n");
}

void unit_test_build(size_t n)
{
  // build a tree using the first n points from the test points
  kdtree *t = kdtree_create(unit_test_points, n);

  if (t == NULL)
    {
      printf("FAILED -- could not create tree\n");
      return;
    }

  // verify that contains finds all the points
  for (size_t i = 0; i < n; i++)
    {
      if (!kdtree_contains(t, &unit_test_points[i]))
	{
	  printf("FAILED -- missing point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }

  // destroy the tree
  kdtree_destroy(t);
  printf("PASSED\n");
}


void unit_test_contains(size_t n)
{
  // build an empty tree
  kdtree *t = kdtree_create(NULL, 0);

  if (t == NULL)
    {
      printf("FAILED -- could not create tree\n");
      return;
    }

  // add the first n test points to the tree
  for (size_t i = 0; i < n; i++)
    {
      if (!kdtree_add(t, &unit_test_points[i]))
	{
	  printf("FAILED -- could not add point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }

  // verify that contains can find the points
  for (size_t i = 0; i < n; i++)
    {
      if (!kdtree_contains(t, &unit_test_points[i]))
	{
	  printf("FAILED -- missing point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }

  // verify that contains does *not* find the test points not added
  for (size_t i = n; i < unit_test_count; i++)
    {
      if (kdtree_contains(t, &unit_test_points[i]))
	{
	  printf("FAILED -- contains point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }

  // destroy the tree
  kdtree_destroy(t);
  printf("PASSED\n");
}


void unit_test_for_each(size_t n, double sw_lat, double sw_lon, double ne_lat, double ne_lon, bool add_borders)
{
  // create an empty tree
  kdtree *t = kdtree_create(NULL, 0);

  if (t == NULL)
    {
      printf("FAILED -- could not create tree\n");
      return;
    }
  else
    {
      // add the first n test points to the tree
      for (size_t i = 0; i < n; i++)
	{
	  if (!kdtree_add(t, &unit_test_points[i]))
	    {
	      printf("FAILED -- could not add point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	      kdtree_destroy(t);
	      return;
	    }
	}
    }

  if (add_borders)
    {
      // add points at the corners of the range passed in
      location corners[] = {{sw_lat, sw_lon}, {sw_lat, ne_lon}, {ne_lat, ne_lon}, {ne_lat, sw_lon}};
      for (size_t i = 0; i < 4; i++)
	{
	  if (!kdtree_add(t, &corners[i]))
	    {
	      printf("FAILED -- could not add point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	      kdtree_destroy(t);
	      return;
	    }
	}
    }

  // set up the corners of the range
  location sw = { sw_lat, sw_lon };
  location ne = { ne_lat, ne_lon };

  // use kdtree_range_for_each to print each point in the range
  // (postprocessing in the test script sorts this output)
  kdtree_range_for_each(t, &sw, &ne, unit_print_point_formatted, "%.3f %.3f\n");

  // destroy the tree
  kdtree_destroy(t);
}


void unit_test_range(size_t n, double sw_lat, double sw_lon, double ne_lat, double ne_lon)
{
  // create an empty tree
  kdtree *t = kdtree_create(NULL, 0);

  if (t == NULL)
    {
      printf("FAILED -- could not create tree\n");
      return;
    }
  else
    {
      // add the first n test points to the tree
      for (size_t i = 0; i < n; i++)
	{
	  if (!kdtree_add(t, &unit_test_points[i]))
	    {
	      printf("FAILED -- could not add point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	      kdtree_destroy(t);
	      return;
	    }
	}
    }

  // set up the corners of the range
  location sw = { sw_lat, sw_lon };
  location ne = { ne_lat, ne_lon };

  // get the points in that range
  int count;
  location *pts = kdtree_range(t, &sw, &ne, &count);

  if (pts == NULL && count != 0)
    {
      printf("FAILED -- could not get points in range\n");
      kdtree_destroy(t);
      return;
    }

  // print the points (postprocessing in the test script sorts this output)
  for (size_t i = 0 ; i < count; i++)
    {
      unit_print_point_formatted(&pts[i], "%.2f %.2f\n");
    }
  
  free(pts); // note that free(NULL) is allowed and does nothing
  kdtree_destroy(t);
}


void unit_print_point_formatted(const location *l, void *a)
{
  const char *format = a;

  printf(format, l->lat, l->lon);
}


void unit_test_remove(size_t n, bool readd)
{
  // create an empty tree
  kdtree *t = kdtree_create(NULL, 0);

  if (t == NULL)
    {
      printf("FAILED -- could not create tree\n");
      return;
    }

  // add the first n test points to the tree
  for (size_t i = 0; i < n; i++)
    {
      if (!kdtree_add(t, &unit_test_points[i]))
	{
	  printf("FAILED -- could not add point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }

  // remove the even-indexed points
  for (size_t i = 0; i < n; i += 2)
    {
      kdtree_remove(t, &unit_test_points[i]);
    }

  for (size_t i = 0; i < n; i++)
    {
      if (i % 2 == 1 && !kdtree_contains(t, &unit_test_points[i]))
	{
	  // odd-indexed points should still be in the tree
	  printf("FAILED -- lost point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
      else if (i % 2 == 0 && kdtree_contains(t, &unit_test_points[i]))
	{
	  // even-indexed points should no longer be in the tree
	  printf("FAILED -- still contains point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }

  if (readd)
    {
      // re-add the deleted points
      for (size_t i = 0; i < n; i += 2)
	{
	  if (!kdtree_add(t, &unit_test_points[i]))
	    {
	      printf("FAILED -- could not re-add point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	      kdtree_destroy(t);
	      return;
	    }
	}

      // check that contains finds the re-added points
      for (size_t i = 0; i < n; i++)
	{
	  if (!kdtree_contains(t, &unit_test_points[i]))
	    {
	      printf("FAILED -- lost point %f %f\n", unit_test_points[i].lat, unit_test_points[i].lon);
	      kdtree_destroy(t);
	      return;
	    }
	}
    }

  // destroy the tree
  kdtree_destroy(t);
  printf("PASSED\n");
}


void unit_test_add_time_random(size_t n, int on, double lat_scale, double lon_scale)
{
  // create an array containing n random points
  location *random_points = malloc(sizeof(location) * n);
  for (size_t i = 0; i < n; i++)
    {
      random_points[i].lat = (double)rand() / RAND_MAX * 90.0 * lat_scale;
      random_points[i].lon = (double)rand() / RAND_MAX * 180.0 * lon_scale;
    }

  // create an empty tree
  kdtree *t = kdtree_create(random_points, 0);
  if (t == NULL)
    {
      printf("FAILED -- could not build tree\n");
      return;
    }

  // calling this with on=false allows us to get a baseline instruction
  // count for all the work aside from the adds and contains calls
  if (on)
    {
      // add the random points (tree will be approximately balanced)
      for (size_t i = 0; i < n; i++)
	{
	  kdtree_add(t, &random_points[i]);
	}

      // verify that contains can find the points
      for (size_t i = 0; i < n; i++)
	{
	  if (!kdtree_contains(t, &random_points[i]))
	    {
	      printf("FAILED -- lost point (%f, %f)\n", random_points[i].lat, random_points[i].lon);
	      kdtree_destroy(t);
	      return;
	    }
	}

    }

  // free resources
  kdtree_destroy(t);
  free(random_points);
}


bool unit_not_close(const location *l1, const location *l2, double close)
{
  return fabs(l1->lat - l2->lat) > close && fabs(l1->lon - l2->lon) > close;
}


void unit_test_range_time(size_t n, int on, double lat_scale, double lon_scale)
{
  location no_points[] = {{0.0, 0.0}};
  const double close = 0.000001;
  
  // create n random points
  location *random_points = malloc(sizeof(location) * n);
  for (size_t i = 0; i < n; i++)
    {
      random_points[i].lat = (double)rand() / RAND_MAX * 90.0 * lat_scale;
      random_points[i].lon = (double)rand() / RAND_MAX * 180.0 * lon_scale;
    }

  // create an empty tree (not passing NULL here in case that case isn't
  // handled correcty in kdtree_create)
  kdtree *t = kdtree_create(no_points, 0);
  
  if (t == NULL)
    {
      printf("FAILED -- could not build tree\n");
      return;
    }

  // add the points to the tree (tree will be approximately balanced)
  for (size_t i = 0; i < n; i++)
    {
      // add point only if not close in lat or lon to last point
      // so we know the ranges we use contain only one point
      if (i == n - 1
	  || unit_not_close(random_points + n - 1, random_points + i, close))
	{
	  kdtree_add(t, &random_points[i]);
	}
    }

  // verify that the points were added
  for (size_t i = 0; i < n; i++)
    {
      if ((i == n - 1
	   || unit_not_close(random_points + n - 1, random_points + i, close))
	   
	  && !kdtree_contains(t, &random_points[i]))
	{
	  printf("FAILED -- lost point (%f, %f)\n", random_points[i].lat, random_points[i].lon);
	  kdtree_destroy(t);
	  return;
	}
    }
  
  if (on)
    {
      // call for_each on a skinny wedge containing only the last point
      int count = 0;
      location tall_sw = {-89.99999, random_points[n - 1].lon - close };
      location tall_ne = {89.99999, random_points[n - 1].lon + close };
      location *p1 = kdtree_range(t, &tall_sw, &tall_ne, &count);
      if (count != 1 || p1 == NULL)
	{
	  printf("FAILED -- tall range returned %d points\n", count);
	  free(p1);
	  kdtree_destroy(t);
	  return;
	}
      else if (p1[0].lat != random_points[n - 1].lat || p1[0].lon != random_points[n - 1].lon)
	{
	  printf("FAILED -- tall range returned point %f %f\n", p1[0].lat, p1[0].lon);
	  free(p1);
	  kdtree_destroy(t);
	  return;
	}
      free(p1);	  

      // call for_each on only a skinny horizontal slice containing only
      // the last point
      count = 0;
      location wide_sw = {random_points[n - 1].lat - close, -179.9999 };
      location wide_ne = {random_points[n - 1].lat + close, 179.9999 };
      location *p2 = kdtree_range(t, &wide_sw, &wide_ne, &count);
      if (count != 1 || p2 == NULL)
	{
	  printf("FAILED -- wide range returned %d points\n", count);
	  free(p2);
	  kdtree_destroy(t);
	  return;
	}
      else if (p2[0].lat != random_points[n - 1].lat || p2[0].lon != random_points[n - 1].lon)
	{
	  printf("FAILED -- wide range returned point %f %f\n", p2[0].lat, p2[0].lon);
	  free(p2);
	  kdtree_destroy(t);
	  return;
	}
      free(p2);
    }

  // free resources
  kdtree_destroy(t);
  free(random_points);
}
