#include "dijkstra.h"
#include "graph_utils.h"
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
// Initialisation des distances et parents
void initialization(vertex_t s, struct graph_t *g, vertex_t *parents,
                    unsigned int *d) {
  for (vertex_t i = 0; i < g->num_vertices; i++) {
    d[i] = UINT_MAX;
    parents[i] = UINT_MAX;
  }
  d[s] = 0;
}

// Ensemble des sommets non visités
void vertices_set(int *vertices, unsigned int n) {
  for (unsigned int i = 0; i < n; i++) {
    vertices[i] = i;
  }
}

// Extrait le sommet avec la plus petite distance
vertex_t extractMin(int *nonBlack, unsigned int *d, int len) {
  unsigned int min = UINT_MAX;
  int index = -1;
  for (int i = 0; i < len; i++) {
    if ((unsigned int)nonBlack[i] != UINT_MAX && d[nonBlack[i]] < min) {
      min = d[nonBlack[i]];
      index = i;
    }
  }
  if (index == -1)
    return UINT_MAX;

  vertex_t res = nonBlack[index];
  nonBlack[index] = UINT_MAX;
  return res;
}

// Relâchement des arcs
void relacherArc(vertex_t x, vertex_t y, unsigned int *d, vertex_t *parents) {
  if (d[x] != UINT_MAX && d[x] + 1 < d[y]) {
    d[y] = d[x] + 1;
    parents[y] = x;
  }
}
struct paths_t paths(vertex_t s, struct graph_t *g) {
  struct paths_t paths = {0};
  paths.w = malloc(g->num_objectives * sizeof(struct way_t));
  unsigned int d[g->num_vertices];
  vertex_t parents[g->num_vertices];
  dijkstra(s, g, d, parents);

  for (unsigned int k = 0; k < g->num_objectives; k++) {
    vertex_t obj = g->objectives[k];
    paths.w[k].len = 0;
    paths.w[k].sommets = malloc(g->num_vertices * sizeof(vertex_t));
    for (unsigned int i = 0; i < g->num_vertices; i++) {
      paths.w[k].sommets[i] = 0;
    }
    vertex_t current = obj;
    while (current != UINT_MAX && current != s) {

      if (paths.w[k].len >= g->num_vertices) {

        vertex_t *tmp = realloc(paths.w[k].sommets,
                                (paths.w[k].len + 1) * sizeof(vertex_t));
        paths.w[k].sommets = tmp;
      }
      paths.w[k].sommets[paths.w[k].len++] = current;
      current = parents[current];
    }
    if (current == s) {
      paths.w[k].sommets[paths.w[k].len++] = s;
    } else {
      paths.w[k].len = 0;
    }
  }
  return paths;
}

void sort_paths(struct paths_t paths, struct graph_t *g) {
  for (unsigned int i = 0; i < g->num_objectives; i++) {
    for (unsigned int j = i + 1; j < g->num_objectives; j++) {
      if (paths.w[i].len > paths.w[j].len) {
        struct way_t tmp = paths.w[i];
        paths.w[i] = paths.w[j];
        paths.w[j] = tmp;
      }
    }
  }
}

int shortest_path(struct graph_t *g, struct paths_t *sorted_paths) {
  struct way_t *w = sorted_paths->w;
  unsigned int idx = UINT_MAX;
  for (unsigned int k = 0; k < g->num_objectives; k++) {
    if (w[k].len > 1 && idx == UINT_MAX) {
      idx = k;
      break;
    }
  }
  if (idx == UINT_MAX) {
    for (unsigned int k = 0; k < g->num_objectives; k++) {
      free(w[k].sommets);
    }
  } else {
    for (unsigned int j = 0; j < w[idx].len / 2; j++) {
      unsigned int tmp = w[idx].sommets[j];
      w[idx].sommets[j] = w[idx].sommets[w[idx].len - 1 - j];
      w[idx].sommets[w[idx].len - 1 - j] = tmp;
    }
  }
  return idx;
}

int is_valid_path(struct way_t w) {
  if (w.len > 1)
    return 1;
  return 0;
}

void free_paths(struct paths_t *paths) { free(paths->w); }

// Algorithme de Dijkstra
void dijkstra(vertex_t s, struct graph_t *g, unsigned int *d,
              vertex_t *parents) {
  initialization(s, g, parents, d);
  int nonBlack[g->num_vertices];
  vertices_set(nonBlack, g->num_vertices);

  for (unsigned int i = 0; i < g->num_vertices; i++) {
    vertex_t x = extractMin(nonBlack, d, g->num_vertices);
    if (x == UINT_MAX)
      break;

    for (vertex_t y = 0; y < g->num_vertices; y++) {
      enum dir_t dir = gsl_spmatrix_uint_get(g->t, x, y);
      if (dir != NO_EDGE && dir != WALL_DIR) {
        relacherArc(x, y, d, parents);
      }
    }
  }
}

struct way_t shortest_path2(vertex_t pos, struct graph_t *graph,
                            vertex_t *objectives_visited) {
  struct way_t way = {.len = 0, .sommets = NULL};

  if (!graph || !graph->t || !objectives_visited) {
    fprintf(stderr, "Paramètres invalides dans shortest_path2\n");
    return way;
  }

  unsigned int n = graph->num_vertices;
  unsigned int *distance = malloc(n * sizeof(unsigned int));
  vertex_t *predecessor = malloc(n * sizeof(vertex_t));
  bool *visited = calloc(n, sizeof(bool));

  if (!distance || !predecessor || !visited) {
    fprintf(stderr, " Échec malloc dans shortest_path2\n");
    free(distance);
    free(predecessor);
    free(visited);
    return way;
  }

  for (vertex_t v = 0; v < n; v++) {
    distance[v] = UINT_MAX;
    predecessor[v] = UINT_MAX;
  }
  distance[pos] = 0;

  for (vertex_t i = 0; i < n; i++) {
    vertex_t u = UINT_MAX;
    unsigned int min_dist = UINT_MAX;
    for (vertex_t v = 0; v < n; v++) {
      if (!visited[v] && distance[v] < min_dist) {
        min_dist = distance[v];
        u = v;
      }
    }

    if (u == UINT_MAX)
      break;

    visited[u] = true;

    for (int idx = graph->t->p[u]; idx < graph->t->p[u + 1]; idx++) {
      vertex_t v = graph->t->i[idx];
      unsigned int edge_value = graph->t->data[idx];

      if (edge_value != WALL_DIR && edge_value != NO_EDGE) {
        unsigned int alt = distance[u] + 1;
        if (alt < distance[v]) {
          distance[v] = alt;
          predecessor[v] = u;
        }
      }
    }
  }

  vertex_t nearest_obj = UINT_MAX;
  unsigned int min_obj_dist = UINT_MAX;

  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    vertex_t obj = graph->objectives[i];
    if (obj >= graph->num_vertices)
      continue;

    if (objectives_visited[i] == 0 && distance[obj] < min_obj_dist) {
      min_obj_dist = distance[obj];
      nearest_obj = obj;
    }
  }

  if (nearest_obj != UINT_MAX && predecessor[nearest_obj] != UINT_MAX) {
    vertex_t curr = nearest_obj;
    int path_len = 1;
    while (predecessor[curr] != UINT_MAX && curr != pos) {
      curr = predecessor[curr];
      path_len++;
    }

    way.len = path_len;
    way.sommets = malloc(path_len * sizeof(vertex_t));
    if (!way.sommets) {
      way.len = 0;
    } else {
      curr = nearest_obj;
      for (int i = path_len - 1; i >= 0; i--) {
        way.sommets[i] = curr;
        curr = predecessor[curr];
      }
      way.sommets[0] = pos;
    }
  }

  free(distance);
  free(predecessor);
  free(visited);

  return way;
}

struct way_t shortest_path_between(vertex_t start, vertex_t goal,
                                   struct graph_t *graph) {
  struct way_t way = {.len = 0, .sommets = NULL};

  if (!graph || !graph->t || start >= graph->num_vertices ||
      goal >= graph->num_vertices)
    return way;

  unsigned int n = graph->num_vertices;
  unsigned int *distance = malloc(n * sizeof(unsigned int));
  vertex_t *predecessor = malloc(n * sizeof(vertex_t));
  bool *visited = calloc(n, sizeof(bool));

  if (!distance || !predecessor || !visited) {
    free(distance);
    free(predecessor);
    free(visited);
    return way;
  }

  for (vertex_t v = 0; v < n; v++) {
    distance[v] = UINT_MAX;
    predecessor[v] = UINT_MAX;
  }
  distance[start] = 0;

  for (vertex_t i = 0; i < n; i++) {
    vertex_t u = UINT_MAX;
    unsigned int min_dist = UINT_MAX;
    for (vertex_t v = 0; v < n; v++) {
      if (!visited[v] && distance[v] < min_dist) {
        min_dist = distance[v];
        u = v;
      }
    }

    if (u == UINT_MAX)
      break;

    visited[u] = true;

    for (int idx = graph->t->p[u]; idx < graph->t->p[u + 1]; idx++) {
      vertex_t v = graph->t->i[idx];
      unsigned int edge_value = graph->t->data[idx];

      if (edge_value != WALL_DIR && edge_value != NO_EDGE) {
        unsigned int alt = distance[u] + 1;
        if (alt < distance[v]) {
          distance[v] = alt;
          predecessor[v] = u;
        }
      }
    }
  }

  if (distance[goal] != UINT_MAX && predecessor[goal] != UINT_MAX) {
    vertex_t curr = goal;
    int path_len = 1;
    while (predecessor[curr] != UINT_MAX && curr != start) {
      curr = predecessor[curr];
      path_len++;
    }

    way.len = path_len;
    way.sommets = malloc(path_len * sizeof(vertex_t));
    if (!way.sommets) {
      way.len = 0;
    } else {
      curr = goal;
      for (int i = path_len - 1; i >= 0; i--) {
        way.sommets[i] = curr;
        curr = predecessor[curr];
      }
      way.sommets[0] = start;
    }
  }

  free(distance);
  free(predecessor);
  free(visited);

  return way;
}
