
#include "test_player_mouvement.h"

void test_move_in_same_direction() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  enum dir_t last_direction1 = E;
  enum dir_t last_direction2 = W;
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  vertex_t start1 = p1->position;
  vertex_t start2 = p2->position;
  vertex_t next_direction1 =
      move_in_same_direction(start1, last_direction1, 1, g1, *p1);
  vertex_t next_direction2 =
      move_in_same_direction(start2, last_direction2, 1, g2, *p2);
  assert(next_direction1 == 1 && next_direction2 == 16);
  printf("test_move_in_same_direction passed\n");
  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}

void test_move_in_30_deg_direction_OUEST() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  enum dir_t last_direction1 = SE;
  enum dir_t last_direction2 = E;
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  vertex_t start1 = p1->position;
  vertex_t start2 = p2->position;
  vertex_t tmp;
  vertex_t next_direction1 =
      move_in_30_deg_direction_OEUST(start1, last_direction1, 1, g1, *p1, &tmp);
  vertex_t tmp1;
  vertex_t next_direction2 = move_in_30_deg_direction_OEUST(
      start2, last_direction2, 1, g2, *p2, &tmp1);
  assert(next_direction1 == 1 && next_direction2 == 14);

  printf("test_move_in_30_deg_direction_OUEST passed\n");
  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}
void test_move_in_30_deg_direction_EST() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  enum dir_t last_direction1 = E;
  enum dir_t last_direction2 = NE;
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  vertex_t start1 = p1->position;
  vertex_t start2 = p2->position;

  vertex_t tmp3;
  vertex_t next_direction1 =
      move_in_30_deg_direction_EST(start1, last_direction1, 1, g1, *p1, &tmp3);
  vertex_t tmp4;
  vertex_t next_direction2 =
      move_in_30_deg_direction_EST(start2, last_direction2, 1, g2, *p2, &tmp4);
  assert(next_direction1 == 4 && next_direction2 == 18);
  printf("test_move_in_30_deg_direction_EST passed\n");
  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}
void test_move_to_neighbour() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  vertex_t start1 = p1->position;
  vertex_t start2 = p2->position;

  vertex_t next_neihbor1 = move_to_neighbour(start1, g1, *p1);
  vertex_t next_neighbor2 = move_to_neighbour(start2, g2, *p2);

  assert(next_neihbor1 == 1 && next_neighbor2 == 13);
  printf("test_move_to_neighbor passed\n");

  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}

void test_is_valid_move() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  p2->position = 4;
  vertex_t start1 = p1->position;
  vertex_t start2 = p2->position;
  vertex_t end1 = start2;
  vertex_t end2 = start1;
  vertex_t far_vertex = 7;
  vertex_t valid1 = 1;
  vertex_t valid2 = 3;
  p1->opponent_position = p2->position;
  p2->opponent_position = p1->position;
  int non_valide_test_opp1 = is_valid_move(start1, end1, g1, *p1);
  int non_valide_test_opp2 = is_valid_move(start2, end2, g2, *p2);
  int non_valide_test_far1 = is_valid_move(start1, far_vertex, g1, *p1);
  int non_valide_test_far2 = is_valid_move(start2, far_vertex, g2, *p2);
  int valide_test1 = is_valid_move(start1, valid1, g1, *p1);
  int valide_test2 = is_valid_move(start2, valid2, g2, *p2);

  assert(non_valide_test_opp1 == 0 && non_valide_test_opp2 == 0 &&
         non_valide_test_far1 == 0 && non_valide_test_far2 == 0 &&
         valide_test1 == 1 && valide_test2 == 1);
  printf("test_is_valid_move passed\n");
  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}

void test_opponnent_front_obj() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  p2->position = 4;
  p1->opponent_position = p2->position;
  g1->num_objectives = 3;
  g2->num_objectives = 3;
  g1->objectives[0] = 5;
  g1->objectives[1] = 6;
  g1->objectives[2] = 17;
  vertex_t front_obj = opponnent_front_obj(g1, *p1);
  assert(front_obj == 5);
  printf("test_opponent_front_obj passed\n");

  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}

void test_can_jump() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  p2->position = 1;
  p1->opponent_position = p2->position;
  vertex_t jump_valid = can_jump(p1->position, p2->position, g1, *p1);
  assert(jump_valid == 2);
  p2->position = 17;
  p1->opponent_position = p2->position;
  vertex_t jump_no_valid = can_jump(p1->position, p2->position, g1, *p1);
  assert(jump_no_valid == (vertex_t)(-1));
  printf("test_can_jump passed\n");
  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}

void test_can_place_wall() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  p2->position = 2;
  p1->opponent_position = p2->position;
  enum player_color_t color = p1->color;
  struct edge_t edge1 = {0, 1};
  struct edge_t edge3 = {2, 1};
  struct edge_t edge4 = {2, 5};
  struct edge_t edge2 = {0, 4};
  struct edge_t edge5 = {11, 10};
  struct edge_t edge6 = {11, 6};
  struct move_t create_wall1 = create_wall(color, edge1, edge2);
  struct move_t create_wall2 = create_wall(color, edge3, edge4);
  struct move_t create_wall3 = create_wall(color, edge5, edge6);
  int test_place_wall1 = place_wall(g1, create_wall1, *p1);
  int test_place_wall2 = place_wall(g1, create_wall2, *p1);
  int test_place_wall3 = place_wall(g1, create_wall3, *p1);
  assert(test_place_wall1 == 1 && test_place_wall2 == 1 &&
         test_place_wall3 == -1);
  printf("test_can_place_wall passed\n");
  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}

void test_place_wall() {
  struct graph_t *g1 = make_graph_type_T(3);
  struct graph_t *g2 = make_graph_type_T(3);
  struct player_t *p1 = malloc(sizeof(struct player_t));
  struct player_t *p2 = malloc(sizeof(struct player_t));
  starting_graph_not_random(g1, g2);
  goal_graph_random(g1, g2);
  init_player(p1, 0, g1);
  init_player(p2, 0, g2);
  struct edge_t edge1 = {0, 1};
  struct edge_t edge2 = {0, 4};
  struct edge_t edge4 = {2, 9};
  struct edge_t edge3 = {4, 8};
  enum player_color_t color = p1->color;
  struct move_t create_wall1 = create_wall(color, edge1, edge2);
  struct move_t create_wall2 = create_wall(color, edge4, edge3);
  int test_place_wall = place_wall(g1, create_wall1, *p1);
  int test_place_wall_already_wall = place_wall(g1, create_wall1, *p1);
  int test_place_wall_wrong_edges = place_wall(g1, create_wall2, *p1);
  assert(test_place_wall == 1 && test_place_wall_already_wall == -1 &&
         test_place_wall_wrong_edges == -1);
  printf("test_place_wall passed\n");
  free(p1->visited_objectives);
  free(p2->visited_objectives);
  free(p1->opp_visited_objectives);
  free(p2->opp_visited_objectives);
  free(p1);
  free(p2);
  free_graph(g1);
  free_graph(g2);
}
