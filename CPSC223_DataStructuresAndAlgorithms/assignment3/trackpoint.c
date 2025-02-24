#include <stdlib.h>

#include "trackpoint.h"

struct trackpoint
{
  location loc;
  long time;
};

trackpoint *trackpoint_create(double lat, double lon, long time)
{
  if (lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon < 180.0)
    {
      trackpoint *pt = malloc(sizeof(trackpoint));
      if (pt != NULL)
	{
	  pt->loc.lat = lat;
	  pt->loc.lon = lon;
	  pt->time = time;
	}
      return pt;
    }
  else
    {
      return NULL;
    }
}

trackpoint *trackpoint_copy(const trackpoint *pt)
{
  return trackpoint_create(pt->loc.lat, pt->loc.lon, pt->time);
}

void trackpoint_destroy(trackpoint *pt)
{
  free(pt);
}

location trackpoint_location(const trackpoint *pt)
{
  return pt->loc;
}

long trackpoint_time(const trackpoint *pt)
{
  return pt->time;
}
