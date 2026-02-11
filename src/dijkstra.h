#ifndef _CORS_DIJKSTRA_H_
#define _CORS_DIJKSTRA_H_
#include "graph.h"
#include "move.h"

struct way_t {
  vertex_t *sommets;
  unsigned int len;
};

struct paths_t {
  struct way_t *w;
};

void initialization(vertex_t s, struct graph_t *g, vertex_t *parents,
                    unsigned int *d);

void vertices_set(int *vertices, unsigned int n);

vertex_t extractMin(int *nonBlack, unsigned int *d, int len);

void dijkstra(vertex_t s, struct graph_t *g, unsigned int *d,
              vertex_t *parents);

struct paths_t paths(vertex_t s, struct graph_t *g);

void sort_paths(struct paths_t paths, struct graph_t *g);

int shortest_path(struct graph_t *g, struct paths_t *paths);

int is_valid_path(struct way_t w);

void free_paths(struct paths_t *paths);
struct way_t shortest_path2(vertex_t pos, struct graph_t *graph,
                            vertex_t *objectives_visited);
struct way_t shortest_path_between(vertex_t start, vertex_t goal,
                                   struct graph_t *graph);

#endif
