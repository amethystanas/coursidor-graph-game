# Coursidor

Coursidor is a C implementation of a graph-based variant of the board game Quoridor. The project provides a game server and autonomous players that compete on different hexagon-shaped boards while following distinct strategies. 

Two players move pawns on a hexagonal graph where each vertex is a cell and each edge is a possible move. On each turn, a player can either move according to specific movement rules (including multi-step moves and jumps) or place a wall that removes edges from the graph to slow down the opponent without completely blocking them. Players must visit all objective vertices at least once and then return to their starting position to win the game. 

The board is modeled as a sparse graph using GNU Scientific Library (GSL) matrices in CSR/COO formats, with three possible topologies: triangular (T), cyclic/empty hexagon (C), and holed hexagon (H). A modular architecture separates the game server, graph algorithms, movement utilities, and player strategies, making the code easier to extend and test. 

Two main AI strategies are implemented: `smartayke.c`, which uses a Minimax algorithm with alpha–beta pruning on a scenario tree to anticipate opponent moves, and `player_wiwi.c`, a reactive strategy that prioritizes visiting remaining objectives using Dijkstra-based shortest paths while placing walls proactively to hinder the opponent. Both strategies use a set of utility functions for safe movement, wall validation, and objective management. 

Extensive unit tests cover core components such as graph construction, movement validation, wall placement, and pathfinding, using custom test scenarios and assertions. Additional functional tests were used to validate complex behaviors like multi-step moves and non-blocking wall placements. 

## Build

The project is built with a `Makefile` that compiles the server, player strategies (as shared libraries), and tests, and installs the binaries into an `install/` directory. 

Common targets:

- `make install`: compile and install the server and players into `install/`
- `make clean`: remove generated build files
- `make test`: run unit tests
- `make coverage`: generate a test coverage report
- `make full-coverage`: rebuild everything with coverage enabled 

## Usage

After `make install`, you can start a game with:

```bash
./install/server [-m M] [-s S] [-M NB] ./install/player1.so ./install/player2.so

./install/server -m 5 -s 10 -M 200 ./install/smartayke.so ./install/player_wiwi.so
```
Options:

-m to specify the graph width (board size)

-s to specify a seed

-M to specify the maximum number of turns
