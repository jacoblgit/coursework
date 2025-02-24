#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ldigraph.h"

/**
 * Reads and returns the graph contained in the given file.
 * Returns NULL if the file could not be read or if the
 * graph could not be created.
 *
 * @param fname the name of the file containing the graph
 * @return a pointer to the graph build
 */
ldigraph *read_graph(const char *fname);


/**
 * Prints the given array.
 *
 * @param a pointer to an array of ints, non-NULL
 * @param n the length of that array
 */
void print_array(const int *a, size_t n);


/**
 * Returns a pointer to the graph path finding function specified by the
 * given string.  The string may be "-shortest" or "-longest"
 * to specify finding the shortest or longest path respectively.
 *
 * @param s a string, non-NULL
 * @return a pointer to the corresponding function
 */
int (*determine_method(const char *s))(const ldigraph*, int, int);


int main(int argc, char **argv)
{
  if (argc < 2)
    {
      fprintf(stderr, "USAGE: %s filename [[method from to...]...]\n", argv[0]);
      return 1;
    }

  ldigraph *g;
  if (strcmp(argv[1], "-timing") == 0)
    {
      int size;
      if (argc < 4 || (size = atoi(argv[argc - 2])) <= 0)
	{
	  fprintf(stderr, "USAGE: %s -timing [[method from to...]...] size on/off\n", argv[0]);
	  return 1;
	}
      int on = atoi(argv[argc - 1]);
      
      // make a sparse graph for timing -shortest and -longest on acyclic
      g = ldigraph_create(size);

      if (g == NULL)
	{
	  return 1;
	}

      ldigraph_add_edge(g, 0, 2);
      ldigraph_add_edge(g, size - 1, 1);
      
      for (int u = 2; u < size - 1; u++)
	{
	  int dest[3];
	  int dest_count = 0;
	  dest[dest_count++] = u + 1;
	  for (int k = 2; k <= 3; k++)
	    {
	      if (u < size - k)
		{
		  dest[dest_count++] = u + k;
		}
	    }

	  // shuffle order of edges
	  int swap = u % 3;
	  if (swap > 0 && swap < dest_count)
	    {
	      int temp = dest[0];
	      dest[0] = dest[swap];
	      dest[swap] = temp;
	    }

	  // add edges
	  for (int i = 0; i < dest_count; i++)
	    {
	      ldigraph_add_edge(g, u, dest[i]);
	    }
	}
      
      if (on != 1)
	{
	  // skip the rest for baseline test
	  return 0;
	}
      
      // ignore last two arguments (size and on/off)
      argc -= 2;
    }
  else
    {
      // read graph from file
      g = read_graph(argv[1]);
    }

  if (g != NULL)
    {
      size_t a = 2;
      while (a + 2 < argc)
	{
	  // determine search method
	  int (*find_path)(const ldigraph *, int, int) = determine_method(argv[a]);

	  if (find_path != NULL)
	    {
	      // get from vertex
	      int from = atoi(argv[a + 1]);
	      int to = atoi(argv[a + 2]);

	      // check whether vertices are legal
	      if (from >= 0 && from < ldigraph_size(g) && to >= 0 && to < ldigraph_size(g))
		{
		  // do the search 
		  int length = find_path(g, from, to);

		  // print answer
		  printf("%9.9s: %3d ~> %3d: %d\n", argv[a], from, to, length);
		}
	    }
	  
	  // go on to next method
	  a += 3;
	}
      
      ldigraph_destroy(g);
    }

  return 0;
}


ldigraph *read_graph(const char *fname)
{
  FILE *in = fopen(fname, "r");
  ldigraph *g = NULL;

  if (in != NULL)
    {
      size_t size;
      if (fscanf(in, "%zu", &size) == 1)
	{
	  g = ldigraph_create(size);

	  if (g != NULL)
	    {
	      int from, to;
	      while (fscanf(in, "%d %d", &from, &to) == 2)
		{
		  if (from >= 0 && from < size && to >= 0 && to < size)
		    {
		      ldigraph_add_edge(g, from, to);
		    }
		}
	    }
	}
      
      fclose(in);
    }

  return g;
}

int (*determine_method(const char *s))(const ldigraph*, int, int)
{
  if (strcmp(s, "-shortest") == 0)
    {
      return ldigraph_shortest_path;
    }
  else if (strcmp(s, "-longest") == 0)
    {
      return ldigraph_longest_path;
    }
  else
    {
      return NULL;
    }
}
