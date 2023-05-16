//
// Created by scrayil on 12/05/23.
//

#ifndef RANDOMMAZESOLVER_MAZE_SOLVING_H
#define RANDOMMAZESOLVER_MAZE_SOLVING_H

#include <vector>
#include "maze_generation.h"

void solve(std::vector<std::vector<MAZE_PATH>> maze, int size, int n_particles, std::mt19937 solution_rng, bool show_steps);

#endif //RANDOMMAZESOLVER_MAZE_SOLVING_H