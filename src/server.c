#include <dlfcn.h>
#include <getopt.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dijkstra.h"
#include "graph.h"
#include "graph_utils.h"
#include "move_utils.h"

#define MAX_LIBS 2
#define NUM_PLAYERS 2
#define MAX_OBJECTIVES 10

struct client_t {
  void *handle;
  char const *(*get_player_name)(void);
  void (*initialize)(unsigned int, struct graph_t *);
  struct move_t (*play)(struct move_t);
  void (*finalize)(void);
  enum player_color_t color;
  vertex_t *positions;
  int index;
};

void load_client(struct client_t *p, const char *lib_path) {
  p->handle = dlopen(lib_path, RTLD_LAZY);
  if (!p->handle) {
    fprintf(stderr, "Erreur lors du chargement de %s : %s\n", lib_path,
            dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); // Clear any existing error
  *(void **)(&p->get_player_name) = dlsym(p->handle, "get_player_name");
  *(void **)(&p->initialize) = dlsym(p->handle, "initialize");
  *(void **)(&p->play) = dlsym(p->handle, "play");
  *(void **)(&p->finalize) = dlsym(p->handle, "finalize");

  char *error = dlerror();
  if (error != NULL) {
    fprintf(stderr, "Erreur lors de la récupération des symboles : %s\n",
            error);
    exit(EXIT_FAILURE);
  }
}

char *client_col(enum player_color_t color) {
  switch (color) {
  case BLACK:
    return "BLACK";
  case WHITE:
    return "WHITE";
  default:
    return "NO_COLOR";
  }
}

int win(struct client_t *c, struct graph_t *g) {
  bool completed[g->num_objectives];
  memset(completed, 0, sizeof(completed));

  // Pour chaque mouvement successif
  for (int i = 1; i < c->index; ++i) {
    vertex_t from = c->positions[i - 1];
    vertex_t to = c->positions[i];

    // Récupère la liste de sommets entre `from` et `to`
    struct way_t path = shortest_path_between(from, to, g);
    if (path.sommets) {
      for (unsigned int k = 0; k < path.len; ++k) {
        vertex_t v = path.sommets[k];
        // Marque chaque objectif atteint le long du chemin
        for (unsigned int j = 0; j < g->num_objectives; ++j) {
          if (v == g->objectives[j]) {
            completed[j] = true;
          }
        }
      }
      free(path.sommets);
    }
  }

  // Vérifie que tous les objectifs ont été cochés
  for (unsigned int j = 0; j < g->num_objectives; ++j) {
    if (!completed[j])
      return 0;
  }

  // Et que la dernière position est bien le départ
  return c->positions[c->index - 1] == g->start[c->color];
}
static int seed = 1;
static unsigned int m = 5;
static unsigned int max_turns = 10000;
static unsigned int num_players = NUM_PLAYERS;

void parse_arguments(int argc, char *argv[], char **libs) {
  int opt;
  int has_seed = 0;

  while ((opt = getopt(argc, argv, "s:m:M:h")) != -1) {
    switch (opt) {
    case 's':
      seed = atoi(optarg);
      has_seed = 1;
      break;
    case 'm':
      m = atoi(optarg);
      if (m < 2) {
        fprintf(stderr, "Erreur : la taille du graphe m doit être ≥ 2.\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'M':
      max_turns = atoi(optarg);
      if (max_turns < 1) {
        fprintf(stderr, "Erreur : le nombre de tours maximum doit être ≥ 1.\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'h':
    default:
      fprintf(stderr,
              "Usage: %s [-s seed] [-m taille_graphe] [-M max_tours] lib1.so "
              "lib2.so [...]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  // seed par défaut
  if (!has_seed) {
    seed = time(NULL);
    srand(seed);
    seed = 1129;
  }

  num_players = argc - optind;
  if (num_players < 2 || num_players > NUM_PLAYERS) {
    fprintf(stderr,
            "Erreur : vous devez fournir entre 2 et %d bibliothèques .so "
            "(joueurs).\n",
            NUM_PLAYERS);
    exit(EXIT_FAILURE);
  }

  for (unsigned int i = 0; i < num_players; ++i) {
    libs[i] = argv[optind + i];
  }
}

// Accesseurs
int get_seed() { return seed; }
unsigned int get_m() { return m; }
unsigned int get_max_turns() { return max_turns; }
unsigned int get_num_players() { return num_players; }

int main(int argc, char *argv[]) {
  char *libs[NUM_PLAYERS];

  parse_arguments(argc, argv, libs);

  srand(get_seed());
  unsigned int m = get_m();
  unsigned int MAX_TURNS = get_max_turns();

  struct graph_t *graph1 = make_graph_type_T(m);
  struct graph_t *graph2 = make_graph_type_T(m);
  struct graph_t *g[2] = {graph1, graph2};
  starting_graph_random(graph1, graph2);
  goal_graph_random(graph1, graph2);

  struct client_t *clients[NUM_PLAYERS];
  libs[0] = "./install/smartayke.so";
  libs[1] = "./install/player_wiwi.so";
  for (unsigned int i = 0; i < NUM_PLAYERS; i++) {
    clients[i] = malloc(sizeof(struct client_t));
    if (!clients[i]) {
      fprintf(stderr, "Erreur d'allocation mémoire pour le client %d\n", i);
      exit(EXIT_FAILURE);
    }
    libs[0] = "install/smartayke.so";
    libs[1] = "install/player_wiwi.so";

    load_client(clients[i], libs[i % MAX_LIBS]);
    clients[i]->initialize(i, g[i]);
    clients[i]->color = i;
    clients[i]->positions = malloc(MAX_TURNS * sizeof(vertex_t));
    clients[i]->positions[0] = g[i]->start[i];
    clients[i]->index = 1;

    printf(" Joueur %s (%s) initialisé à la position de départ %u\n",
           clients[i]->get_player_name(), client_col(i), g[i]->start[i]);
  }

  printf("\n Objectifs commun :\n");
  for (unsigned int j = 0; j < g[0]->num_objectives; ++j) {
    printf("%u ", g[0]->objectives[j]);
  }
  printf("\n");

  enum player_color_t player_col = BLACK;
  struct move_t current_move = create_no_move();
  unsigned int turn = 1;
  struct graph_t *initial1 = make_copy(graph1);
  struct graph_t *initial2 = make_copy(graph2);
  while (1) {
    printf(" Tour %d : %s joue...\n", turn, client_col(player_col));
    current_move = clients[player_col]->play(current_move);
    /********************* */
    if (player_col == 0) {
      // Vérification post-play
      assert(current_move.c == player_col && "Erreur : mauvais joueur actif !");

      // Si c'est un MOVE
      if (current_move.t == MOVE) {
        // Doit rester dans les bornes
        assert(current_move.m < g[player_col]->num_vertices &&
               "Erreur : position de destination hors bornes !");
        // Ne doit pas rester sur place
        assert(clients[player_col]->positions[clients[player_col]->index - 1] !=
                   current_move.m &&
               "Erreur : le joueur reste sur place !");
      }

      // Si c'est un WALL
      if (current_move.t == WALL) {
        for (int i = 0; i < 2; ++i) {
          vertex_t fr = current_move.e[i].fr;
          vertex_t to = current_move.e[i].to;
          if (fr != UINT_MAX && to != UINT_MAX) {
            // Interdit fr == to
            assert(fr != to && "Erreur : un mur relie un sommet à lui-même !");
            // Doit être dans les bornes
            assert(fr < g[player_col]->num_vertices &&
                   to < g[player_col]->num_vertices &&
                   "Erreur : mur en dehors du graphe !");
          }
        }
      }
    }
    /********************* */

    // assert(current_move.c == player_col);
    if (current_move.t == MOVE) {
      clients[player_col]->positions[clients[player_col]->index++] =
          current_move.m;
      printf(" 🚶 %s (%s) se déplace vers %u\n",
             clients[player_col]->get_player_name(), client_col(player_col),
             current_move.m);
    } else if (current_move.t == WALL) {
      printf("🧱%s (%s) place un mur entre (%u ↔ %u)",
             clients[player_col]->get_player_name(), client_col(player_col),
             current_move.e[0].fr, current_move.e[0].to);

      if (current_move.e[1].fr != UINT_MAX &&
          current_move.e[1].to != UINT_MAX) {
        printf(" et (%u ↔ %u)", current_move.e[1].fr, current_move.e[1].to);
      }

      printf("\n");
      /****************************************modif de
       * matrice************************************** */
      for (int p = 0; p < NUM_PLAYERS; ++p) {
        gsl_spmatrix_uint *csr = g[p]->t;

        // Conversion CSR → COO
        gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
            csr->size1, csr->size2, csr->nz, GSL_SPMATRIX_COO);

        for (size_t row = 0; row < csr->size1; ++row) {
          for (int k = csr->p[row]; k < csr->p[row + 1]; ++k) {
            gsl_spmatrix_uint_set(coo, row, csr->i[k], csr->data[k]);
          }
        }

        // Appliquer le(s) mur(s), avec vérification de validité
        for (int i = 0; i < 2; ++i) {
          if (current_move.e[i].fr != UINT_MAX &&
              current_move.e[i].to != UINT_MAX &&
              current_move.e[i].fr < g[p]->num_vertices &&
              current_move.e[i].to < g[p]->num_vertices) {
            gsl_spmatrix_uint_set(coo, current_move.e[i].fr,
                                  current_move.e[i].to, WALL_DIR);
            gsl_spmatrix_uint_set(coo, current_move.e[i].to,
                                  current_move.e[i].fr, WALL_DIR);
          }
        }

        // Recompression COO → CSR et mise à jour
        gsl_spmatrix_uint *new_csr =
            gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
        gsl_spmatrix_uint_free(coo);
        gsl_spmatrix_uint_free(g[p]->t);
        g[p]->t = new_csr;
      }

      /************************************************************************************************/
    } else if (current_move.t == NO_TYPE) {
      printf("️ Aucun mouvement valide retourné par %s (%s). Partie arrêtée.\n",
             clients[player_col]->get_player_name(), client_col(player_col));
      break;
    }
    if (win(clients[player_col], player_col == 0 ? initial1 : initial2)) {
      setlocale(LC_ALL, "");
      printf("  %s (%s) a gagné la partie en atteignant tous ses objectifs !\n",
             clients[player_col]->get_player_name(), client_col(player_col));
      break;
    }

    if (turn >= MAX_TURNS) {
      printf(" Nombre de tours maximum (%u) atteint. Partie arrêtée.\n",
             MAX_TURNS);
      break;
    }

    player_col = 1 - player_col;
    turn++;
  }

  for (unsigned int i = 0; i < NUM_PLAYERS; i++) {
    if (clients[i]) {
      clients[i]->finalize();
      free(clients[i]->positions);
      dlclose(clients[i]->handle);
      free(clients[i]);
    }
  }
  free_graph(initial1);
  free_graph(initial2);

  return 0;
}
