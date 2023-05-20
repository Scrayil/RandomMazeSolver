// Copyright (c) 2023. Created by Mattia Bennati, a.k.a Scrayil. All rights reserved.

#ifndef RANDOMMAZESOLVER_MAZE_SOLVING_H
#define RANDOMMAZESOLVER_MAZE_SOLVING_H

#include <vector>
#include "maze_generation.h"

std::vector<std::vector<MAZE_PATH>> solve(std::vector<std::vector<MAZE_PATH>> maze, int size, int n_particles, std::mt19937 solution_rng, bool show_steps);

#endif //RANDOMMAZESOLVER_MAZE_SOLVING_H
