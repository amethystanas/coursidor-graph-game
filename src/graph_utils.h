#ifndef _CORS_GRAPH_UTILS_H_
#define _CORS_GRAPH_UTILS_H_
#include "graph.h"
#include <assert.h>
#include <gsl/gsl_spmatrix.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* fonction qui cree le graph type T */
struct graph_t *make_graph_type_T(unsigned int m);

/* fonction qui verifie si un sommet appartient au graph type T */
bool is_in_graph_type_T(int col, int row, int m);

/* fonction qui cree le graph type C */
struct graph_t *make_graph_type_C(unsigned int m);

/* fonction qui verifie si un sommet appartient au graph type C */
bool is_in_graph_type_C(int col, int row, int m);

/* fonction qui cree le graph type H */
struct graph_t *make_graph_type_H(unsigned int m);

/* fonction qui verifie si un sommet appartient au graph type H */
bool is_in_graph_type_H(int col, int row, int m);

/*fonction qui libere la memoire allouer pendant la creation du graph */
void free_graph(struct graph_t *g);

/*fonction qui affiche les eleemnt d'un graph et les direction entre chaque
sommet */
void print_graph(const struct graph_t *g);

/*fonction qui choisi au hasard de point de departs dans le graphe*/
void starting_graph_random(struct graph_t *g1, struct graph_t *g2);

/*fonction qui choisi comme point de depart le premier et le dernier point du
 * graphe*/
void starting_graph_not_random(struct graph_t *g1, struct graph_t *g2);

/*fonction qui choisi les objectifs des joueurs aléatoirement*/
void goal_graph_random(struct graph_t *g1, struct graph_t *g2);

/* fonction qui met à jour le graphe pour les mouvements de type WALL
(ajout d'un mur entre deux sommets)*/
void apply_wall_move(struct graph_t *g, struct move_t move);

/* fonction qui crée une copie du graphe */
struct graph_t *make_copy(struct graph_t *g);

/* fonction qui affiche un graphe */
void print_hex_grid(struct graph_t *g);
#endif
