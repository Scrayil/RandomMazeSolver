// Copyright (c) 2023. Created by Mattia Bennati, a.k.a Scrayil. All rights reserved.

#ifndef RANDOMMAZESOLVER_SEQUENTIAL_VERSION_H
#define RANDOMMAZESOLVER_SEQUENTIAL_VERSION_H

#include "maze/maze_generation.h"

std::vector<std::vector<MAZE_PATH>> sequential_solution(std::vector<std::vector<MAZE_PATH>> maze, int &size, int n_particles, std::mt19937 generation_rng, std::mt19937 solution_rng, bool show_steps);

#endif //RANDOMMAZESOLVER_SEQUENTIAL_VERSION_H
