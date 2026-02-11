#include "player_utils.h"
#include "move_utils.h"
#include <stddef.h>

void init_player(struct player_t *player, unsigned int player_id,
                 struct graph_t *graph) {
  if (!graph || !player || player_id > 1) {
    fprintf(stderr, "Erreur init_player: arguments invalides\n");
    return;
  }

  if (graph == NULL) {
    fprintf(stderr, "Invalid graph\n");
    return;
  }
  if (player == NULL) {
    fprintf(stderr, "Invalid player\n");
    return;
  }
  if (player_id > 1) {
    fprintf(stderr, "Invalid player_id\n");
    return;
  }
  switch (player_id) {
  case 1:
    break;
  case 0:
    break;
  }

  player->player_id = player_id;
  player->color = (enum player_color_t)player_id;
  player->graph = graph;
  player->position = graph->start[player_id];
  player->opponent_position = graph->start[1 - player_id];
  player->opp_last_position = graph->start[1 - player_id];
  player->last_move_from = player->position;
  player->last_direction = NO_EDGE;

  player->num_walls_left = graph->num_edges / 16;
  player->opp_walls_left = graph->num_edges / 16;

  // Objectifs
  player->my_objectives = graph->objectives;
  player->opp_objectives = graph->objectives;

  // Objectifs visités
  player->visited_objectives = calloc(graph->num_objectives, sizeof(vertex_t));
  player->opp_visited_objectives =
      calloc(graph->num_objectives, sizeof(vertex_t));
}

void update_player_position(struct player_t *p, vertex_t new_position) {
  if (!p) {
    return;
  }

  if (p->position != new_position) {
    enum dir_t dir =
        gsl_spmatrix_uint_get(p->graph->t, p->position, new_position);
    if (dir != NO_EDGE && dir != WALL_DIR) {
      p->last_direction = dir;
    }
    p->last_move_from = p->position;
    p->position = new_position;
  }
  for (unsigned int i = 0; i < p->graph->num_objectives; i++) {
    if (p->position == p->graph->objectives[i]) {
      p->visited_objectives[i] = 1;
    }
  }
}

void update_opponent_position(struct player_t *p, vertex_t new_position) {
  if (!p || !p->graph || !p->opp_objectives || !p->opp_visited_objectives)
    return;

  // Mise à jour des positions
  p->opp_last_position = p->opponent_position;
  p->opponent_position = new_position;

  // Mise à jour des objectifs atteints
  unsigned int nb_obj = p->graph->num_objectives;
  for (unsigned int i = 0; i < nb_obj; ++i) {
    if (!p->opp_visited_objectives[i] &&
        p->opponent_position == p->opp_objectives[i]) {
      p->opp_visited_objectives[i] = 1;
    }
  }
}

void update_player_graph(struct move_t previous_move, struct player_t *player) {
  if (!player || !player->graph || !player->graph->t || previous_move.t != WALL)
    return;

  gsl_spmatrix_uint *csr = player->graph->t;

  // Conversion CSR → COO
  gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
      csr->size1, csr->size2, csr->nz, GSL_SPMATRIX_COO);

  for (size_t row = 0; row < csr->size1; ++row) {
    for (int k = csr->p[row]; k < csr->p[row + 1]; ++k) {
      gsl_spmatrix_uint_set(coo, row, csr->i[k], csr->data[k]);
    }
  }

  // Application des murs avec vérifications
  for (int i = 0; i < 2; ++i) {
    vertex_t fr = previous_move.e[i].fr;
    vertex_t to = previous_move.e[i].to;

    if (fr != UINT_MAX && to != UINT_MAX && fr < player->graph->num_vertices &&
        to < player->graph->num_vertices) {
      gsl_spmatrix_uint_set(coo, fr, to, WALL_DIR);
      gsl_spmatrix_uint_set(coo, to, fr, WALL_DIR);
    } else {
      fprintf(stderr, "⛔ Ignoré : indices de mur invalides (%u ↔ %u)\n", fr,
              to);
    }
  }

  // Recompression COO → CSR
  gsl_spmatrix_uint *new_csr =
      gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(coo);
  gsl_spmatrix_uint_free(player->graph->t);
  player->graph->t = new_csr;
}
