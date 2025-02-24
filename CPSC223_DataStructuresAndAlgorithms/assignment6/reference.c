#include <stdlib.h>
#include <stdbool.h>

#include "ldigraph.h"

typedef struct
{
  const ldigraph *g; // the graph that was searched
  int from; // the starting vertex of the search (if applicable)
  int *color; // current status of each vertex (using enum below)
  int *dist; // number of edges on the path that was found to each vertex
  int *pred; // predecessor along the path that was found (won't be needed)
  int *finish; // vertices that are marked DONE in reverse order if finish
  int count; // the number of vertices that have been found so far
  bool found_cycle;  // a flag indicating whether we have seen a cycle
  // YOU CAN ADD MORE THINGS HERE!
} ldigraph_search;

struct ldigraph
{
  size_t n;          // the number of vertices
  size_t *list_size; // the size of each adjacency list
  size_t *list_cap;  // the capacity of each adjacency list
  int **adj;      // the adjacency lists
};

enum {LDIGRAPH_UNSEEN, LDIGRAPH_PROCESSING, LDIGRAPH_DONE};