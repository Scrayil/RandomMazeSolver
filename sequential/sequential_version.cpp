//
// Created by scrayil on 08/05/23.
//

#include <iostream>
#include "json.hpp"
#include <random>

#include "maze/maze_generation.h"
#include "maze/maze_solving.h"


int sequential_solution(int &size, int n_particles, std::mt19937 generation_rng, std::mt19937 solution_rng, bool show_steps) {
    // Generates a square maze with the specified width and height if given
    // Otherwise a random maze is generated
    std::vector<std::vector<MAZE_PATH>> maze = generate_square_maze(size, generation_rng);

    solve(maze, size, n_particles, solution_rng, show_steps);

    return 0;
}
