#include "player_mouvement.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Déplacer le pion dans la même direction jusqu’à 3 cases. Si ça échoue
// retourner -1
vertex_t move_in_same_direction(vertex_t start, enum dir_t last_direction,
                                int max_steps, struct graph_t *graph,
                                struct player_t p) {
  vertex_t current = start;
  for (int steps = 1; steps <= max_steps; ++steps) {
    int found = 0;
    for (vertex_t v = 0; v < graph->num_vertices; ++v) {
      if (gsl_spmatrix_uint_get(graph->t, current, v) == last_direction &&
          is_valid_move(current, v, graph, p)) {
        current = v;
        found = 1;
        break;
      }
    }
    if (!found) {
      return (vertex_t)(-1);
    }
  }
  return current;
}

// déplacer dans une direction à 30° du dernier mouvement a l'ouest , jusqu’à 2
// cases.
vertex_t move_in_30_deg_direction_OEUST(vertex_t start,
                                        enum dir_t last_direction,
                                        int max_steps, struct graph_t *graph,
                                        struct player_t p,
                                        vertex_t *first_step) {
  enum dir_t next_direction = next_dir(last_direction);
  vertex_t current = start;
  *first_step = (vertex_t)(-1);
  for (int steps = 1; steps <= max_steps; ++steps) {
    int found = 0;
    for (vertex_t v = 0; v < graph->num_vertices; ++v) {
      if (gsl_spmatrix_uint_get(graph->t, current, v) == next_direction &&
          is_valid_move(current, v, graph, p)) {
        if (steps == 1)
          *first_step = v; // On retient le premier pas
        current = v;
        found = 1;
        break;
      }
    }
    if (!found) {
      return (vertex_t)(-1);
    }
  }
  return current;
}
// Déplacer vers un voisin valide
vertex_t move_to_neighbour(vertex_t start, struct graph_t *graph,
                           struct player_t p) {
  for (vertex_t v = 0; v < graph->num_vertices; ++v) {
    if (is_valid_move(start, v, graph, p)) {
      return v;
    }
  }
  return (vertex_t)(-1);
}

/* A function to determine the next dir in the clockwise order */
enum dir_t next_dir_clock(enum dir_t d) {
  return (d == 0) ? 0 : (d == 6) ? FIRST_DIR : (d + 1);
}

// déplacer dans une direction à 30° du dernier mouvement a l'est , jusqu’à 2
// cases.
// Déplacer dans une direction à 30° du dernier mouvement à l'est, jusqu’à 2
// cases.
vertex_t move_in_30_deg_direction_EST(vertex_t start, enum dir_t last_direction,
                                      int max_steps, struct graph_t *graph,
                                      struct player_t p, vertex_t *first_step) {
  enum dir_t next_direction = next_dir_clock(last_direction);
  vertex_t current = start;
  *first_step = (vertex_t)(-1);
  for (int steps = 1; steps <= max_steps; ++steps) {
    int found = 0;
    for (vertex_t v = 0; v < graph->num_vertices; ++v) {
      if (gsl_spmatrix_uint_get(graph->t, current, v) == next_direction &&
          is_valid_move(current, v, graph, p)) {
        if (steps == 1)
          *first_step = v; // On retient le premier pas
        current = v;
        found = 1;
        break;
      }
    }
    if (!found) {
      return (vertex_t)(-1);
    }
  }
  return current;
}

// Cette fonction vérifiera si une case est libre (pas d’adversaire dessus) et
// accessible (correspond à une arête du graphe).
int is_valid_move(vertex_t start, vertex_t end, struct graph_t *graph,
                  struct player_t p) {
  if (gsl_spmatrix_uint_get(graph->t, start, end) == NO_EDGE) {
    return 0;
  }
  if (gsl_spmatrix_uint_get(graph->t, start, end) == WALL_DIR) {
    return 0;
  }
  if (p.position == end || p.opponent_position == end) {
    return 0;
  }
  return 1;
}

// verifier si l'adversaire est devant son objectif sans obstacle
vertex_t opponnent_front_obj(struct graph_t *g, struct player_t p) {
  for (vertex_t v = 0; v < g->num_objectives; v++) {
    if (FIRST_DIR <= gsl_spmatrix_uint_get(g->t, p.opponent_position,
                                           g->objectives[v]) &&
        gsl_spmatrix_uint_get(g->t, p.opponent_position, g->objectives[v]) <=
            LAST_DIR) {
      return g->objectives[v];
    }
  }
  return (vertex_t)(-1);
}

// Vérifier si une arête est bloquée par un mur.
int is_edge_blocked(vertex_t start, vertex_t end, struct graph_t *graph) {
  if (gsl_spmatrix_uint_get(graph->t, start, end) == WALL_DIR) {
    return 1;
  }
  return 0;
}

// sauter par-dessus l'adversaire .
vertex_t can_jump(vertex_t my_pos, vertex_t opponent_pos, struct graph_t *graph,
                  struct player_t p) {
  // Vérifier si l'adversaire est adjacent
  enum dir_t jump_dir = gsl_spmatrix_uint_get(graph->t, my_pos, opponent_pos);
  if (jump_dir == NO_EDGE) {
    return (vertex_t)(-1);
  }
  // position d'arrivée en sautant l'adversaire
  vertex_t jump_target = (vertex_t)(-1);
  for (vertex_t v = 0; v < graph->num_vertices; ++v) {
    if (gsl_spmatrix_uint_get(graph->t, opponent_pos, v) == jump_dir &&
        is_valid_move(opponent_pos, v, graph, p) && v != my_pos) {
      jump_target = v;
      break;
    }
  }

  if (jump_target == (vertex_t)(-1)) {
  } else {
  }

  return jump_target;
}

// compter le nombre d'arrétes entourés par un sommet bloqués ou non par le mur
int count_edge(struct graph_t *g, vertex_t v) {
  int a = 3;
  int b = -3;
  int c = 1 - (int)g->num_vertices;
  int m;
  double delta = b * b - 4 * a * c;

  double sqrt_delta = sqrt(delta);
  double m1 = (-b + sqrt_delta) / (2 * a);
  double m2 = (-b - sqrt_delta) / (2 * a);

  for (int m_int = (int)floor(fmin(m1, m2)); m_int <= (int)ceil(fmax(m1, m2));
       ++m_int) {
    if (3 * m_int * m_int - 3 * m_int + 1 == (int)g->num_vertices &&
        9 * m_int * m_int - 15 * m_int + 6 == (int)g->num_edges) {
      m = m_int;
    }
  }
  int i = (int)v;
  int N = 3 * (m * m) - 3 * 3 + 1;
  if (i == 0 || i == N - 1 || i == m - 1 || i == N - m ||
      i == (N - 1) / 2 - m - 1 || i == (N - 1) / 2 + m - 1)
    return 3;
  for (int a = 1; a < m - 1; a++) {
    if (i == a)
      return 4;
  }
  for (int a = N - m + 1; a < N - 1; a++) {
    if (i == a)
      return 4;
  }
  int u = m - 1;
  for (int k = 0; k < m - 2; k++) {
    int a = u + 1;
    int u = a + (m + k);
    if (i == a || i == u) {
      return 4;
    }
  }
  u = u + (2 * m) - 1;
  for (int k = m - 3; k >= 0; k--) {
    int a = u + 1;
    int u = a + (m + k);
    if (i == a || i == u) {
      return 4;
    }
  }
  return 6;
}

// nombre d'arrétes bloqués bloqués par les murs autour d'un sommet
int edge_wall(struct graph_t *g, vertex_t v) {
  int nb_edge = 0;
  for (enum dir_t i = FIRST_DIR; i <= LAST_DIR; i++) {
    for (vertex_t j = 0; j < g->num_vertices; j++) {
      if (gsl_spmatrix_uint_get(g->t, v, j) == i) {
        nb_edge++;
      }
    }
  }
  int total_edge = count_edge(g, v);
  return total_edge - nb_edge;
}

/*/ Compter le nombre de murs autour d'un sommet
int count_wall(struct graph_t *g, vertex_t v) {
  int n = count_wall(g, v);
  return n/2;
}*/
// Vérifier si le mur bloque le chemin vers un sommet
int does_wall_block_v(struct graph_t *g, vertex_t v) {
  int nb_edge = count_edge(g, v) - edge_wall(g, v);
  if (nb_edge <= 2) {
    return 1;
  }
  return 0;
}

// Placer un mur sans bloquer l'adversaire oui ou non
int can_place_wall(struct graph_t *g, struct player_t p) {
  if (p.num_walls_left <= 0) {
    return 0;
  }
  vertex_t pos_opponent = p.opponent_position;
  int remaining_edges =
      count_edge(g, pos_opponent) - edge_wall(g, pos_opponent);

  if (remaining_edges <= 2) {
    return 0;
  }
  return 1;
}

// placer un mur
int place_wall(struct graph_t *g, struct move_t move, struct player_t p) {
  if (p.num_walls_left == 0) {
    return -1;
  }
  if (is_edge_blocked(move.e[0].fr, move.e[0].to, g) ||
      is_edge_blocked(move.e[1].fr, move.e[1].to, g)) {
    return -1;
  }
  if (!can_place_wall(g, p)) {
    return -1;
  }
  if (gsl_spmatrix_uint_get(g->t, move.e[0].fr, move.e[0].to) == NO_EDGE ||
      gsl_spmatrix_uint_get(g->t, move.e[1].fr, move.e[1].to) == NO_EDGE) {
    return -1;
  }
  // Convertir la matrice CSR en COO
  gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
      g->t->size1, g->t->size2, g->t->nz, GSL_SPMATRIX_COO);
  for (size_t row = 0; row < g->t->size1; ++row) {
    for (int k = g->t->p[row]; k < g->t->p[row + 1]; ++k) {
      gsl_spmatrix_uint_set(coo, row, g->t->i[k], g->t->data[k]);
    }
  }

  // Modifier la matrice COO pour ajouter le mur
  gsl_spmatrix_uint_set(coo, move.e[0].fr, move.e[0].to, WALL_DIR);
  gsl_spmatrix_uint_set(coo, move.e[0].to, move.e[0].fr, WALL_DIR);
  gsl_spmatrix_uint_set(coo, move.e[1].fr, move.e[1].to, WALL_DIR);
  gsl_spmatrix_uint_set(coo, move.e[1].to, move.e[1].fr, WALL_DIR);

  // Reconvertir COO en CSR
  gsl_spmatrix_uint *new_csr =
      gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(coo);
  gsl_spmatrix_uint_free(g->t);
  g->t = new_csr;

  p.num_walls_left--;
  return 1;
}

// Vérifier si l'adversaire est sur un sommet objectif
int is_opponent_on_objective(struct graph_t *graph, struct player_t player) {
  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    if (player.opponent_position == graph->objectives[i]) {
      return 1;
    }
  }
  return 0;
}

// Connaitre quel objectif on atteint par la fonction shortest_path
vertex_t obj_to_visit(struct player_t *player, struct way_t way) {
  if (way.len == 0) {
    return (vertex_t)(-1);
  }
  // dernier sommet du chemin
  vertex_t next_objective = way.sommets[way.len - 1];
  // Vérifier si c'est un objectif
  for (unsigned int i = 0; i < player->graph->num_objectives; i++) {
    if (next_objective == player->graph->objectives[i]) {
      return next_objective;
    }
  }
  return (vertex_t)(-1);
}
