#include "test_move.h"

void test_create_move() {

  printf("%s", __func__);
  enum player_color_t color = BLACK;
  vertex_t destination = 42;

  struct move_t move = create_move(color, destination);

  assert(move.c == BLACK);
  assert(move.t == MOVE);
  assert(move.m == 42);

  printf(" passed\n");
}

void test_create_wall() {

  printf("%s", __func__);
  enum player_color_t color = WHITE;
  struct edge_t edge1 = {5, 6};
  struct edge_t edge2 = {5, 7};

  struct move_t move = create_wall(color, edge1, edge2);

  assert(move.c == WHITE);
  assert(move.t == WALL);
  assert(move.e[0].fr == 5);
  assert(move.e[0].to == 6);
  assert(move.e[1].fr == 5);
  assert(move.e[1].to == 7);

  printf(" passed\n");
}

void test_create_no_move() {

  printf("%s", __func__);

  struct move_t move = create_no_move();

  assert(move.c == NO_COLOR);
  assert(move.t == NO_TYPE);

  printf(" passed\n");
}

void test_is_move_structure_valid() {

  printf("%s", __func__);

  struct move_t valid_move = create_move(BLACK, 42);
  struct move_t invalid_move = create_no_move();

  assert(is_move_structure_valid(valid_move) == 1);
  assert(is_move_structure_valid(invalid_move) == 0);

  printf(" passed\n");
}
