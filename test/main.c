#include "test_dijkstra.h"
#include "test_graph.h"
#include "test_move.h"
#include "test_player_mouvement.h"
#include <stdio.h>

int main() {
  printf("\e[0;33m===Run Tests===\e[0;37m\n");
  // test_initialization();
  // test_extractMin();
  test_dijkstra();
  test_shortest_path();
  test_make_graph_type_T();
  test_is_in_graph_type_T();
  test_starting_graph_random();
  test_goal_graph_random();
  test_apply_wall_move();
  test_make_copy();
  test_move_in_same_direction();
  test_move_in_30_deg_direction_OUEST();
  test_move_in_30_deg_direction_EST();
  test_move_to_neighbour();
  test_is_valid_move();
  test_place_wall();
  test_can_place_wall();
  test_create_move();
  test_create_wall();
  test_create_no_move();
  test_is_move_structure_valid();
  test_opponnent_front_obj();
  test_can_jump();
  // test_make_graph_type_C();
  printf(" ALL TESTS PASSED!\n");

  return 0;
}
