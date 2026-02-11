#ifndef _CORS_PLAYER_MOUVEMENT_H_
#define _CORS_PLAYER_MOUVEMENT_H_
#include "dijkstra.h"
#include "player_utils.h"

// Déplacer le pion dans la même direction jusqu’à 3 cases. Si ça échoue
// retourner -1
vertex_t move_in_same_direction(vertex_t start, enum dir_t last_direction,
                                int max_steps, struct graph_t *graph,
                                struct player_t p);

// déplacer dans une direction à 30° du dernier mouvement a l'est , jusqu’à 2
// cases.
vertex_t move_in_30_deg_direction_EST(vertex_t start, enum dir_t last_direction,
                                      int max_steps, struct graph_t *graph,
                                      struct player_t p, vertex_t *first_step);

/* A function to determine the next dir in the clockwise order */
enum dir_t next_dir_clock(enum dir_t d);

// déplacer dans une direction à 30° du dernier mouvement a l'ouest , jusqu’à 2
// cases.
vertex_t move_in_30_deg_direction_OEUST(vertex_t start,
                                        enum dir_t last_direction,
                                        int max_steps, struct graph_t *graph,
                                        struct player_t p,
                                        vertex_t *first_step);

// Déplacer vers un voisin valide
vertex_t move_to_neighbour(vertex_t start, struct graph_t *graph,
                           struct player_t p);

// Cette fonction vérifiera si une case est libre (pas d’adversaire dessus) et
// accessible (correspond à une arête du graphe).
int is_valid_move(vertex_t start, vertex_t end, struct graph_t *graph,
                  struct player_t p);

// verifier si l'adversaire est devant son objectif sans obstacle
vertex_t opponnent_front_obj(struct graph_t *g, struct player_t p);

// Vérifier si une arête est bloquée par un mur.
int is_edge_blocked(vertex_t start, vertex_t end, struct graph_t *graph);

// sauter par-dessus l'adversaire.
vertex_t can_jump(vertex_t my_pos, vertex_t opponent_pos, struct graph_t *graph,
                  struct player_t p);

// Vérifie si un mur est placable sans bloquer un joueur
int is_wall_placeable(struct graph_t *graph, struct move_t wall);

// nombre d'arrétes bloqués bloqués par les murs autour d'un sommet
int edge_wall(struct graph_t *g, vertex_t v);

// compter le nombre d'arrétes entourés par un sommet bloqués ou non par le mur
int count_edge(struct graph_t *g, vertex_t v);

// Copmter le nombre de mur entourés par un joueur
// int count_wall(struct graph_t *g, vertex_t v);

// nombre d'arrétes bloqués par les murs autour d'un sommet
int edge_wall(struct graph_t *g, vertex_t v);

// Placer un mur sans bloquer l'adversaire oui ou non
int can_place_wall(struct graph_t *g, struct player_t p);

// placer un mur
int place_wall(struct graph_t *g, struct move_t move, struct player_t p);

// Vérifier si le mur bloque le chemin vers un sommet
int does_wall_block_v(struct graph_t *g, vertex_t v);

// Vérifier si l'adversaire est sur un sommet objectif
int is_opponent_on_objective(struct graph_t *graph, struct player_t player);

// Connaitre quel objectif on atteint par la fonction shortest_path
vertex_t obj_to_visit(struct player_t *player, struct way_t way);
#endif
