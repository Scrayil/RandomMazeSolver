// Copyright (c) 2023. Created by Mattia Bennati, a.k.a Scrayil. All rights reserved.

#include <iostream>
#include <random>

#include "maze/maze_solving.h"


/**
 * This function is the sequential access point used to generate the maze and solve it randomly.
 *
 * @param maze It's the matrix representing the maze that is being generated.
 * @param size This value represents each maze's side size.
 * @param n_particles This allows to specify the number of particles to spawn.
 * @param generation_rng This is the random number engine to use for the random generation values.
 * @param solution_rng This is the random number engine to use for the random solution values.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @return This is the matrix that represents the maze's inner structure along with the solution path.
 */
std::vector<std::vector<MAZE_PATH>> sequential_solution(std::vector<std::vector<MAZE_PATH>> maze, int &size, int n_particles, std::mt19937 generation_rng, std::mt19937 solution_rng, bool show_steps) {
    // Generates a square maze with the specified width and height if given
    // Otherwise a random maze is generated
    generate_square_maze(maze, size, generation_rng, show_steps);

    return solve(maze, size, n_particles, solution_rng, show_steps);
}
