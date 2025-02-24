#include <stdlib.h>
#include <stdbool.h>

#include "ldigraph.h"
#include "queue.h"

struct ldigraph
{
  size_t n;          // the number of vertices
  size_t *list_size; // the size of each adjacency list
  size_t *list_cap;  // the capacity of each adjacency list
  int **adj;      // the adjacency lists
};

typedef struct
{
  const ldigraph *g; // the graph that was searched
  int from; // the starting vertex of the search (if applicable)
  int *color; // current status of each vertex (using enum below)
  int *dist; // number of edges on the path that was found to each vertex
  int *pred; // predecessor along the path that was found (won't be needed)
  int *finish; // vertices that are marked DONE in reverse order if finish
  int count; // the number of vertices that have been found so far
  int max_count; // the max path length to target node (applies to bruteforce DFS)
  bool found_cycle;  // a flag indicating whether we have seen a cycle
  // YOU CAN ADD MORE THINGS HERE!
} ldigraph_search;

enum {LDIGRAPH_UNSEEN, LDIGRAPH_PROCESSING, LDIGRAPH_DONE};

#define LDIGRAPH_ADJ_LIST_INITIAL_CAPACITY 4

int ldigraph_dfs_bruteforce(const ldigraph *g, int from, int target);
void ldigraph_dfs_visit_brute(const ldigraph* g, ldigraph_search *s, int from, int target);
 
/**
 * Returns the result of running breadth-first search on the given
 * graph starting with the given vertex.  When the search arrives
 * at a vertex, its neighbors are considered in the order the
 * corresponding edges were added to the graph.
 * It is the caller's responsibility to destroy to result.
 *
 * @param g a pointer to a directed graph, non-NULL
 * @param from the index of a vertex in the given graph
 * @return the result of the search
 */
static ldigraph_search *ldigraph_bfs(const ldigraph *g, int from);


/**
 * Returns the result of running depth-first search on the given
 * graph starting with the given vertex.  When the search arrives
 * at a vertex, its neighbors are considered in the order the
 * corresponding edges were added to the graph.
 * It is the caller's responsibility to destroy to result.
 *
 * @param g a pointer to a directed graph, non-NULL
 * @param from the index of a vertex in the given graph
 * @return the result of the search
 */
static ldigraph_search *ldigraph_dfs(const ldigraph *g, int from);


/**
 * Returns the result of running depth-first search on the given
 * graph.  Each connected component is searched from an arbitrarily
 * selected starting point.  When the search arrives
 * at a vertex, its neighbors are considered in the order the
 * corresponding edges were added to the graph.
 * It is the caller's responsibility to destroy to result.
 *
 * @param g a pointer to a directed graph, non-NULL
 * @return the result of the search
 */
static ldigraph_search *ldigraph_dfs_with_restart(const ldigraph *g);

/**
 * Visits the given vertex in the given search of the given graph.
 *
 * @param g a pointer to a directed graph
 * @param s a search in that graph
 * @param from a vertex in that graph
 */
static void ldigraph_dfs_visit(const ldigraph* g, ldigraph_search *s, int from);


/**
 * Resizes the adjacency list for the given vertex in the given graph.
 * 
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 */
static void ldigraph_list_embiggen(ldigraph *g, int from);


/**
 * Prepares a search result for the given graph starting from the given
 * vertex.  It is the responsibility of the caller to destroy the result.
 *
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 * @return a pointer to a search result
 */
static ldigraph_search *ldigraph_search_create(const ldigraph *g, int from);


/**
 * Initializes a search result that has its associated graph set and
 * has space allocated for its arrays.
 *
 * @param s a pointer to a search result, non-NULL
 * @param from the index of a vertex in the graph s holds a search result from
 */
static void ldigraph_search_init(ldigraph_search *s, int from);


/**
 * Destroys the given search result.
 *
 * @param s a pointer to a search result, non-NULL
 */
static void ldigraph_search_destroy(ldigraph_search *s);


ldigraph *ldigraph_create(size_t n)
{
  if (n < 1)
    {
      return NULL;
    }
  
  ldigraph *g = malloc(sizeof(ldigraph));
  if (g != NULL)
    {
      g->n = n;
      g->list_size = malloc(sizeof(size_t) * n);
      g->list_cap = malloc(sizeof(size_t) * n);
      g->adj = malloc(sizeof(int *) * n);
      
      if (g->list_size == NULL || g->list_cap == NULL || g->adj == NULL)
	{
	  free(g->list_size);
	  free(g->list_cap);
	  free(g->adj);
	  free(g);

	  return NULL;
	}

      for (int i = 0; i < n; i++)
	{
	  g->list_size[i] = 0;
	  g->adj[i] = malloc(sizeof(int) * LDIGRAPH_ADJ_LIST_INITIAL_CAPACITY);
	  g->list_cap[i] = g->adj[i] != NULL ? LDIGRAPH_ADJ_LIST_INITIAL_CAPACITY : 0;
	}
    }

  return g;
}


size_t ldigraph_size(const ldigraph *g)
{
  if (g != NULL)
    {
      return g->n;
    }
  else
    {
      return 0;
    }
}


void ldigraph_list_embiggen(ldigraph *g, int from)
{
  if (g->list_cap[from] != 0)
    {
      g->adj[from] = realloc(g->adj[from], sizeof(int*) * g->list_cap[from] * 2);
      g->list_cap[from] = g->adj[from] != NULL ? g->list_cap[from] * 2 : 0;
    }
}


void ldigraph_add_edge(ldigraph *g, int from, int to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      // make room if necessary
      if (g->list_size[from] == g->list_cap[from])
	{
	  ldigraph_list_embiggen(g, from);
	}

      // add to end of array if there is room
      if (g->list_size[from] < g->list_cap[from])
	{
	  g->adj[from][g->list_size[from]++] = to;
	}
    }
}


bool ldigraph_has_edge(const ldigraph *g, int from, int to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      // sequential search of from's adjacency list
      int i = 0;
      while (i < g->list_size[from] && g->adj[from][i] != to)
	{
	  i++;
	}
      return i < g->list_size[from];
    }
  else
    {
      return false;
    }
}


int ldigraph_shortest_path(const ldigraph *g, int from, int to)
{
  if (g == NULL || from < 0 || from >= g->n || to < 0 || to >= g->n)
    {
      return -1;
    }

  // do BFS starting from the from vertex
  ldigraph_search *s = ldigraph_bfs(g, from);

  if (s != NULL)
    {
      // look up the distance to the to vertex in the result and return it
      int shortest = s->dist[to];
      ldigraph_search_destroy(s);
      return shortest;
    }
  else
    {
      return -1;
    }
}


// caller takes responsibility of returned search structure
ldigraph_search *ldigraph_bfs(const ldigraph *g, int from)
{
  ldigraph_search* search = ldigraph_search_create(g, from);
  queue* q = queueCreate();   // stores nodes being processed
  int u, v;
  size_t num_out_neighbors;

  search->dist[from] = 0;
  
  enq(q, from);
  while (!queueEmpty(q))
  {
    u = deq(q);
    num_out_neighbors = g->list_size[u];
    
    for (size_t i = 0; i < num_out_neighbors; i++)
    {
      v = g->adj[u][i];   // neighbor
      if (search->color[v] == LDIGRAPH_UNSEEN)
      {
        enq(q, v);
        search->color[v] = LDIGRAPH_PROCESSING;
        search->pred[v] = u;
        search->dist[v] = search->dist[u] + 1;
        
      }
      search->color[u] = LDIGRAPH_DONE;
    }
  }

  queueDestroy(q);
  return search;
}


int ldigraph_longest_path(const ldigraph *g, int from, int to)
{
  if (g == NULL || from < 0 || from >= g->n || to < 0 || to >= g->n)
    {
      return -1;
    }

  // do a DFS to find cycles and do topological sort if none

  // do full DFS, recording vertices in reverse order of finish time
  ldigraph_search *s = ldigraph_dfs_with_restart(g);
  
  if (!s->found_cycle)
    {
      int i; // index in finish array
      int u; // current node
      int v; // out-neighbor node
      int max;

      // iterate backwards through topo sort until "to" node is found
      // set distance value to "-1" to indicate no path
      for (i = (g->n - 1); s->finish[i] != to; i--) {
        if (i == 0) return -1; // from node not found. no path exists
        s->dist[s->finish[i]] = -1;
      }
      
      // "to" node has distance 0 to itself
      s->dist[s->finish[i]] = 0;

      // iterate through remainder of finish array until "from" node
      while (s->finish[i] != from) {
        i--;
        assert(i >= 0);

        v = s->finish[i];

        // for each out-neighbor, v, of u, checks its distance to "to". If no
        // valid path exists, store -1 in distances for v. otherwise store max distance + 1
        max = -1;
        for (int j = 0; j < g->list_size[v]; j++)
        {
          u = g->adj[v][j];

          int curr_max = s->dist[u];
          if (curr_max > max) max = curr_max;
        }

        if (max == -1) s->dist[v] = -1;   // no valid path found
        else s->dist[v] = max + 1;        // valid path found
      }

      int result = s->dist[from]; // this should be -1 if no path
      ldigraph_search_destroy(s);
      return result;
    } else 
    {
      ldigraph_search_destroy(s);
      return ldigraph_dfs_bruteforce(g, from, to);
    }
}


ldigraph_search *ldigraph_dfs(const ldigraph *g, int from)
{
  if (g == NULL || from < 0 || from >= g->n)
    {
      return NULL;
    }

  ldigraph_search *s = ldigraph_search_create(g, from);
  if (s != NULL)
    {
      // start at from
      // (note we do not have the restart-if-some-vertices-unvisited
      // loop here; consider whether you will need it)
      s->dist[from] = 0;
      ldigraph_dfs_visit(g, s, from);
    }
  return s;
}

int ldigraph_dfs_bruteforce(const ldigraph *g, int from, int target)
{
  int result;
  if (g == NULL) return -1;

  ldigraph_search *s = ldigraph_search_create(g, from);
  ldigraph_dfs_visit_brute(g, s, from, target);
  result = s->max_count;
  
  ldigraph_search_destroy(s);
  
  return result;
} 

// helper for dfs visit for brute force search
void ldigraph_dfs_visit_brute(const ldigraph* g, ldigraph_search *s, int from, int target)
{
  if (from == target) {
    if (s->count > s->max_count) s->max_count = s->count;  
    return;
  }

  // make alias for adjacency list for from vertex
  const int *neighbors = g->adj[from];
  s->color[from] = LDIGRAPH_DONE;
  s->count++;

  // iterate over outgoing edges
  for (int i = 0; i < g->list_size[from]; i++)
    {
      int to = neighbors[i];
      if (s->color[to] != LDIGRAPH_DONE)
        {
          // found an edge to a new vertex -- explore it
          ldigraph_dfs_visit_brute(g, s, to, target);
        }
    }
  
  // mark and record current vertex finished
  s->color[from] = LDIGRAPH_UNSEEN;
  s->count--;
}

ldigraph_search *ldigraph_dfs_with_restart(const ldigraph *g)
{
  if (g == NULL)
    {
      return NULL;
    }

  ldigraph_search *s = ldigraph_search_create(g, 0);
  if (s != NULL)
    {
      // try all starting points for DFS
      for (int from = 0; from < g->n; from++)
	{
	  // use from as a starting point if no previous search found it
	  if (s->color[from] == LDIGRAPH_UNSEEN)
	    {
	      s->dist[from] = 0;
	      ldigraph_dfs_visit(g, s, from);
	    }
	}
    }
  return s;
}


void ldigraph_dfs_visit(const ldigraph* g, ldigraph_search *s, int from)
{
  s->color[from] = LDIGRAPH_PROCESSING;

  // make alias for adjacency list for from vertex
  const int *neighbors = g->adj[from];
  
  // iterate over outgoing edges
  for (int i = 0; i < g->list_size[from]; i++)
    {
      int to = neighbors[i];
      if (s->color[to] == LDIGRAPH_UNSEEN)
	{
	  // found an edge to a new vertex -- explore it
	  s->dist[to] = s->dist[from] + 1;
	  s->pred[to] = from;
	  
	  ldigraph_dfs_visit(g, s, to);
	}
      else if (s->color[to] == LDIGRAPH_PROCESSING)
	{
	  // edge from current vertex to a still active vertex forms a cycle
	  s->found_cycle = true;
	}
    }
  
  // mark and record current vertex finished
  s->color[from] = LDIGRAPH_DONE;
  s->finish[g->n - s->count - 1] = from;
  s->count++;
}


void ldigraph_destroy(ldigraph *g)
{
  if (g != NULL)
    {
      for (int i = 0; i < g->n; i++)
	{
	  free(g->adj[i]);
	}
      free(g->adj);
      free(g->list_cap);
      free(g->list_size);
      free(g);
    }
}


ldigraph_search *ldigraph_search_create(const ldigraph *g, int from)
{
  if (g != NULL && from >= 0 && from < g->n)
    {
      ldigraph_search *s = malloc(sizeof(ldigraph_search));
      
      if (s != NULL)
	{
	  s->g = g;
	  s->color = malloc(sizeof(int) * g->n);
	  s->dist = malloc(sizeof(int) * g->n);
	  s->pred = malloc(sizeof(int) * g->n);
	  s->finish = malloc(sizeof(int) * g->n);

	  if (s->color != NULL && s->dist != NULL && s->pred != NULL)
	    {
	      ldigraph_search_init(s, from);
	    }
	  else
	    {
	      free(s->pred);
	      free(s->dist);
	      free(s->color);
	      free(s);
	      return NULL;
	    }
	}

      return s;
    }
  else
    {
      return NULL;
    }
}


void ldigraph_search_init(ldigraph_search *s, int from)
{
  // set from vertex
  s->from = from;
  
  // initialize all vertices to unseen
  for (int i = 0; i < s->g->n; i++)
    {
      s->color[i] = LDIGRAPH_UNSEEN;
      s->dist[i] = -1; // -1 for no path yet
      s->pred[i] = -1; // no predecessor yet
    }

  // other bookkeeping
  s->found_cycle = false;
  s->count = 0;
  s->max_count = -1;

}


void ldigraph_search_destroy(ldigraph_search *s)
{
  if (s != NULL)
    {
      free(s->color);
      free(s->dist);
      free(s->pred);
      free(s->finish);
      free(s);
    }
}
