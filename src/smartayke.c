#include "dijkstra.h"
#include "graph_utils.h"
#include "player_utils.h"
#include <gsl/gsl_spmatrix_uint.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MOVES 50
#define MAX_SCENARIOS 1000
#define MAX_CHILDREN 8

struct player_t ayke;

struct scenario_t {
  struct scenario_t *parent;
  struct graph_t *graph;
  vertex_t pos[2];
  unsigned int walls_left[2];
  vertex_t **objectives;
  int current_player;
  struct scenario_t *fils[MAX_CHILDREN];
  int score[2];
  vertex_t **objectives_visited;
  int depth;
  enum dir_t last_direction;
};

char const *get_player_name() { return "Smart_ayke"; }

// fonction qui initialise le player avec tous ces propriétés( position ,
// position de l'adversaire ...)
void initialize(unsigned int player_id, struct graph_t *graph) {
  ayke.last_direction = NO_EDGE;
  init_player(&ayke, player_id, graph);
  printf("\u2705 ayke initialis\u00e9 (joueur %d, position %u)\n", player_id,
         ayke.position);
}
// focntion qui prend en paramètre un scénario et l id du player et retourne ce
// même scénario mais avec ses objectifs mis a jour
void update_objectives_visited(struct scenario_t *s, int p) {
  if (!s || !s->graph || p < 0 || p > 1 || !s->objectives ||
      !s->objectives_visited || !s->objectives[p] ||
      !s->objectives_visited[p]) {
    return;
  }

  unsigned int nb_obj = s->graph->num_objectives;
  for (unsigned int i = 0; i < nb_obj; ++i) {
    if (i < nb_obj && s->pos[p] < s->graph->num_vertices && // Validate position
        !s->objectives_visited[p][i] && s->pos[p] == s->objectives[p][i]) {
      s->objectives_visited[p][i] = 1;
    }
  }
}
bool has_path_to(vertex_t from, vertex_t to, struct graph_t *g) {
  if (!g || from >= g->num_vertices || to >= g->num_vertices)
    return false;

  struct way_t path = shortest_path_between(from, to, g);

  bool result = (path.len > 0 && path.sommets != NULL);
  if (path.sommets)
    free(path.sommets);

  return result;
}
bool all_objectives_visited(vertex_t *visited, unsigned int nb_obj) {
  for (unsigned int i = 0; i < nb_obj; i++) {
    if (!visited[i])
      return false;
  }
  return true;
}

bool is_wall_valid(struct scenario_t *scenario, vertex_t a, vertex_t b,
                   vertex_t c) {
  struct graph_t *g = scenario->graph;

  if (!g || a >= g->num_vertices || b >= g->num_vertices ||
      c >= g->num_vertices)
    return false;

  if (gsl_spmatrix_uint_get(g->t, a, b) == WALL_DIR ||
      gsl_spmatrix_uint_get(g->t, b, a) == WALL_DIR ||
      gsl_spmatrix_uint_get(g->t, a, c) == WALL_DIR ||
      gsl_spmatrix_uint_get(g->t, c, a) == WALL_DIR)
    return false;

  // Copie du graphe
  struct graph_t *test_g = make_copy(g);
  if (!test_g)
    return false;

  // Conversion en COO
  gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
      test_g->t->size1, test_g->t->size2, test_g->t->nz, GSL_SPMATRIX_COO);

  for (size_t row = 0; row < test_g->t->size1; ++row) {
    for (int idx = test_g->t->p[row]; idx < test_g->t->p[row + 1]; ++idx) {
      gsl_spmatrix_uint_set(coo, row, test_g->t->i[idx], test_g->t->data[idx]);
    }
  }

  // Bloque les deux arêtes
  gsl_spmatrix_uint_set(coo, a, b, WALL_DIR);
  gsl_spmatrix_uint_set(coo, b, a, WALL_DIR);
  gsl_spmatrix_uint_set(coo, a, c, WALL_DIR);
  gsl_spmatrix_uint_set(coo, c, a, WALL_DIR);

  gsl_spmatrix_uint *new_csr =
      gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(coo);
  gsl_spmatrix_uint_free(test_g->t);
  test_g->t = new_csr;

  // Vérifie pour chaque joueur que tous les objectifs sont accessibles + retour
  // possible au départ
  bool path_ok[2] = {true, true};

  for (int p = 0; p < 2; ++p) {
    vertex_t from = scenario->pos[p];
    const vertex_t *goals = scenario->objectives[p];
    const vertex_t *visited = scenario->objectives_visited[p];
    vertex_t start = scenario->graph->start[p];
    unsigned int nb_obj = test_g->num_objectives;

    for (unsigned int i = 0; i < nb_obj; ++i) {
      if (!visited[i]) {
        if (!has_path_to(from, goals[i], test_g)) {
          path_ok[p] = false;
          break;
        }
      }
    }

    if (path_ok[p] && from != start && !has_path_to(from, start, test_g))
      path_ok[p] = false;
  }

  free_graph(test_g);
  return path_ok[0] && path_ok[1];
}

enum dir_t get_direction_between_vertices(const struct graph_t *g,
                                          vertex_t from, vertex_t to) {
  if (!g || !g->t || from >= g->num_vertices || to >= g->num_vertices)
    return NO_EDGE;

  return (enum dir_t)gsl_spmatrix_uint_get(g->t, from, to);
}
vertex_t get_vertex_from_direction(const struct graph_t *g, vertex_t from,
                                   enum dir_t dir) {
  if (!g || !g->t || from >= g->num_vertices || dir == NO_EDGE ||
      dir > WALL_DIR)
    return UINT_MAX;

  for (int idx = g->t->p[from]; idx < g->t->p[from + 1]; ++idx) {
    if (g->t->data[idx] == dir) {
      return g->t->i[idx]; // voisin trouvé dans la direction donnée
    }
  }

  return UINT_MAX;
}
vertex_t go_to_position_ahead(struct scenario_t *scenario) {
  vertex_t opp_pos, me_pos;
  opp_pos = scenario->pos[1 - scenario->current_player];
  me_pos = scenario->pos[scenario->current_player];

  struct way_t my_way;
  if (!all_objectives_visited(
          scenario->objectives_visited[scenario->current_player],
          scenario->graph->num_objectives)) {
    my_way =
        shortest_path2(me_pos, scenario->graph,
                       scenario->objectives_visited[scenario->current_player]);
  } else {
    my_way = shortest_path_between(
        me_pos, scenario->graph->start[scenario->current_player],
        scenario->graph);
  }

  if (my_way.len < 2 || my_way.sommets == NULL) {
    if (my_way.sommets)
      free(my_way.sommets);
    return UINT_MAX; // Ne retourne plus la position actuelle
  }

  assert(my_way.sommets[0] == me_pos);
  vertex_t ahead_me = my_way.sommets[1];
  vertex_t new_pos;
  scenario->last_direction =
      get_direction_between_vertices(scenario->graph, me_pos, ahead_me);
  if (opp_pos == ahead_me) {
    new_pos = get_vertex_from_direction(
        scenario->graph, opp_pos,
        get_direction_between_vertices(scenario->graph, me_pos, opp_pos));
  } else {
    new_pos = ahead_me;
  }
  if (new_pos == scenario->pos[scenario->current_player])
    new_pos = UINT_MAX;

  free(my_way.sommets);
  return new_pos;
}
vertex_t go_to_position_right(struct scenario_t *scenario) {
  vertex_t opp_pos, me_pos;
  opp_pos = scenario->pos[1 - scenario->current_player];
  me_pos = scenario->pos[scenario->current_player];

  struct way_t my_way;

  my_way =
      shortest_path2(me_pos, scenario->graph,
                     scenario->objectives_visited[scenario->current_player]);

  if (my_way.len < 2 || !my_way.sommets) {
    if (my_way.sommets)
      free(my_way.sommets);
    return me_pos;
  }

  vertex_t ahead_me = my_way.sommets[1];
  enum dir_t dir_ahead =
      get_direction_between_vertices(scenario->graph, me_pos, ahead_me);
  enum dir_t dir_right = (dir_ahead % 6) + 1;
  if (dir_right > 6)
    dir_right = 1;
  scenario->last_direction = dir_right;
  vertex_t to_right =
      get_vertex_from_direction(scenario->graph, me_pos, dir_right);
  vertex_t new_pos = me_pos;

  if (to_right != UINT_MAX) {
    if (opp_pos == to_right) {
      enum dir_t dir =
          get_direction_between_vertices(scenario->graph, me_pos, opp_pos);
      new_pos = get_vertex_from_direction(scenario->graph, opp_pos, dir);
    }
  } else {
    new_pos = to_right;
  }

  free(my_way.sommets);
  return (new_pos != UINT_MAX) ? new_pos : UINT_MAX;
}
vertex_t go_to_position_left(struct scenario_t *scenario) {
  vertex_t opp_pos, me_pos;
  opp_pos = scenario->pos[1 - scenario->current_player];
  me_pos = scenario->pos[scenario->current_player];

  struct way_t my_way;

  my_way =
      shortest_path2(me_pos, scenario->graph,
                     scenario->objectives_visited[scenario->current_player]);

  if (my_way.len < 2 || !my_way.sommets) {
    if (my_way.sommets)
      free(my_way.sommets);
    return me_pos;
  }

  vertex_t ahead_me = my_way.sommets[1];

  enum dir_t dir_ahead =
      get_direction_between_vertices(scenario->graph, me_pos, ahead_me);

  enum dir_t dir_left = (dir_ahead - 2 + 6) % 6 + 1;
  scenario->last_direction = dir_left;

  vertex_t to_left =
      get_vertex_from_direction(scenario->graph, me_pos, dir_left);
  vertex_t new_pos = me_pos;

  if (to_left != UINT_MAX) {
    if (opp_pos == to_left) {
      enum dir_t dir =
          get_direction_between_vertices(scenario->graph, me_pos, opp_pos);
      new_pos = get_vertex_from_direction(scenario->graph, opp_pos, dir);
    }
  } else {
    new_pos = to_left;
  }

  free(my_way.sommets);
  return (new_pos != UINT_MAX) ? new_pos : UINT_MAX;
}

vertex_t go_to_position_3(struct scenario_t *scenario) {
  vertex_t me_pos = scenario->pos[scenario->current_player];
  vertex_t opp_pos = scenario->pos[1 - scenario->current_player];
  if (ayke.last_direction == NO_EDGE) {
    return UINT_MAX;
  }
  struct way_t path =
      shortest_path2(me_pos, scenario->graph,
                     scenario->objectives_visited[scenario->current_player]);
  if (path.len < 2 || !path.sommets) {
    if (path.sommets)
      free(path.sommets);
    return me_pos;
  }

  enum dir_t ref_dir;

  if ((unsigned int)scenario->current_player == ayke.player_id)
    ref_dir = ayke.last_direction;
  else
    ref_dir = get_direction_between_vertices(
        scenario->graph, ayke.opp_last_position, ayke.opponent_position);
  scenario->last_direction = ref_dir;

  vertex_t new_pos = me_pos;
  for (int i = 0; i < 3; i++) {
    vertex_t next =
        get_vertex_from_direction(scenario->graph, new_pos, ref_dir);
    if (new_pos == UINT_MAX || new_pos == opp_pos) {
      free(path.sommets);
      return UINT_MAX;
    }
    new_pos = next;

    // marquer un objectif éventuel
    for (unsigned j = 0; j < scenario->graph->num_objectives; ++j) {
      if (new_pos == scenario->objectives[scenario->current_player][j]) {
        scenario->objectives_visited[scenario->current_player][j] = 1;
      }
    }
  }
  free(path.sommets);
  return new_pos;
}

vertex_t go_to_position_2_right(struct scenario_t *scenario) {
  vertex_t me_pos = scenario->pos[scenario->current_player];
  vertex_t opp_pos = scenario->pos[1 - scenario->current_player];
  if (ayke.last_direction == NO_EDGE) {
    return UINT_MAX;
  }

  struct way_t path =
      shortest_path2(me_pos, scenario->graph,
                     scenario->objectives_visited[scenario->current_player]);

  if (path.len < 2 || !path.sommets) {
    if (path.sommets)
      free(path.sommets);
    return UINT_MAX;
  }

  enum dir_t ref_dir;
  if ((unsigned int)scenario->current_player == ayke.player_id)
    ref_dir = ayke.last_direction;
  else
    ref_dir = get_direction_between_vertices(
        scenario->graph, ayke.opp_last_position, ayke.opponent_position);

  enum dir_t dir_right = (ref_dir % 6) + 1;
  scenario->last_direction = dir_right;
  if (dir_right > 6)
    dir_right = 1;

  vertex_t first =
      get_vertex_from_direction(scenario->graph, me_pos, dir_right);
  if (first == UINT_MAX) {
    free(path.sommets);
    return UINT_MAX;
  } else {
    for (unsigned int i = 0; i < scenario->graph->num_objectives; i++) {
      if (first == scenario->objectives[scenario->current_player][i])
        scenario->objectives_visited[scenario->current_player][i] = 1;
    }
  }

  vertex_t second =
      get_vertex_from_direction(scenario->graph, first, dir_right);
  if (second != UINT_MAX && second != opp_pos) {
    free(path.sommets);
    for (unsigned int i = 0; i < scenario->graph->num_objectives; i++) {
      if (second == scenario->objectives[scenario->current_player][i])
        scenario->objectives_visited[scenario->current_player][i] = 1;
    }

    return second;
  } else {
    if (first != opp_pos) {
      free(path.sommets);
      return first;
    }
  }
  free(path.sommets);
  return UINT_MAX;
}

vertex_t go_to_position_2_left(struct scenario_t *scenario) {
  vertex_t me_pos = scenario->pos[scenario->current_player];
  vertex_t opp_pos = scenario->pos[1 - scenario->current_player];
  if (ayke.last_direction == NO_EDGE) {
    return UINT_MAX;
  }

  struct way_t path =
      shortest_path2(me_pos, scenario->graph,
                     scenario->objectives_visited[scenario->current_player]);

  if (path.len < 2 || !path.sommets) {
    if (path.sommets)
      free(path.sommets);
    return UINT_MAX;
  }

  enum dir_t ref_dir;
  if ((unsigned int)scenario->current_player == ayke.player_id)
    ref_dir = ayke.last_direction;
  else
    ref_dir = get_direction_between_vertices(
        scenario->graph, ayke.opp_last_position, ayke.opponent_position);
  enum dir_t dir_left = (ref_dir - 2 + 6) % 6 + 1;
  scenario->last_direction = dir_left;

  vertex_t first = get_vertex_from_direction(scenario->graph, me_pos, dir_left);
  if (first == UINT_MAX) {
    free(path.sommets);
    return UINT_MAX;
  } else {
    for (unsigned int i = 0; i < scenario->graph->num_objectives; i++) {
      if (first == scenario->objectives[scenario->current_player][i])
        scenario->objectives_visited[scenario->current_player][i] = 1;
    }
  }

  vertex_t second = get_vertex_from_direction(scenario->graph, first, dir_left);

  if (second != UINT_MAX && second != opp_pos) {
    free(path.sommets);
    for (unsigned int i = 0; i < scenario->graph->num_objectives; i++) {
      if (second == scenario->objectives[scenario->current_player][i])
        scenario->objectives_visited[scenario->current_player][i] = 1;
    }
    return second;
  } else {
    if (first != opp_pos) {
      free(path.sommets);
      return first;
    }
  }
  free(path.sommets);
  return UINT_MAX;
}

void free_scenario(struct scenario_t *node) {
  if (!node)
    return;

  // Libération récursive des enfants
  for (int i = 0; i < MAX_CHILDREN; i++) {
    if (node->fils[i])
      free_scenario(node->fils[i]);
  }

  // Libération du graphe (si ce n'est pas le graphe principal du joueur)
  if (node->graph && node->graph != ayke.graph) {
    free_graph(node->graph);
  }

  // Libération des objectifs
  if (node->objectives) {
    for (int p = 0; p < 2; ++p) {
      free(node->objectives[p]);
    }
    free(node->objectives);
  }

  // Libération des objectifs atteints
  if (node->objectives_visited) {
    for (int p = 0; p < 2; ++p) {
      free(node->objectives_visited[p]);
    }
    free(node->objectives_visited);
  }

  free(node);
}

size_t count_neighbors(const struct graph_t *g, vertex_t v) {
  size_t count = 0;
  for (int i = g->t->p[v]; i < g->t->p[v + 1]; ++i) {
    if (g->t->data[i] != WALL_DIR && g->t->data[i] != NO_EDGE) {
      count++;
    }
  }
  return count;
}

void add_wall(struct graph_t *g, vertex_t a, vertex_t b) {
  if (a >= g->t->size1 || b >= g->t->size2) {
    fprintf(stderr,
            "\u274c [add_wall] Indices hors limites : a=%u, b=%u (max=%zu)\n",
            a, b, g->t->size1);
    return;
  }

  gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
      g->t->size1, g->t->size2, g->t->nz, GSL_SPMATRIX_COO);
  for (size_t row = 0; row < g->t->size1; ++row) {
    for (int idx = g->t->p[row]; idx < g->t->p[row + 1]; ++idx) {
      gsl_spmatrix_uint_set(coo, row, g->t->i[idx], g->t->data[idx]);
    }
  }

  gsl_spmatrix_uint_set(coo, a, b, WALL_DIR);
  gsl_spmatrix_uint_set(coo, b, a, WALL_DIR);

  gsl_spmatrix_uint *new_csr =
      gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(coo);
  gsl_spmatrix_uint_free(g->t);
  g->t = new_csr;
}

bool place_wall_right(struct scenario_t *scenario) {
  vertex_t opp_pos = scenario->pos[1 - scenario->current_player];

  struct way_t opp_way = shortest_path2(
      opp_pos, scenario->graph,
      scenario->objectives_visited[1 - scenario->current_player]);

  // Marquage d'un objectif atteint
  if (opp_way.len == 1 && opp_way.sommets) {
    unsigned int nb_obj = scenario->graph->num_objectives;
    for (unsigned int i = 0; i < nb_obj; i++) {
      if (!scenario->objectives_visited[1 - scenario->current_player][i] &&
          opp_pos == scenario->objectives[1 - scenario->current_player][i]) {
        scenario->objectives_visited[1 - scenario->current_player][i] = 1;
        break;
      }
    }
  }

  if (opp_way.len < 2 || !opp_way.sommets) {
    if (opp_way.sommets)
      free(opp_way.sommets);
    return false;
  }

  vertex_t ahead = opp_way.sommets[1];
  enum dir_t dir_to_ahead =
      get_direction_between_vertices(scenario->graph, opp_pos, ahead);

  if (dir_to_ahead == NO_EDGE || dir_to_ahead == WALL_DIR) {
    free(opp_way.sommets);
    return false;
  }

  enum dir_t dir_right = (dir_to_ahead % 6) + 1;
  if (dir_right > 6)
    dir_right = 1;
  vertex_t right =
      get_vertex_from_direction(scenario->graph, opp_pos, dir_right);

  if (ahead == UINT_MAX || right == UINT_MAX) {
    free(opp_way.sommets);
    return false;
  }

  size_t neighbor_count = count_neighbors(scenario->graph, opp_pos);

  if (neighbor_count >= 3 && is_wall_valid(scenario, opp_pos, ahead, right)) {
    add_wall(scenario->graph, opp_pos, ahead);
    add_wall(scenario->graph, opp_pos, right);
    free(opp_way.sommets);

    return true;
  }
  free(opp_way.sommets);

  return false;
}

bool place_wall_left(struct scenario_t *scenario) {
  vertex_t opp_pos = scenario->pos[1 - scenario->current_player];

  struct way_t opp_way = shortest_path2(
      opp_pos, scenario->graph,
      scenario->objectives_visited[1 - scenario->current_player]);

  // Marquage d’un objectif atteint
  if (opp_way.len == 1 && opp_way.sommets) {
    unsigned int nb_obj = scenario->graph->num_objectives;
    for (unsigned int i = 0; i < nb_obj; i++) {
      if (!scenario->objectives_visited[1 - scenario->current_player][i] &&
          opp_pos == scenario->objectives[1 - scenario->current_player][i]) {
        scenario->objectives_visited[1 - scenario->current_player][i] = 1;
        break;
      }
    }
  }

  if (opp_way.len < 2 || !opp_way.sommets) {
    if (opp_way.sommets)
      free(opp_way.sommets);
    return false;
  }

  vertex_t ahead = opp_way.sommets[1];
  enum dir_t dir_to_ahead =
      get_direction_between_vertices(scenario->graph, opp_pos, ahead);

  if (dir_to_ahead == NO_EDGE || dir_to_ahead == WALL_DIR) {
    free(opp_way.sommets);
    return false;
  }

  enum dir_t dir_left = (dir_to_ahead - 2 + 6) % 6 + 1;

  vertex_t left = get_vertex_from_direction(scenario->graph, opp_pos, dir_left);

  if (ahead == UINT_MAX || left == UINT_MAX) {
    free(opp_way.sommets);
    return false;
  }

  size_t neighbor_count = count_neighbors(scenario->graph, opp_pos);

  if (neighbor_count >= 3 && is_wall_valid(scenario, opp_pos, ahead, left)) {
    add_wall(scenario->graph, opp_pos, ahead);
    add_wall(scenario->graph, opp_pos, left);
    free(opp_way.sommets);
    return true;
  }
  free(opp_way.sommets);
  return false;
}

struct scenario_t *init_first_scenario(struct graph_t *g,
                                       unsigned int player_id, int depth) {
  struct scenario_t *s = calloc(1, sizeof(struct scenario_t));
  if (!s)
    return NULL;

  s->parent = NULL;
  s->depth = depth;
  s->graph = g;
  s->current_player = player_id;

  s->pos[player_id] =
      (player_id == ayke.player_id) ? ayke.position : ayke.opponent_position;
  s->pos[!player_id] =
      (player_id == ayke.player_id) ? ayke.opponent_position : ayke.position;

  s->objectives = malloc(2 * sizeof(vertex_t *));
  s->objectives_visited = malloc(2 * sizeof(vertex_t *));
  if (!s->objectives || !s->objectives_visited) {
    free(s->objectives);
    free(s->objectives_visited);
    free(s);
    return NULL;
  }

  for (int p = 0; p < 2; ++p) {
    unsigned int nb_obj = g->num_objectives;
    s->objectives[p] = malloc(nb_obj * sizeof(vertex_t));
    s->objectives_visited[p] = calloc(nb_obj, sizeof(vertex_t));

    if (!s->objectives[p] || !s->objectives_visited[p]) {
      for (int i = 0; i <= p; i++) {
        free(s->objectives[i]);
        free(s->objectives_visited[i]);
      }
      free(s->objectives);
      free(s->objectives_visited);
      free(s);
      return NULL;
    }

    const vertex_t *src_obj = ((unsigned int)p == player_id)
                                  ? ayke.my_objectives
                                  : ayke.opp_objectives;
    memcpy(s->objectives[p], src_obj, nb_obj * sizeof(vertex_t));

    const vertex_t *src_vis = ((unsigned int)p == player_id)
                                  ? ayke.visited_objectives
                                  : ayke.opp_visited_objectives;
    memcpy(s->objectives_visited[p], src_vis, nb_obj * sizeof(vertex_t));
  }

  unsigned int my_walls =
      (player_id == ayke.player_id) ? ayke.num_walls_left : ayke.opp_walls_left;
  unsigned int opp_walls =
      (player_id == ayke.player_id) ? ayke.opp_walls_left : ayke.num_walls_left;

  s->walls_left[player_id] = my_walls;
  s->walls_left[!player_id] = opp_walls;

  for (int i = 0; i < MAX_CHILDREN; i++)
    s->fils[i] = NULL;

  return s;
}

struct scenario_t *init_scenario(struct graph_t *g, unsigned int player_id,
                                 int depth, struct scenario_t *parent) {
  struct scenario_t *s = calloc(1, sizeof(struct scenario_t));
  if (!s)
    return NULL;

  s->parent = parent;
  s->depth = depth;
  s->graph = g;
  s->current_player = player_id;

  s->pos[player_id] = (player_id == ayke.player_id)
                          ? parent->pos[ayke.player_id]
                          : parent->pos[1 - ayke.player_id];
  s->pos[!player_id] = (player_id == ayke.player_id)
                           ? parent->pos[1 - ayke.player_id]
                           : parent->pos[ayke.player_id];

  s->objectives = malloc(2 * sizeof(vertex_t *));
  s->objectives_visited = malloc(2 * sizeof(vertex_t *));
  if (!s->objectives || !s->objectives_visited) {
    free(s->objectives);
    free(s->objectives_visited);
    free(s);
    return NULL;
  }

  for (int p = 0; p < 2; ++p) {
    unsigned int nb_obj = g->num_objectives;
    s->objectives[p] = malloc(nb_obj * sizeof(vertex_t));
    s->objectives_visited[p] = calloc(nb_obj, sizeof(vertex_t));

    if (!s->objectives[p] || !s->objectives_visited[p]) {
      for (int i = 0; i <= p; i++) {
        free(s->objectives[i]);
        free(s->objectives_visited[i]);
      }
      free(s->objectives);
      free(s->objectives_visited);
      free(s);
      return NULL;
    }

    const vertex_t *src_obj = ((unsigned int)p == player_id)
                                  ? ayke.my_objectives
                                  : ayke.opp_objectives;
    memcpy(s->objectives[p], src_obj, nb_obj * sizeof(vertex_t));

    const vertex_t *src_vis = ((unsigned int)p == player_id)
                                  ? parent->objectives_visited[player_id]
                                  : parent->objectives_visited[1 - player_id];

    memcpy(s->objectives_visited[p], src_vis, nb_obj * sizeof(vertex_t));
  }

  unsigned int my_walls = (player_id == ayke.player_id)
                              ? parent->walls_left[ayke.player_id]
                              : parent->walls_left[1 - ayke.player_id];
  unsigned int opp_walls = (player_id == ayke.player_id)
                               ? parent->walls_left[1 - ayke.player_id]
                               : parent->walls_left[ayke.player_id];

  s->walls_left[player_id] = my_walls;
  s->walls_left[!player_id] = opp_walls;

  for (int i = 0; i < MAX_CHILDREN; i++)
    s->fils[i] = NULL;

  return s;
}

vertex_t find_arbitrary_neighbor(struct graph_t *g, vertex_t from,
                                 int direction) {
  for (int i = 0; i < 6; ++i) {
    enum dir_t dir = (direction + i) % 6;
    for (vertex_t to = 0; to < g->num_vertices; ++to) {
      if (gsl_spmatrix_uint_get(g->t, from, to) == dir &&
          to < g->num_vertices) {
        return to;
      }
    }
  }
  return UINT_MAX;
}

void evaluate_scores(struct scenario_t *s, unsigned int ref_player) {
  if (!s || !s->graph || !s->objectives_visited)
    return;

  // Met à jour l'état des objectifs visités pour les deux joueurs
  update_objectives_visited(s, 0);
  update_objectives_visited(s, 1);

  // Calcul des scores bruts pour chaque joueur
  int sc[2] = {0, 0};
  for (int p = 0; p < 2; ++p) {
    struct way_t path =
        shortest_path2(s->pos[p], s->graph, s->objectives_visited[p]);
    int path_len = path.len;
    if (path.sommets)
      free(path.sommets);

    unsigned int nb_unvisited = 0;
    for (unsigned int i = 0; i < s->graph->num_objectives; ++i)
      if (!s->objectives_visited[p][i])
        ++nb_unvisited;

    sc[p] = -2 * path_len - 13 * nb_unvisited;
  }

  // Score relatif pour le joueur de référence
  s->score[ref_player] = sc[ref_player];
  s->score[1 - ref_player] = sc[1 - ref_player];
}
vertex_t safe_move(vertex_t from, vertex_t target, struct graph_t *g,
                   vertex_t *objectives_visited) {
  if (!g || !objectives_visited) {
    return from; // Return current position if invalid inputs
  }

  enum dir_t dir = get_direction_between_vertices(g, from, target);
  if (dir == NO_EDGE || dir == WALL_DIR) {
    struct way_t path = shortest_path2(from, g, objectives_visited);
    vertex_t next = (path.len > 1 && path.sommets) ? path.sommets[1] : from;
    if (path.sommets) {
      free(path.sommets);
    }
    return next;
  }
  return target;
}
void generate_scenario_depth(struct scenario_t *root, int depth,
                             int player_id) {
  if (depth <= 0 || !root || !root->graph)
    return;

  // Tableau des fonctions de déplacement
  vertex_t (*move_funcs[6])(struct scenario_t *) = {
      go_to_position_ahead, go_to_position_right,  go_to_position_left,
      go_to_position_3,     go_to_position_2_left, go_to_position_2_right};

  for (int i = 0; i < 6; ++i) {
    struct graph_t *copy = make_copy(root->graph);
    if (!copy)
      continue;

    root->fils[i] = init_scenario(copy, player_id, root->depth + 1, root);
    if (root->fils[i]) {
      vertex_t move = move_funcs[i](root->fils[i]);
      if (move != UINT_MAX && move != root->pos[player_id]) {
        root->fils[i]->pos[player_id] = move;
        update_objectives_visited(root->fils[i], player_id);
      } else {
        free_scenario(root->fils[i]);
        root->fils[i] = NULL;
      }
    }
  }

  struct graph_t *copyR = make_copy(root->graph);
  if (copyR) {
    root->fils[6] = init_scenario(copyR, player_id, root->depth + 1, root);
    if (root->fils[6]) {
      if (!place_wall_right(root->fils[6])) {
        free_scenario(root->fils[6]);
        root->fils[6] = NULL;
      } else {
        root->fils[6]->walls_left[player_id] = root->walls_left[player_id] - 1;
        update_objectives_visited(root->fils[6], player_id);
      }
    } else {
      free_graph(copyR);
    }
  }

  struct graph_t *copyL = make_copy(root->graph);
  if (copyL) {
    root->fils[7] = init_scenario(copyL, player_id, root->depth + 1, root);
    if (root->fils[7]) {
      if (!place_wall_left(root->fils[7])) {
        free_scenario(root->fils[7]);
        root->fils[7] = NULL;
      } else {
        root->fils[7]->walls_left[player_id] = root->walls_left[player_id] - 1;
        update_objectives_visited(root->fils[7], player_id);
      }
    } else {
      free_graph(copyL);
    }
  }

  for (int i = 0; i < MAX_CHILDREN; ++i) {
    if (root->fils[i]) {
      generate_scenario_depth(root->fils[i], depth - 1, !player_id);
    }
  }
}

struct minimax_result_t {
  int score;
  struct scenario_t *best_scenario;
};

// struct minimax_result_t minimax(struct scenario_t *node, int depth, int
// alpha,
//                                 int beta, int maximizing_player_id) {
//   struct minimax_result_t result = {.score = 0, .best_scenario = NULL};

//   if (!node)
//     return result;

//   if (depth == 0) {
//     evaluate_scores(node);
//     int opponent_id = !maximizing_player_id;
//     result.score = node->score[maximizing_player_id] -
//     node->score[opponent_id]; return result;
//   }

//   int is_max = (node->current_player == maximizing_player_id);
//   result.score = is_max ? INT_MIN : INT_MAX;

//   for (int i = 0; i < MAX_CHILDREN; i++) {
//     struct scenario_t *child = node->fils[i];
//     if (!child)
//       continue;

//     struct minimax_result_t child_result =
//         minimax(child, depth - 1, alpha, beta, maximizing_player_id);

//     if (is_max) {
//       if (child_result.score > result.score) {
//         result.score = child_result.score;
//         if (node->depth == 0)
//           result.best_scenario = child;
//       }
//       alpha = (child_result.score > alpha) ? child_result.score : alpha;
//     } else {
//       if (child_result.score < result.score) {
//         result.score = child_result.score;
//         if (node->depth == 0)
//           result.best_scenario = child;
//       }
//       beta = (child_result.score < beta) ? child_result.score : beta;
//     }

//     if (beta <= alpha)
//       break;
//   }

//   if (node->depth == 0 && result.best_scenario) {
//     assert(result.best_scenario->depth == 1 &&
//            "🔥 Le meilleur scénario doit être à depth == 1 !");
//   }

//   return result;
// }

// 2) minimax : on appelle evaluate_scores(node, maximizing_player_id) et on
// prend node->score[maximizer]
struct minimax_result_t minimax(struct scenario_t *node, int depth, int alpha,
                                int beta, int maximizing_player_id,
                                int max_depth) {
  struct minimax_result_t result = {.score = 0, .best_scenario = NULL};
  if (!node)
    return result;

  // FEUILLE : évalue une seule fois avec le même référentiel
  if (depth == max_depth) {
    evaluate_scores(node, maximizing_player_id);
    // stocké dans node->score[maximizer]
    result.score = node->score[maximizing_player_id] -
                   node->score[1 - maximizing_player_id];
    return result;
  }

  if (node->current_player == maximizing_player_id) {
    int max_eval = INT_MIN;
    for (int i = 0; i < MAX_CHILDREN; ++i) {
      if (!node->fils[i])
        continue;
      struct minimax_result_t child =
          minimax(node->fils[i], depth + 1, alpha, beta, maximizing_player_id,
                  max_depth);
      if (child.score > max_eval) {
        max_eval = child.score;
        if (depth == 0)
          result.best_scenario = node->fils[i];
      }
      alpha = (alpha > child.score) ? alpha : child.score;
      if (beta <= alpha)
        break;
    }
    result.score = max_eval;
  } else {
    int min_eval = INT_MAX;
    for (int i = 0; i < MAX_CHILDREN; ++i) {
      if (!node->fils[i])
        continue;
      struct minimax_result_t child =
          minimax(node->fils[i], depth + 1, alpha, beta, maximizing_player_id,
                  max_depth);
      if (child.score < min_eval) {
        min_eval = child.score;
        if (depth == 0)
          result.best_scenario = node->fils[i];
      }
      beta = (beta < child.score) ? beta : child.score;
      if (beta <= alpha)
        break;
    }
    result.score = min_eval;
  }
  return result;
}

struct move_t detect_wall_difference(struct graph_t *before,
                                     struct graph_t *after) {
  struct move_t move = {.c = ayke.color,
                        .t = NO_TYPE,
                        .m = 0,
                        .e = {{UINT_MAX, UINT_MAX}, {UINT_MAX, UINT_MAX}}};

  if (!before || !after || !before->t || !after->t)
    return move;

  int found = 0;

  for (vertex_t u = 0; u < before->num_vertices; ++u) {
    for (vertex_t v = u + 1; v < before->num_vertices; ++v) {
      unsigned int val_before = gsl_spmatrix_uint_get(before->t, u, v);
      unsigned int val_after = gsl_spmatrix_uint_get(after->t, u, v);

      if (val_before != WALL_DIR && val_after == WALL_DIR) {
        if (found < 2) {
          // Normaliser fr < to
          vertex_t fr = (u < v) ? u : v;
          vertex_t to = (u < v) ? v : u;
          move.e[found].fr = fr;
          move.e[found].to = to;
          found++;
        }
      }
    }
  }

  if (found == 2) {
    // Forcer même fr si possible
    if (move.e[0].fr != move.e[1].fr) {
      // On essaie d'inverser e[1] pour voir si ça matche
      if (move.e[0].fr == move.e[1].to) {
        vertex_t tmp = move.e[1].fr;
        move.e[1].fr = move.e[1].to;
        move.e[1].to = tmp;
      } else if (move.e[0].to == move.e[1].fr) {
        vertex_t tmp = move.e[0].fr;
        move.e[0].fr = move.e[0].to;
        move.e[0].to = tmp;
      }
    }

    bool aligned = (move.e[0].fr == move.e[1].fr);
    bool neighbors = true; // À adapter si tu veux tester plus précisément

    if (aligned && neighbors) {
      move.t = WALL;
    } else {
      move.t = NO_TYPE;
    }
  }

  return move;
}
bool place_random_valid_wall(struct scenario_t *scenario) {
  if (!scenario || !scenario->graph ||
      scenario->walls_left[scenario->current_player] == 0)
    return false;

  unsigned int n = scenario->graph->num_vertices;

  // Tente jusqu'à 100 essais
  for (int attempt = 0; attempt < 100; ++attempt) {
    vertex_t a = rand() % n;

    // Trouver ses voisins
    vertex_t neighbors[6];
    int count = 0;
    for (int i = scenario->graph->t->p[a]; i < scenario->graph->t->p[a + 1];
         ++i) {
      unsigned int val = scenario->graph->t->data[i];
      if (val != WALL_DIR && val != NO_EDGE) {
        neighbors[count++] = scenario->graph->t->i[i];
      }
    }

    // Besoin d'au moins 2 voisins pour former un mur en T
    if (count < 2)
      continue;

    // Choisir 2 voisins distincts aléatoirement
    int i = rand() % count;
    int j = rand() % count;
    if (i == j)
      continue;

    vertex_t b = neighbors[i];
    vertex_t c = neighbors[j];

    if (is_wall_valid(scenario, a, b, c)) {
      add_wall(scenario->graph, a, b);
      add_wall(scenario->graph, a, c);
      scenario->walls_left[scenario->current_player]--;
      return true;
    }
  }

  return false;
}

struct move_t play(const struct move_t previous_move) {
  // Si c’est le tout premier tour
  if (previous_move.t == NO_TYPE) {
    struct way_t path =
        shortest_path2(ayke.position, ayke.graph, ayke.visited_objectives);
    if (path.len >= 2 && path.sommets) {
      struct move_t first_move = {
          .t = MOVE, .m = path.sommets[1], .c = ayke.color};
      update_player_position(&ayke, first_move.m);
      struct scenario_t tmp_scenario = {
          .graph = ayke.graph,
          .objectives = (vertex_t *[]){ayke.my_objectives, ayke.opp_objectives},
          .objectives_visited = (vertex_t *[]){ayke.visited_objectives,
                                               ayke.opp_visited_objectives},
      };
      tmp_scenario.pos[ayke.player_id] = ayke.position;
      update_objectives_visited(&tmp_scenario, ayke.player_id);
      free(path.sommets);
      return first_move;
    }
    if (path.sommets)
      free(path.sommets);
  }
  if (previous_move.t == MOVE) {
    update_opponent_position(&ayke, previous_move.m);
  } else if (previous_move.t == WALL)
    update_player_graph(previous_move, &ayke);

  if (all_objectives_visited(ayke.visited_objectives,
                             ayke.graph->num_objectives)) {
    printf("returning home");
    vertex_t start_pos = ayke.graph->start[ayke.player_id];
    struct way_t back_path =
        shortest_path_between(ayke.position, start_pos, ayke.graph);

    if (back_path.len >= 2 && back_path.sommets) {
      vertex_t next = back_path.sommets[1];
      free(back_path.sommets);
      update_player_position(&ayke, next);
      vertex_t opp = ayke.opponent_position;
      if (next == opp) {
        enum dir_t dir =
            get_direction_between_vertices(ayke.graph, ayke.position, opp);
        vertex_t landing = get_vertex_from_direction(ayke.graph, opp, dir);
        if (landing != UINT_MAX) {
          next = landing;
        }
      }

      return (struct move_t){.t = MOVE, .m = next, .c = ayke.color};
    }

    if (back_path.sommets)
      free(back_path.sommets);

    return (struct move_t){.t = NO_TYPE, .m = ayke.position, .c = ayke.color};
  }

  // Génération du scénario initial
  struct way_t path =
      shortest_path2(ayke.position, ayke.graph, ayke.visited_objectives);

  struct move_t best_move = {.t = NO_TYPE, .m = ayke.position, .c = ayke.color};
  if (path.len >= 2 && path.sommets) {
    best_move.t = MOVE;
    best_move.m = path.sommets[1];
  }
  // Met à jour les objectifs atteints
  struct scenario_t tmp_scenario = {
      .graph = ayke.graph,
      .objectives = (vertex_t *[]){ayke.my_objectives, ayke.opp_objectives},
      .objectives_visited =
          (vertex_t *[]){ayke.visited_objectives, ayke.opp_visited_objectives},
  };
  tmp_scenario.pos[ayke.player_id] = ayke.position;
  update_objectives_visited(&tmp_scenario, ayke.player_id);
  struct graph_t *copy = make_copy(ayke.graph);
  struct scenario_t *scenario_root =
      init_first_scenario(copy, ayke.player_id, 0);

  if (!scenario_root) {
    free_graph(copy);
    if (path.sommets)
      free(path.sommets);
    return best_move;
  }

  int profondeur_max = 1;
  generate_scenario_depth(scenario_root, profondeur_max, ayke.player_id);

  struct minimax_result_t best_score = minimax(
      scenario_root, 0, INT_MIN, INT_MAX, ayke.player_id, profondeur_max);

  if (best_score.best_scenario) {
    vertex_t suggested_pos = best_score.best_scenario->pos[ayke.player_id];
    if (ayke.position != suggested_pos) {
      best_move.t = MOVE;
      best_move.m = suggested_pos;
      update_player_position(&ayke, suggested_pos);
      ayke.last_direction = best_score.best_scenario->last_direction;
      memcpy(ayke.visited_objectives,
             best_score.best_scenario->objectives_visited[ayke.player_id],
             ayke.graph->num_objectives * sizeof(vertex_t));
      struct scenario_t tmp_scenario2 = {
          .graph = ayke.graph,
          .objectives = (vertex_t *[]){ayke.my_objectives, ayke.opp_objectives},
          .objectives_visited = (vertex_t *[]){ayke.visited_objectives,
                                               ayke.opp_visited_objectives}};
      tmp_scenario2.pos[ayke.player_id] = ayke.position;
      update_objectives_visited(&tmp_scenario2, ayke.player_id);
      ayke.last_direction = best_score.best_scenario->last_direction;

    } else {
      struct move_t wall_move =
          detect_wall_difference(ayke.graph, best_score.best_scenario->graph);
      if (wall_move.t == WALL) {
        if (ayke.num_walls_left > 0) {
          best_move = wall_move;
          update_player_graph(best_move, &ayke);
          ayke.num_walls_left--;
        }
      }
    }
  }
  if (best_move.t == MOVE) {
    vertex_t opp = ayke.opponent_position;
    if (best_move.m == opp) {
      enum dir_t dir =
          get_direction_between_vertices(ayke.graph, ayke.position, opp);
      vertex_t landing = get_vertex_from_direction(ayke.graph, opp, dir);
      if (landing != UINT_MAX) {
        best_move.m = landing;
        update_player_position(&ayke, landing);
        ayke.last_direction = dir;
      } else {

        struct graph_t *g_copy = make_copy(ayke.graph);
        if (g_copy) {
          struct scenario_t *wall_scenario =
              init_first_scenario(g_copy, ayke.player_id, 0);
          if (wall_scenario) {
            wall_scenario->pos[ayke.player_id] = ayke.position;
            wall_scenario->pos[1 - ayke.player_id] = ayke.opponent_position;

            if (place_random_valid_wall(wall_scenario)) {
              struct move_t wall_move =
                  detect_wall_difference(ayke.graph, wall_scenario->graph);
              if (wall_move.t == WALL) {
                best_move = wall_move;
                update_player_graph(best_move, &ayke);
                ayke.num_walls_left--;
              }
            }

            free_scenario(wall_scenario); // libère aussi g_copy
          } else {
            free_graph(g_copy); // fallback si échec
          }
        }
      }

    } else {
      update_player_position(&ayke, best_move.m);
    }
  }
  if (path.sommets)
    free(path.sommets);
  free_scenario(scenario_root);

  return best_move;
}

void finalize() {
  free_graph(ayke.graph);
  free(ayke.visited_objectives);
  free(ayke.opp_visited_objectives);

  printf("🛑 Smart_ayke finalisé\n");
}
