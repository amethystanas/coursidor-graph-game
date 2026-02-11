#include "test_graph.h"

void test_make_graph_type_T() {
  printf("%s\n", __func__);
  unsigned int m = 5;
  struct graph_t *g = make_graph_type_T(m);
  assert(g != NULL);
  assert(g->num_vertices == 3 * m * m - 3 * m + 1);
  print_hex_grid(g);
  free_graph(g);
}

void test_is_in_graph_type_T() {
  printf("%s\n", __func__);
  unsigned int m = 5;
  assert(is_in_graph_type_T(0, 0, m) == true);
  assert(is_in_graph_type_T(1, 1, m) == true);
  assert(is_in_graph_type_T(2, 2, m) == true);
  assert(is_in_graph_type_T(3, 3, m) == false);
  assert(is_in_graph_type_T(-3, -2, m) == false);
}

void test_starting_graph_random() {
  printf("%s\n", __func__);
  unsigned int m = 4;
  struct graph_t *g1 = make_graph_type_T(m);
  struct graph_t *g2 = make_graph_type_T(m);

  vertex_t initial_start1_0 = g1->start[0];
  vertex_t initial_start1_1 = g1->start[1];
  vertex_t initial_start2_0 = g2->start[0];
  vertex_t initial_start2_1 = g2->start[1];

  starting_graph_random(g1, g2);

  assert(g1->start[0] < g1->num_vertices);
  assert(g1->start[1] < g1->num_vertices);

  // verifier si les positions de départ sont similaires
  assert(g1->start[0] == g2->start[0]);
  assert(g1->start[1] == g2->start[1]);

  // verifier si les positions de départ ont changé
  assert(g1->start[0] != initial_start1_0 || g1->start[1] != initial_start1_1 ||
         g2->start[0] != initial_start2_0 || g2->start[1] != initial_start2_1);

  free_graph(g1);
  free_graph(g2);
}

void test_goal_graph_random() {
  printf("%s\n", __func__);
  unsigned int m = 4;
  struct graph_t *g1 = make_graph_type_T(m);
  struct graph_t *g2 = make_graph_type_T(m);

  starting_graph_not_random(g1, g2);

  assert(g1->objectives == NULL);
  assert(g2->objectives == NULL);
  assert(g1->num_objectives == 0);

  goal_graph_random(g1, g2);

  assert(g1->num_objectives == 3); // 3 objectives par défaut
  assert(g1->objectives != NULL);
  assert(g2->objectives != NULL);

  for (unsigned int i = 0; i < g1->num_objectives; i++) {
    assert(g1->objectives[i] == g2->objectives[i]);
  }

  // verifier si les objectifs sont différents des positions de départ
  for (unsigned int i = 0; i < g1->num_objectives; i++) {
    assert(g1->objectives[i] != g1->start[0]);
    assert(g1->objectives[i] != g1->start[1]);
  }

  free_graph(g1);
  free_graph(g2);
}

void test_apply_wall_move() {
  printf("%s\n", __func__);
  unsigned int m = 4;
  struct graph_t *g = make_graph_type_T(m);

  // Create a wall move
  struct move_t wall_move;
  wall_move.t = WALL;
  wall_move.c = BLACK;
  wall_move.e[0].fr = 0;
  wall_move.e[0].to = 1;
  wall_move.e[1].fr = 0;
  wall_move.e[1].to = 5;

  // initialement, pas de mur
  assert(gsl_spmatrix_uint_get(g->t, 0, 1) != WALL_DIR);
  assert(gsl_spmatrix_uint_get(g->t, 0, 5) != WALL_DIR);

  apply_wall_move(g, wall_move);

  // verifier que le mur a été appliqué
  assert(gsl_spmatrix_uint_get(g->t, 0, 1) == WALL_DIR);
  assert(gsl_spmatrix_uint_get(g->t, 1, 0) == WALL_DIR);
  assert(gsl_spmatrix_uint_get(g->t, 0, 5) == WALL_DIR);
  assert(gsl_spmatrix_uint_get(g->t, 5, 0) == WALL_DIR);

  free_graph(g);
}

void test_make_copy() {
  printf("%s\n", __func__);
  unsigned int m = 4;
  struct graph_t *original = make_graph_type_T(m);

  starting_graph_not_random(original, original);
  struct graph_t *g1 =
      make_graph_type_T(m); // pour l'utiliser dans goal_graph_random
  goal_graph_random(original, g1);
  free_graph(g1);

  struct move_t wall_move;
  wall_move.t = WALL;
  wall_move.c = BLACK;
  wall_move.e[0].fr = 0;
  wall_move.e[0].to = 1;
  wall_move.e[1].fr = 0;
  wall_move.e[1].to = 5;
  apply_wall_move(original, wall_move);

  struct graph_t *copy = make_copy(original);

  assert(copy != NULL);

  // verifier que le graphe copié a les mêmes propriétés
  assert(copy->num_vertices == original->num_vertices);
  assert(copy->num_edges == original->num_edges);
  assert(copy->type == original->type);
  assert(copy->start[0] == original->start[0]);
  assert(copy->start[1] == original->start[1]);
  assert(copy->num_objectives == original->num_objectives);

  for (unsigned int i = 0; i < original->num_objectives; i++) {
    assert(copy->objectives[i] == original->objectives[i]);
  }

  assert(copy->t != NULL);
  assert(copy->t->size1 == original->t->size1);
  assert(copy->t->size2 == original->t->size2);
  assert(copy->t->nz == original->t->nz);

  assert(gsl_spmatrix_uint_get(copy->t, 0, 1) == WALL_DIR);
  assert(gsl_spmatrix_uint_get(copy->t, 0, 5) == WALL_DIR);

  // verifier que modifier le graphe copié ne modifie pas l'original
  struct move_t another_wall;
  another_wall.t = WALL;
  another_wall.c = WHITE;
  another_wall.e[0].fr = 10;
  another_wall.e[0].to = 11;
  another_wall.e[1].fr = 10;
  another_wall.e[1].to = 15;
  apply_wall_move(copy, another_wall);

  assert(gsl_spmatrix_uint_get(copy->t, 10, 11) == WALL_DIR);
  assert(gsl_spmatrix_uint_get(original->t, 10, 11) != WALL_DIR);

  free_graph(original);
  free_graph(copy);
}
