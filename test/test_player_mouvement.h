#ifndef _CORS_TEST_T_T_PLAYER_MOUVEMENT_H_
#define _CORS_TEST_T_T_PLAYER_MOUVEMENT_H_
#include "graph_utils.h"
#include "move_utils.h"
#include "player_mouvement.h"
#include <stdio.h>

struct test_result;

void test_move_in_same_direction();

void test_move_in_30_deg_direction_OUEST();

void test_move_in_30_deg_direction_EST();

void test_move_to_neighbour();

void test_is_valid_move();

void test_is_edge_blocked();

void test_can_jump();

void test_can_place_wall();

void test_place_wall();

void test_opponnent_front_obj();
#endif
