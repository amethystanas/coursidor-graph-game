#ifndef _CORS_MOVE_UTILS_H_
#define _CORS_MOVE_UTILS_H_

#include "move.h"

// Crée un mouvement move
struct move_t create_move(enum player_color_t color, vertex_t destination);

// Crée un move de placement de mur
struct move_t create_wall(enum player_color_t color, struct edge_t edge1,
                          struct edge_t edge2);

// crée un move initial/invalide
struct move_t create_no_move();

// Vérifie si le move (structure) est valide ou pas
int is_move_structure_valid(struct move_t move);

#endif // _CORS_MOVE_UTILS_H_