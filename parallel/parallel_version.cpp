//
// Created by scrayil on 08/05/23.
//

#include <iostream>
#include <random>

#include "../sequential/maze/maze_generation.h"
#include "../sequential/maze/maze_solving.h"


// FUNCTIONS

/**
 * This function is the sequential access point used to generate the maze and solve it randomly.
 *
 * @param size This value represents each maze's side size.
 * @param n_particles This allows to specify the number of particles to spawn.
 * @param generation_rng This is the random number engine to use for the random generation values.
 * @param solution_rng This is the random number engine to use for the random solution values.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @return The maze's matrix with the solution path.
 */
std::vector<std::vector<MAZE_PATH>> parallel_solution(std::vector<std::vector<MAZE_PATH>> maze, int &size, int n_particles, std::mt19937 generation_rng, std::mt19937 solution_rng, bool show_steps) {
    // Generates a square maze with the specified width and height if given
    // Otherwise a random maze is generated
    generate_square_maze(maze, size, generation_rng);

    return solve(maze, size, n_particles, solution_rng, show_steps);
}