#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dijkstra.h"
#include "graph_utils.h"
#include "move_utils.h"
#include "player.h"
#include "player_mouvement.h"
#include "player_utils.h"

struct player_t wiwi;

char const *get_player_name() { return "wissal"; }

void initialize(unsigned int player_id, struct graph_t *graph) {
  init_player(&wiwi, player_id, graph);
  printf("wiwi initialisé (joueur %d, position %u)\n", player_id,
         wiwi.position);
}

struct move_t play(const struct move_t previous_move) {

  if (previous_move.t == MOVE) {
    update_opponent_position(&wiwi, previous_move.m);
  } else if (previous_move.t == WALL) {
    update_player_graph(previous_move, &wiwi);
  }
  unsigned int visited_obj = 0;
  for (unsigned int i = 0; i < wiwi.graph->num_objectives; i++) {
    visited_obj += wiwi.visited_objectives[i];
  }

  if (visited_obj == wiwi.graph->num_objectives) {
    printf("Tous les objectifs visités retour à la position de départ\n");
    struct way_t to_start = shortest_path_between(
        wiwi.position, wiwi.graph->start[wiwi.player_id], wiwi.graph);
    if (to_start.len == 0) {
      printf("Aucun chemin trouvé pour retourner à la position de départ\n");
      free(to_start.sommets);
      struct move_t move = create_no_move();
      move.c = wiwi.color;
      return move;
    }
    // Vérifier si le premier mouvement est valide
    if (!is_valid_move(wiwi.position, to_start.sommets[1], wiwi.graph, wiwi)) {
      printf("Premier mouvement invalide vers %u\n", to_start.sommets[1]);

      // Si un adversaire bloque le chemin, tenter un saut
      if (to_start.sommets[1] == wiwi.opponent_position) {
        vertex_t jump_target =
            can_jump(wiwi.position, wiwi.opponent_position, wiwi.graph, wiwi);
        if (jump_target != (vertex_t)(-1)) {
          printf("Saut réussi vers %u\n", jump_target);
          update_player_position(&wiwi, jump_target);
          free(to_start.sommets);
          return create_move(wiwi.color, jump_target);
        }
      }

      // Si un mur bloque le chemin, trouver un chemin alternatif
      printf("Tentative de trouver un chemin alternatif\n");
      struct way_t alt_path = shortest_path_between(
          wiwi.position, wiwi.graph->start[wiwi.player_id], wiwi.graph);
      if (alt_path.len > 0 &&
          is_valid_move(wiwi.position, alt_path.sommets[1], wiwi.graph, wiwi)) {
        printf("Chemin alternatif trouvé vers %u\n", alt_path.sommets[1]);
        update_player_position(&wiwi, alt_path.sommets[1]);
        vertex_t new_position = alt_path.sommets[1];
        free(to_start.sommets);
        free(alt_path.sommets);
        return create_move(wiwi.color, new_position);
      }

      free(alt_path.sommets);
      printf("Aucun chemin alternatif trouvé\n");
    } else {
      // Si le premier mouvement est valide, effectuer le déplacement
      printf("Déplacement vers la position de départ : %u\n",
             to_start.sommets[1]);
      update_player_position(&wiwi, to_start.sommets[1]);
      vertex_t new_position = to_start.sommets[1];
      free(to_start.sommets);
      return create_move(wiwi.color, new_position);
    }

    free(to_start.sommets);
    printf("Aucun mouvement valide pour retourner à la position de départ\n");
    struct move_t move = create_no_move();
    move.c = wiwi.color;
    return move;
  }
  struct way_t way =
      shortest_path2(wiwi.position, wiwi.graph, wiwi.visited_objectives);
  vertex_t next_objective = obj_to_visit(&wiwi, way);
  if (way.len == 0 && visited_obj < wiwi.graph->num_objectives) {
    printf("Aucun chemin trouvé\n");
    printf("Objectifs restants : ");
    for (unsigned int i = 0; i < wiwi.graph->num_objectives; i++) {
      if (!wiwi.visited_objectives[i]) {
        printf("%u ", wiwi.graph->objectives[i]);
        does_wall_block_v(wiwi.graph, wiwi.graph->objectives[i]);
      }
    }
    printf("\n");
    free(way.sommets);
    struct move_t move = create_no_move();
    move.c = wiwi.color;
    return move;
  }
  // placer un mur si l'adversaire est devant son objectif mais faut pas que
  // l'objectif soit inaccessible
  vertex_t u = opponnent_front_obj(wiwi.graph, wiwi);
  if (wiwi.num_walls_left != 0 && u != (vertex_t)(-1) &&
      can_place_wall(wiwi.graph, wiwi) && !does_wall_block_v(wiwi.graph, u)) {
    printf("attention ladversaire est devant sans objectif\n");
    struct move_t wall_move = {.c = wiwi.color, .t = WALL};
    enum dir_t dir = next_dir(
        gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.opponent_position, u));
    if (dir > LAST_DIR) {
      dir = next_dir_clock(
          gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.opponent_position, u));
    }

    // Trouver l autre voisin du mur
    vertex_t neighbor = UINT_MAX;
    for (vertex_t v = 0; v < wiwi.graph->num_vertices; v++) {
      if (gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.opponent_position, v) ==
          dir) {
        neighbor = v;
        break;
      }
    }
    if (neighbor != UINT_MAX) {
      wall_move.e[0].fr = wiwi.opponent_position;
      wall_move.e[0].to = u;
      wall_move.e[1].fr = wiwi.opponent_position;
      wall_move.e[1].to = neighbor;

      if (place_wall(wiwi.graph, wall_move, wiwi) == 1) {
        printf("Mur placé pour bloquer l'adversaire devant son objectif\n");
        update_player_graph(wall_move, &wiwi);
        free(way.sommets);
        return wall_move;
      } else {
        printf("Échec du placement du mur\n");
      }
    }
  }

  // sauter s il y a un adversaire sur le chemin
  if (way.sommets[1] == wiwi.opponent_position) {
    printf("il y a un adversaire sur le chemin\n");
    // Vérifier si l'adversaire est sur un sommet objectif
    if ((is_opponent_on_objective(wiwi.graph, wiwi) &&
         wiwi.opponent_position == next_objective) ||
        can_jump(wiwi.position, wiwi.opponent_position, wiwi.graph, wiwi) ==
            (vertex_t)(-1)) {
      printf("L'adversaire est sur un sommet objectif %u, saut interdit\n",
             wiwi.opponent_position);

      // Parcourir tous les objectifs non atteints par l'adversaire
      for (unsigned int i = 0; i < wiwi.graph->num_objectives; i++) {
        if (!wiwi.opp_visited_objectives[i]) {
          vertex_t other_objective = wiwi.graph->objectives[i];
          printf("Tentative de placement d'un mur pour ralentir l'adversaire "
                 "sur l'objectif %u\n",
                 other_objective);

          if (wiwi.num_walls_left > 0 && can_place_wall(wiwi.graph, wiwi) &&
              !does_wall_block_v(wiwi.graph, other_objective)) {
            struct move_t wall_move = {.c = wiwi.color, .t = WALL};

            // Trouver une direction pour placer le mur
            enum dir_t dir = next_dir(gsl_spmatrix_uint_get(
                wiwi.graph->t, wiwi.opponent_position, other_objective));
            if (dir > LAST_DIR) {
              dir = next_dir_clock(gsl_spmatrix_uint_get(
                  wiwi.graph->t, wiwi.opponent_position, other_objective));
            }

            // Trouver l'autre voisin du mur
            vertex_t neighbor = UINT_MAX;
            for (vertex_t v = 0; v < wiwi.graph->num_vertices; v++) {
              if (gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.opponent_position,
                                        v) == dir) {
                neighbor = v;
                break;
              }
            }

            if (neighbor != UINT_MAX) {
              wall_move.e[0].fr = wiwi.opponent_position;
              wall_move.e[0].to = other_objective;
              wall_move.e[1].fr = wiwi.opponent_position;
              wall_move.e[1].to = neighbor;

              if (place_wall(wiwi.graph, wall_move, wiwi) == 1) {
                printf("Mur placé pour ralentir l'adversaire\n");
                update_player_graph(wall_move, &wiwi);
                free(way.sommets);
                return wall_move;
              } else {
                printf("Échec du placement du mur sur l'objectif %u\n",
                       other_objective);
              }
            }
          }
        }
      }

      // Si aucun mur ne peut être placé, agir au lieu d'attendre
      printf("Aucun mur ne peut être placé. Tentative d'action proactive\n");

      // Vérifier si des murs sont disponibles
      if (wiwi.num_walls_left > 0 && can_place_wall(wiwi.graph, wiwi)) {
        printf("Placement d'un mur pour gêner l'adversaire\n");
        struct move_t wall_move = {.c = wiwi.color, .t = WALL};

        // Trouver une direction pour placer le mur
        enum dir_t dir = next_dir(wiwi.last_direction);
        vertex_t neighbor = UINT_MAX;

        // Trouver un voisin valide pour placer le mur
        for (vertex_t v = 0; v < wiwi.graph->num_vertices; v++) {
          if (gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.position, v) == dir) {
            neighbor = v;
            break;
          }
        }

        if (neighbor != UINT_MAX) {
          wall_move.e[0].fr = wiwi.position;
          wall_move.e[0].to = neighbor;
          wall_move.e[1].fr = wiwi.position;
          wall_move.e[1].to = next_dir_clock(neighbor);

          if (place_wall(wiwi.graph, wall_move, wiwi) == 1) {
            printf("Mur placé avec succès\n");
            update_player_graph(wall_move, &wiwi);
            free(way.sommets);
            return wall_move;
          } else {
            printf("Échec du placement du mur\n");
          }
        }
      }

      // Si aucun mur ne peut être placé, se déplacer vers un sommet voisin
      // valide
      printf("Aucun mur disponible, déplacement vers un sommet voisin\n");
      for (vertex_t v = 0; v < wiwi.graph->num_vertices; v++) {
        if (is_valid_move(wiwi.position, v, wiwi.graph, wiwi)) {
          printf("Déplacement vers le sommet voisin %u\n", v);
          update_player_position(&wiwi, v);
          free(way.sommets);
          return create_move(wiwi.color, v);
        }
      }

    } else {
      vertex_t v =
          can_jump(wiwi.position, wiwi.opponent_position, wiwi.graph, wiwi);
      if (v != (vertex_t)(-1)) {
        printf("wiwi a sauté son adversaire en sautant vers %u\n", v);
        update_player_position(&wiwi, v);
        free(way.sommets);
        return create_move(wiwi.color, v);
      }
    }
  }

  // Déplacement dans la même direction
  if (gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.position, way.sommets[1]) ==
          wiwi.last_direction &&
      is_valid_move(wiwi.position, way.sommets[1], wiwi.graph, wiwi)) {
    for (int steps = 3; steps > 0; --steps) {
      vertex_t new_position = move_in_same_direction(
          wiwi.position, wiwi.last_direction, steps, wiwi.graph, wiwi);
      if (new_position != (vertex_t)(-1)) {
        printf("Déplacement dans la même direction de %d pas\n", steps);
        update_player_position(&wiwi, new_position);
        free(way.sommets);
        return create_move(wiwi.color, new_position);
      }
    }
  }

  // Déplacement à 30° ouest
  if (gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.position, way.sommets[1]) ==
          next_dir(wiwi.last_direction) &&
      is_valid_move(wiwi.position, way.sommets[1], wiwi.graph, wiwi)) {
    for (int step = 2; step > 0; step--) {
      vertex_t first_step;
      vertex_t new_position =
          move_in_30_deg_direction_OEUST(wiwi.position, wiwi.last_direction,
                                         step, wiwi.graph, wiwi, &first_step);
      if (new_position != (vertex_t)(-1)) {
        // Met à jour la direction avec le premier pas
        wiwi.last_direction =
            gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.position, first_step);
        update_player_position(&wiwi, new_position);
        free(way.sommets);
        return create_move(wiwi.color, new_position);
      }
    }
  }
  // Déplacement à 30° est
  if (gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.position, way.sommets[1]) ==
          next_dir_clock(wiwi.last_direction) &&
      is_valid_move(wiwi.position, way.sommets[1], wiwi.graph, wiwi)) {
    for (int step = 2; step > 0; step--) {
      vertex_t first_step;
      vertex_t new_position =
          move_in_30_deg_direction_EST(wiwi.position, wiwi.last_direction, step,
                                       wiwi.graph, wiwi, &first_step);
      if (new_position != (vertex_t)(-1)) {
        wiwi.last_direction =
            gsl_spmatrix_uint_get(wiwi.graph->t, wiwi.position, first_step);
        update_player_position(&wiwi, new_position);
        free(way.sommets);
        return create_move(wiwi.color, new_position);
      }
    }
  }

  // sinon, on se déplace vers le sommet suivant valide
  if (is_valid_move(wiwi.position, way.sommets[1], wiwi.graph, wiwi)) {
    printf("Déplacement vers le sommet %u\n", way.sommets[1]);
    update_player_position(&wiwi, way.sommets[1]);
    vertex_t new_position = way.sommets[1];
    free(way.sommets);
    return create_move(wiwi.color, new_position);
  }

  free(way.sommets);
  printf("Aucun mouvement valide possible\n");
  struct move_t move = create_no_move();
  move.c = wiwi.color;
  return move;
}

void finalize() {
  free_graph(wiwi.graph);
  free(wiwi.visited_objectives);

  free(wiwi.opp_visited_objectives);
}
