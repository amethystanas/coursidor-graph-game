#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graph_utils.h"
#include "move_utils.h"
#include "player.h"
#include "player_mouvement.h"
#include "player_utils.h"

struct player_t p_random;

char const *get_player_name() { return "p_randomPlayer"; }

void initialize(unsigned int player_id, struct graph_t *graph) {
  // srand(time(NULL));  // Active l'aléatoire
  init_player(&p_random, player_id, graph);
  printf("p_randomPlayer initialisé (joueur %d, position %u)\n", player_id,
         p_random.position);
}

struct move_t play(const struct move_t previous_move) {
  if (previous_move.t == MOVE) {
    update_opponent_position(&p_random, previous_move.m);
  }

  // Recherche des mouvements valides (6 max)
  vertex_t valid_moves[6];
  int num_valid_moves = 0;

  for (vertex_t i = 0; i < p_random.graph->num_vertices; i++) {
    enum dir_t dir =
        gsl_spmatrix_uint_get(p_random.graph->t, p_random.position, i);
    if (dir != NO_EDGE && dir != WALL_DIR && i != p_random.opponent_position) {
      valid_moves[num_valid_moves++] = i;
      if (num_valid_moves == 6)
        break;
    }
  }

  // Aucun mouvement direct possible : essayer de sauter l'adversaire
  if (num_valid_moves == 0) {
    enum dir_t jump_dir = gsl_spmatrix_uint_get(
        p_random.graph->t, p_random.position, p_random.opponent_position);
    if (jump_dir != NO_EDGE && jump_dir != WALL_DIR) {
      for (vertex_t i = 0; i < p_random.graph->num_vertices; i++) {
        if (i != p_random.position &&
            gsl_spmatrix_uint_get(p_random.graph->t, p_random.opponent_position,
                                  i) == jump_dir) {
          update_player_position(&p_random, i);
          return create_move(p_random.color, i);
        }
      }
    }

    // Vraiment bloqué
    printf("%s bloqué en position %u. Aucun mouvement possible.\n",
           get_player_name(), p_random.position);
    return create_no_move();
  }

  // Choix aléatoire parmi les mouvements valides
  vertex_t chosen = valid_moves[rand() % num_valid_moves];
  update_player_position(&p_random, chosen);
  return create_move(p_random.color, chosen);
}

void finalize() {
  free_graph(p_random.graph);
  free(p_random.visited_objectives);
  free(p_random.opp_visited_objectives);
  printf(" p_randomPlayer finalisé\n");
}
