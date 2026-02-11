#include "move_utils.h"

struct move_t create_move(enum player_color_t color, vertex_t destination) {
  struct move_t move;
  move.c = color;
  move.t = MOVE;
  move.m = destination;
  return move;
}

struct move_t create_wall(enum player_color_t color, struct edge_t edge1,
                          struct edge_t edge2) {
  struct move_t move;
  move.c = color;
  move.t = WALL;
  move.e[0] = edge1;
  move.e[1] = edge2;
  return move;
}

struct move_t create_no_move() {
  struct move_t move;
  move.c = NO_COLOR;
  move.t = NO_TYPE;
  return move;
}

int is_move_structure_valid(struct move_t move) {

  if (move.c == NO_COLOR || move.t == NO_TYPE) {
    return 0;
  }
  return 1;
}
