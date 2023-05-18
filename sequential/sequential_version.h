//
// Created by scrayil on 13/05/23.
//

#ifndef RANDOMMAZESOLVER_SEQUENTIAL_VERSION_H
#define RANDOMMAZESOLVER_SEQUENTIAL_VERSION_H

#include "maze/maze_generation.h"

std::vector<std::vector<MAZE_PATH>> sequential_solution(std::vector<std::vector<MAZE_PATH>> maze, int &size, int n_particles, std::mt19937 generation_rng, std::mt19937 solution_rng, bool show_steps);

#endif //RANDOMMAZESOLVER_SEQUENTIAL_VERSION_H
