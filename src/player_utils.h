#ifndef _CORS_PLAYER_UTILS_H_
#define _CORS_PLAYER_UTILS_H_

#include "graph.h"
#include "move.h"

#define MAX_VERTICES 100

struct player_t {
  unsigned int player_id;
  struct graph_t *graph;
  enum player_color_t color;
  vertex_t position;
  vertex_t opponent_position;
  vertex_t last_move_from;   // D'où vient le dernier mouvement
  enum dir_t last_direction; // Direction du dernier mouvement
  unsigned int num_walls_left;
  vertex_t *visited_objectives;
  vertex_t *opp_visited_objectives;
  unsigned int opp_walls_left;
  vertex_t *opp_objectives;
  vertex_t *my_objectives;
  vertex_t opp_last_position;
};

void init_player(struct player_t *player, unsigned int player_id,
                 struct graph_t *graph);
void update_player_position(struct player_t *p, vertex_t new_position);
void update_opponent_position(struct player_t *p, vertex_t new_position);
void update_player_graph(struct move_t previous_move, struct player_t *player);
#endif
