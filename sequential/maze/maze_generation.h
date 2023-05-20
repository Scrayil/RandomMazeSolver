// Copyright (c) 2023. Created by Mattia Bennati, a.k.a Scrayil. All rights reserved.

#ifndef RANDOMMAZESOLVER_MAZE_GENERATION_H
#define RANDOMMAZESOLVER_MAZE_GENERATION_H

#include <vector>
#include <random>

// ENUMS AND STRUCTS

/** This structure is used in order to determine the maze's inner structure and
 * the available cells for the particles' movement.
 */
enum MAZE_PATH
{
    EMPTY = 0x0,
    WALL = 0x1,
    EXIT = 0x2,
    SOLUTION = 0x3,
    START = 0x4,
    PARTICLE = 0x5,
};

void generate_square_maze(std::vector<std::vector<MAZE_PATH>> &maze, int &size, std::mt19937 generation_rng);

#endif //RANDOMMAZESOLVER_MAZE_GENERATION_H