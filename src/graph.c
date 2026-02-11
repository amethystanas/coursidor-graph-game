#include "graph_utils.h"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define VIOLET "\033[1;35m"
#define RESET "\033[0m"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Ajoute une arête bidirectionnelle avec direction
static void add_bidirectional_edge(gsl_spmatrix_uint *t, size_t i, size_t j,
                                   enum dir_t d) {
  gsl_spmatrix_uint_set(t, i, j, d);
  gsl_spmatrix_uint_set(t, j, i, opposite_dir(d));
}

// renvoyer la chaine de caractères correspondant à la direction
static char *direction_to_string(enum dir_t dir) {
  char *s = NULL;
  switch (dir) {
  case NW:
    s = "NW";
    break;
  case NE:
    s = "NE";
    break;
  case E:
    s = "E";
    break;
  case SE:
    s = "SE";
    break;
  case SW:
    s = "SW";
    break;
  case W:
    s = "W";
    break;
  default:
    s = "NO_EDGE";
    break;
  }
  return s;
}

// Structure pour les positions
struct Position {
  int row;
  int col;
};

static double distance_euclidienne_position(struct Position x,
                                            struct Position y) {
  return sqrt((x.row - y.row) * (x.row - y.row) +
              (x.col - y.col) * (x.col - y.col));
}

bool is_in_graph_type_T(int col, int row, int m) {
  return (-m < col && col < m && -m < row && row < m && -m < row + col &&
          row + col < m);
}

// Créer un graphe de type T
struct graph_t *make_graph_type_T(unsigned int m) {
  int N = 3 * m * m - 3 * m + 1;
  gsl_spmatrix_uint *t = gsl_spmatrix_uint_alloc(N, N);
  struct Position *positions = malloc(sizeof(struct Position) * N);

  int count = 0;
  for (int row = -((int)m) + 1; row < (int)m; ++row) {
    for (int col = -((int)m) + 1; col < (int)m; ++col) {
      if (is_in_graph_type_T(col, row, m)) {
        positions[count].row = row;
        positions[count].col = col;
        count++;
      }
    }
  }

  struct {
    int drow, dcol;
    enum dir_t dir;
  } directions[] = {{0, 1, E},   {0, -1, W},  {-1, 0, NW},
                    {-1, 1, NE}, {1, -1, SW}, {1, 0, SE}};

  for (int i = 0; i < N; ++i) {
    int row = positions[i].row;
    int col = positions[i].col;
    for (int k = 0; k < 6; ++k) {
      int r2 = row + directions[k].drow;
      int c2 = col + directions[k].dcol;
      for (int j = 0; j < N; ++j) {
        if (positions[j].row == r2 && positions[j].col == c2) {
          add_bidirectional_edge(t, i, j, directions[k].dir);
          break;
        }
      }
    }
  }

  struct graph_t *g = malloc(sizeof(struct graph_t));
  g->t = t;
  g->num_vertices = N;
  g->num_edges = t->nz / 2;
  gsl_spmatrix_uint *csr = gsl_spmatrix_uint_compress(g->t, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(g->t);
  g->t = csr;
  g->start[0] = 0;
  g->start[1] = N - 1;
  g->objectives = NULL;
  g->num_objectives = 0;
  g->type = 0;

  free(positions);
  return g;
}

bool is_in_graph_type_C(int col, int row, int m) {
  int q = col;
  int r = row;
  int s = -q - r;
  int dist = fmax(q, fmax(r, s));

  return (dist >= m - 2 && dist < m);
}

struct graph_t *make_graph_type_C(unsigned int m) {
  int N = 12 * m - 18;
  gsl_spmatrix_uint *t = gsl_spmatrix_uint_alloc(N, N);
  struct Position *positions = malloc(sizeof(struct Position) * N);
  positions[0].row = -m + 1;
  positions[0].col = m - 1;
  int count = 1;
  for (int row = -((int)m) + 1; row < (int)m; ++row) {
    for (int col = -((int)m) + 1; col < (int)m; ++col) {
      if (is_in_graph_type_C(col, row, m)) {
        positions[count].row = row;
        positions[count].col = col;
        count++;
      }
    }
  }

  struct {
    int drow, dcol;
    enum dir_t dir;
  } directions[] = {{0, 1, E},   {0, -1, W},  {-1, 0, NW},
                    {-1, 1, NE}, {1, -1, SW}, {1, 0, SE}};

  for (int i = 0; i < N; ++i) {
    int row = positions[i].row;
    int col = positions[i].col;
    for (int k = 0; k < 6; ++k) {
      int r2 = row + directions[k].drow;
      int c2 = col + directions[k].dcol;
      for (int j = 0; j < N; ++j) {
        if (positions[j].row == r2 && positions[j].col == c2) {
          add_bidirectional_edge(t, i, j, directions[k].dir);
          break;
        }
      }
    }
  }

  struct graph_t *g = malloc(sizeof(struct graph_t));
  g->t = t;
  g->num_vertices = N;
  g->num_edges = t->nz / 2;
  gsl_spmatrix_uint *csr = gsl_spmatrix_uint_compress(g->t, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(g->t);
  g->t = csr;
  g->start[0] = 0;
  g->start[1] = N - 1;
  g->objectives = NULL;
  g->num_objectives = 0;
  g->type = 1;
  free(positions);
  return g;
}

bool is_in_graph_type_H(int col, int row, int m) {
  int q = col;
  int r = row;
  struct Position point = (struct Position){r, q};
  struct Position tab_hole_center[7] = {
      (struct Position){0, 0},
      (struct Position){0, 2 * m / 3 - 1},
      (struct Position){0, -2 * m / 3 + 1},
      (struct Position){2 * m / 3 - 1, 0},
      (struct Position){-2 * m / 3 + 1, 0},
      (struct Position){2 * m / 3 - 1, -2 * m / 3 + 1},
      (struct Position){-2 * m / 3 + 1, 2 * m / 3 - 1}};
  struct Position closest_center = {0, 0};
  for (int i = 1; i < 7; i++) {
    if (distance_euclidienne_position(point, tab_hole_center[i]) <
        distance_euclidienne_position(point, closest_center)) {
      closest_center = tab_hole_center[i];
    }
  }
  return distance_euclidienne_position(point, closest_center) >=
             distance_euclidienne_position(tab_hole_center[0],
                                           (struct Position){m / 3 - 1, 0}) &&
         (-m < col && col < m && -m < row && row < m && -m < row + col &&
          row + col < m);
}

struct graph_t *make_graph_type_H(unsigned int m) {
  assert(m % 3 == 0);
  int N = 2 * m * m / 3 + 18 * m - 48;
  gsl_spmatrix_uint *t = gsl_spmatrix_uint_alloc(N, N);
  struct Position *positions = malloc(sizeof(struct Position) * N);
  int count = 0;
  for (int row = -((int)m) + 1; row < (int)m; ++row) {
    for (int col = -((int)m) + 1; col < (int)m; ++col) {
      if (is_in_graph_type_H(col, row, m)) {
        positions[count].row = row;
        positions[count].col = col;
        count++;
      }
    }
  }

  struct {
    int drow, dcol;
    enum dir_t dir;
  } directions[] = {{0, 1, E},   {0, -1, W},  {-1, 0, NW},
                    {-1, 1, NE}, {1, -1, SW}, {1, 0, SE}};

  for (int i = 0; i < N; ++i) {
    int row = positions[i].row;
    int col = positions[i].col;
    for (int k = 0; k < 6; ++k) {
      int r2 = row + directions[k].drow;
      int c2 = col + directions[k].dcol;
      for (int j = 0; j < N; ++j) {
        if (positions[j].row == r2 && positions[j].col == c2) {
          add_bidirectional_edge(t, i, j, directions[k].dir);
          break;
        }
      }
    }
  }

  struct graph_t *g = malloc(sizeof(struct graph_t));
  g->t = t;
  g->num_vertices = N;
  g->num_edges = t->nz / 2;
  gsl_spmatrix_uint *csr = gsl_spmatrix_uint_compress(g->t, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(g->t);
  g->t = csr;
  g->start[0] = 0;
  g->start[1] = N - 1;
  g->objectives = NULL;
  g->num_objectives = 0;
  g->type = 2;

  free(positions);
  return g;
}

void free_graph(struct graph_t *g) {
  if (g) {
    if (g->t)
      gsl_spmatrix_uint_free(g->t);

    if (g->objectives)
      free(g->objectives);

    g->t = NULL;
    g->objectives = NULL;
    g->num_objectives = 0;

    free(g);
  }
}

void print_graph(const struct graph_t *g) {
  for (unsigned int i = 0; i < g->num_vertices; ++i) {
    for (unsigned int j = 0; j < g->num_vertices; ++j) {
      unsigned int val = gsl_spmatrix_uint_get(g->t, i, j);
      if (val != 0) {
        printf("%d -> %d : %s\n", i, j, direction_to_string(val));
      }
    }
  }
}
void starting_graph_random(struct graph_t *g1, struct graph_t *g2) {
  for (int i = 0; i < NUM_PLAYERS; ++i) {
    vertex_t z = g1->num_vertices;
    int a = rand() % z;
    g1->start[i] = a;
    g2->start[i] = a;
  }
}
// fonction qui initialise chaque joueurs au bout du plateau (de facon oppose)
void starting_graph_not_random(struct graph_t *g1, struct graph_t *g2) {
  for (int i = 0; i < NUM_PLAYERS; ++i) {
    g1->start[i] = 0;
    g2->start[i] = g1->num_vertices - 2;
  }
}

void goal_graph_random(struct graph_t *g1, struct graph_t *g2) {
  g1->num_objectives = 3;
  g2->num_objectives = g1->num_objectives;
  g1->objectives = malloc(sizeof(vertex_t) * g1->num_objectives);
  g2->objectives = malloc(sizeof(vertex_t) * g2->num_objectives);

  for (unsigned int i = 0; i < g1->num_objectives; ++i) {
    int a = rand() % g1->num_vertices;
    while ((vertex_t)a == g1->start[0] || (vertex_t)a == g1->start[1]) {
      a = rand() % g1->num_vertices;
    }
    g1->objectives[i] = a;
    g2->objectives[i] = a;
  }
}
// void print_graph_hexagonal(unsigned int m, char type) {

//   // Sélection de la fonction d'appartenance
//   struct graph_t *(*make_graph)(unsigned int m);
//   switch (type) {
//   case 'T':
//     make_graph = make_graph_type_T;
//     break;
//   case 'C':
//     make_graph = make_graph_type_C;
//     break;
//   case 'H':
//     make_graph = make_graph_type_H;
//     break;
//   default:
//     printf("Type inconnu : %c (T, C, H)\n", type);
//     return;
//   }
//   struct graph_t *g = make_graph(m);
//   // extraction de la matrice g->t
//   gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
//       g->t->size1, g->t->size2, g->t->nz, GSL_SPMATRIX_COO);

//   for (size_t row = 0; row < g->t->size1; ++row) {
//     for (int k = g->t->p[row]; k < g->t->p[row + 1]; ++k) {
//       size_t col = g->t->i[k];
//       unsigned int val = g->t->data[k];
//       gsl_spmatrix_uint_set(coo, row, col, val);
//     }
//   }

//   for (unsigned int i = 0; i < g->num_vertices; i++) {
//   }
// }

/*int main() {
  unsigned int m = 15;

   struct graph_t *gt = make_graph_type_T(m);
   struct graph_t *gc = make_graph_type_C(m);
   struct graph_t *gh = make_graph_type_H(m); // m divisible par 3 !

   print_graph(gt);
   free_graph(gt);
   free_graph(gc);
   free_graph(gh);

   return 0;
 }

  printf("\n🔍 Graphe H :\n\n");
   print_graph_hexagonal(gh, m, 'H');

  free_graph(gt);
  free_graph(gc);
  free_graph(gh);


  return 0;
}*/

int axial_to_index(int row, int col, int m) {
  int index = 0;
  for (int r = -m + 1; r < m; r++) {
    for (int c = -m + 1; c < m; c++) {
      if (is_in_graph_type_T(c, r, m)) {
        if (r == row && c == col)
          return index;
        index++;
      }
    }
  }
  return -1; // non trouvé
}

void print_hex_grid(struct graph_t *g) {
  if (!g)
    return;

  int m;
  bool (*is_in_graph)(int col, int row, int m) = NULL;
  switch (g->type) {
  case TRIANGULAR:
    m = (int)((3 + sqrt(12 * g->num_vertices - 3)) / 6);
    is_in_graph = is_in_graph_type_T;
    break;
  case CYCLIC:
    m = (g->num_vertices + 18) / 12;
    is_in_graph = is_in_graph_type_C;
    break;
  case HOLEY:
    m = (int)((-54 + sqrt(24 * g->num_vertices + 4068)) / 4);
    is_in_graph = is_in_graph_type_H;
    break;
  default:
    printf("Graph type inconnu.\n");
    return;
  }

  for (int row = -m + 1; row < m; row++) {
    for (int i = 0; i < row; i++) {
      printf("   ");
    }

    for (int col = -m + 1; col < m; col++) {
      if (is_in_graph(col, row, m)) {
        int index = axial_to_index(row, col, m);
        printf("%3d   ", index); // largeur 6 (3 chiffres + 3 espaces)
      } else {
        printf("   "); // case vide = 6 espaces
      }
    }
    printf("\n");
  }
}

// int main() {
//   srand(time(NULL));

//   unsigned int m = 5;

//   // === Graphe de type T ===
//   struct graph_t *gT = make_graph_type_T(m);
//   starting_graph_random(gT, gT);
//   goal_graph_random(gT, gT);
//   printf("\n" GREEN "--- Graphe de type T (m=%u) ---\n" RESET, m);
//   print_graph(gT);
//   print_hex_grid(gT);
//   free_graph(gT);

//   // === Graphe de type C ===
//   struct graph_t *gC = make_graph_type_C(m);
//   starting_graph_random(gC, gC);
//   goal_graph_random(gC, gC);
//   printf("\n" BLUE "--- Graphe de type C (m=%u) ---\n" RESET, m);
//   print_graph(gC);
//   print_hex_grid(gC);
//   free_graph(gC);

//   // === Graphe de type H ===
//   unsigned int mH = 6; // Doit être divisible par 3 !
//   struct graph_t *gH = make_graph_type_H(mH);
//   starting_graph_random(gH, gH);
//   goal_graph_random(gH, gH);
//   printf("\n" VIOLET "--- Graphe de type H (m=%u) ---\n" RESET, mH);
//   print_graph(gH);
//   print_hex_grid(gH);
//   free_graph(gH);

//   return 0;
// }

void apply_wall_move(struct graph_t *g, struct move_t move) {
  if (!g || !g->t || move.t != WALL)
    return;

  gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
      g->t->size1, g->t->size2, g->t->nz, GSL_SPMATRIX_COO);

  for (size_t row = 0; row < g->t->size1; ++row) {
    for (int idx = g->t->p[row]; idx < g->t->p[row + 1]; ++idx) {
      gsl_spmatrix_uint_set(coo, row, g->t->i[idx], g->t->data[idx]);
    }
  }

  // ➔ Ajout des murs
  for (int i = 0; i < 2; ++i) {
    vertex_t a = move.e[i].fr;
    vertex_t b = move.e[i].to;
    if (a != UINT_MAX && b != UINT_MAX && a < g->t->size1 && b < g->t->size2) {
      gsl_spmatrix_uint_set(coo, a, b, WALL_DIR);
      gsl_spmatrix_uint_set(coo, b, a, WALL_DIR);
    }
  }

  gsl_spmatrix_uint *new_csr =
      gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(coo);
  gsl_spmatrix_uint_free(g->t);
  g->t = new_csr;
}

struct graph_t *make_copy(struct graph_t *g) {
  if (!g || !g->t)
    return NULL;

  struct graph_t *copy = malloc(sizeof(struct graph_t));
  if (!copy)
    return NULL;

  // Copie des champs simples
  copy->num_vertices = g->num_vertices;
  copy->num_edges = g->num_edges;
  copy->type = g->type;
  copy->start[0] = g->start[0];
  copy->start[1] = g->start[1];
  copy->num_objectives = g->num_objectives;

  // Copie ou allocation des objectifs
  if (g->num_objectives > 0) {
    copy->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    if (!copy->objectives) {
      free(copy);
      return NULL;
    }

    if (g->objectives) {
      memcpy(copy->objectives, g->objectives,
             g->num_objectives * sizeof(vertex_t));
    } else {
      // On initialise à 0 si objectifs non définis dans g
      memset(copy->objectives, 0, g->num_objectives * sizeof(vertex_t));
    }
  } else {
    copy->objectives = NULL;
  }

  // Duplication de la matrice t (CSR)
  gsl_spmatrix_uint *coo = gsl_spmatrix_uint_alloc_nzmax(
      g->t->size1, g->t->size2, g->t->nz, GSL_SPMATRIX_COO);
  if (!coo) {
    free(copy->objectives);
    free(copy);
    return NULL;
  }

  for (size_t row = 0; row < g->t->size1; ++row) {
    for (int idx = g->t->p[row]; idx < g->t->p[row + 1]; ++idx) {
      gsl_spmatrix_uint_set(coo, row, g->t->i[idx], g->t->data[idx]);
    }
  }

  copy->t = gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(coo);

  if (!copy->t) {
    free(copy->objectives);
    free(copy);
    return NULL;
  }

  return copy;
}
