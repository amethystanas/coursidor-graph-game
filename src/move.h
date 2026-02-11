#ifndef _CORS_MOVE_H_
#define _CORS_MOVE_H_

#include <stddef.h>
#include <stdint.h>

/* le nombre de joueurs */
#define NUM_PLAYERS 2

/* Les couleurs possible pour les joueurs */
enum player_color_t { BLACK = 0, WHITE = 1, NO_COLOR = 2 };

/* les differeent type possible  */
enum move_type_t { NO_TYPE = 0, WALL = 1, MOVE = 2 };

/* Les ids des sommet dans le graphe allant de 0 jusqu'a nbr de sommet -1*/
typedef unsigned int vertex_t;

/* la representation d,un sommet dans le graph*/
struct edge_t {
  vertex_t fr, to;
};

/* un mouvement dans le jeu*/
struct move_t {
  enum player_color_t c;
  enum move_type_t t;
  vertex_t m;
  struct edge_t e[2];
};

#endif // _CORS_MOVE_H_
