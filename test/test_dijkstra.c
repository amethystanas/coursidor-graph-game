#include "test_dijkstra.h"
#include "../src/dijkstra.h"
#include "../src/graph_utils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_tab(unsigned int *t, int len) {
  printf("[");
  for (int i = 0; i < len; i++) {
    printf(" %u ", t[i]);
  }
  printf("]\n");
}
void print_path(struct way_t *w) {
  for (unsigned int i = 0; i < w->len; i++) {
    printf("-> %u ", w->sommets[i]);
  }
  printf(" Sa longueur: %d\n", w->len);
}

void test_initialization() {
  unsigned int m = 5;
  struct graph_t *g = make_graph_type_T(m);
  unsigned int d[g->num_vertices];
  unsigned int parents[g->num_vertices];
  unsigned int s = 0;

  g->objectives = malloc(sizeof(vertex_t) * g->num_objectives);
  for (unsigned int i = 0; i < g->num_objectives; ++i) {
    g->objectives[i] = rand() % g->num_vertices;
  }

  initialization(s, g, parents, d);
  assert(d[s] == 0);
  assert(parents[s] == 0);
  for (vertex_t i = 1; i < g->num_vertices; i++) {
    assert(d[i] == UINT_MAX);
    assert(parents[i] == 0);
  }
  printf("test_initialisation passed");
  free_graph(g);
}

void test_extractMin(void) {
  printf("%s\n", __func__);
  int num_vertices = 6;
  int nonBlack[num_vertices];
  vertices_set(nonBlack, num_vertices);
  unsigned int *d = malloc(num_vertices * sizeof(unsigned int));
  d[0] = 0;
  d[1] = 1;
  d[2] = UINT_MAX;
  d[3] = 4;
  d[4] = 2;
  d[5] = 3;
  vertex_t x = extractMin(nonBlack, d, num_vertices);
  printf("%u\n", x);
  free(d);
}

void test_dijkstra(void) {
  printf("%s\n", __func__);
  srand(time(NULL));
  unsigned int m = 5;
  struct graph_t *g = make_graph_type_T(m);
  g->num_objectives = 3;
  g->objectives = malloc(g->num_objectives * sizeof(vertex_t));

  for (unsigned int i = 0; i < g->num_objectives; ++i) {
    g->objectives[i] = rand() % g->num_vertices;
  }

  printf("Objectifs : ");
  for (unsigned int i = 0; i < g->num_objectives; ++i) {
    printf("%u ", g->objectives[i]);
  }
  printf("\n");

  unsigned int d[g->num_vertices];
  vertex_t parents[g->num_vertices];

  vertex_t start = 0;

  dijkstra(start, g, d, parents);
  print_tab(d, g->num_vertices);
  print_tab(parents, g->num_vertices);
  free_graph(g);
}

void test_shortest_path() {
  printf("%s\n", __func__);
  unsigned int m = 2;
  // unsigned int NUM_VERTICES= 3 * m * m - 3 * m + 1;
  struct graph_t *g1 = make_graph_type_T(m);
  struct graph_t *g2 = make_graph_type_T(m);
  starting_graph_random(g1, g2);
  printf("Point de départ : ");
  printf("%u\n", g1->start[BLACK]);
  goal_graph_random(g1, g2);
  printf("Objectifs : ");
  for (unsigned int i = 0; i < g1->num_objectives; ++i) {
    printf("%u ", g1->objectives[i]);
  }
  printf("\n");
  unsigned int d[g1->num_vertices];
  vertex_t parents[g1->num_vertices];
  dijkstra(g1->start[BLACK], g1, d, parents);
  struct paths_t paths_tab = paths(g1->start[BLACK], g1);
  sort_paths(paths_tab, g1);
  for (unsigned int i = 0; i < g1->num_objectives; i++) {
    if (is_valid_path(paths_tab.w[i]))
      print_path(&paths_tab.w[i]);
  }
  int idx = shortest_path(g1, &paths_tab);
  printf("Longueur du chemin du court chemin: %d\n", paths_tab.w[idx].len);
  for (unsigned int i = 0; i < g1->num_objectives; i++) {
    if (paths_tab.w[i].sommets != NULL) {
      free(paths_tab.w[i].sommets);
    }
  }
  free_paths(&paths_tab);
  free_graph(g1);
  free_graph(g2);
}
