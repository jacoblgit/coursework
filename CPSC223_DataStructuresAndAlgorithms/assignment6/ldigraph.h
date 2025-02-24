#ifndef __LDIGRAPH_H__
#define __LDIGRAPH_H__

#include <stdlib.h>
#include <stdbool.h>

typedef struct ldigraph ldigraph;

/**
 * Creates a new directed graph with the given number of vertices.  The
 * vertices will be numbered 0, ..., n-1.
 *
 * @param n a positive integer
 * @return a pointer to the new graph
 */
ldigraph *ldigraph_create(size_t n);


/**
 * Returns the number of vertices in the given graph.
 *
 * @param g a pointer to a directed graph
 * @return the number of vertices in that graph
 */
size_t ldigraph_size(const ldigraph *g);


/**
 * Adds the given directed edge to this graph.  The edge must
 * not already be present in the graph.
 *
 * @param g a pointer to a directed graph, non-NULL
 * @param from a valid vertex index in g
 * @param to a valid vertex index in g, not equal to from
 */
void ldigraph_add_edge(ldigraph *g, int from, int to);


/**
 * Determines if the given graph contains an edge from the given
 * from vertex to the given to vertex.
 *
 * @param g a pointer to a directed graph, non-NULL
 * @param from a valid vertex index in g
 * @param to a valid vertex index in g, not equal to from
 * @return true if and only if the edge exists
 */
bool ldigraph_has_edge(const ldigraph *g, int from, int to);


/**
 * Returns the length of the shortest path from the given vertex
 * to the given vertex.  If there is no path then the return value
 * is -1.
 *
 * @param g a pointer to a directed graph, non-NULL
 * @param from a valid vertex index in g
 * @param to a valid vertex index in g
 * @return the length of the shortest path, or -1
 */
int ldigraph_shortest_path(const ldigraph *g, int from, int to);


/**
 * Returns the length of the longest simple path from the given vertex
 * to the given vertex.  If there is no path then the return value
 * is -1.
 *
 * @param g a pointer to a directed graph, non-NULL
 * @param from a valid vertex index in g
 * @param to a valid vertex index in g
 * @return the length of the longest simple path, or -1
 */
int ldigraph_longest_path(const ldigraph *g, int from, int to);


/**
 * Destroys the given directed graph.
 *
 * @param g a pointer to a directed graph, non-NULL
 */
void ldigraph_destroy(ldigraph *g);

#endif
