#ifndef _CORS_PLAYER_H_
#define _CORS_PLAYER_H_

#include "graph.h"
#include "move.h"

/* Access to player informations
 * RETURNS:
 * - the player name as an [a-zA-Z0-9 -_]* string
 */
char const *get_player_name();

/* Player initialization. Is called once before the start of the game.
 * PARAM:
 * - player_id:    the id of the player, between 0 and NUM_PLAYERS-1
 * - graph:        the graph on which the game is played
 * PRECOND:
 * - `id` is either 0 or 1
 * - `graph` contains an adjacency matrix for the graph of the board,
 *   and this matrix is the responsibility of the player, and in
 *   particular must be freed in `finalize`
 */
void initialize(unsigned int player_id, struct graph_t *graph);

/* Computes next move
 * PARAM:
 * - previous_move: the move from the previous player. If this is the
 *                  first move of the game, the move type is equal to NO_TYPE
 * RETURNS:
 * - the next move for the player.
 */
struct move_t play(const struct move_t previous_move);

/* Clean up the resources the player has been using. Is called once at
   the end of the game.
 * POSTCOND:
 * - every allocation done during the calls to `initialize` and `play`
 *   functions must have been freed
 */
void finalize();

#endif // _CORS_PLAYER_H_
