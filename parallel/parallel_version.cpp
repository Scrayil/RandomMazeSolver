//
// Created by scrayil on 08/05/23.
//

#include <iostream>
#include <random>
#include <omp.h>

#include "../parallel/maze/maze_generation.h"
#include "../parallel/maze/maze_solving.h"


// FUNCTIONS

/**
 * This function is the sequential access point used to generate the maze and solve it randomly.
 *
 * @param maze This is the matrix that represents the maze's structure.
 * @param size This value represents each maze's side size.
 * @param n_particles This allows to specify the number of particles to spawn.
 * @param generation_rng This is the random number engine to use for the random generation values.
 * @param solution_rng This is the random number engine to use for the random solution values.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @return the matrix that represents the maze's inner structure along with the solution path.
 */
std::vector<std::vector<MAZE_PATH>> parallel_solution(std::vector<std::vector<MAZE_PATH>> maze, int &size, int n_particles, std::mt19937 generation_rng, std::mt19937 solution_rng, bool show_steps) {
    bool parallelize = false;

    #ifdef _OPENMP
        if(!omp_get_nested())
            omp_set_nested(true);
        if(omp_get_max_active_levels() < 2)
            omp_set_max_active_levels(2);

        if(size * size / static_cast<int>(omp_get_max_threads()) > 100) {
            parallelize = true;
            std::cout << "Parallelizing the generation!" << std::endl;
        }
    #endif

    // Generates a square maze with the specified width and height if given
    // Otherwise a random maze is generated
    p_generate_square_maze(maze, size, generation_rng, parallelize);

    #ifdef _OPENMP
        parallelize = false;
        if(n_particles / omp_get_max_threads() > 100) {
            parallelize = true;
            std::cout << "Parallelizing the solution!" << std::endl;
        }
    #endif

    return p_solve(maze, size, n_particles, solution_rng, show_steps, parallelize);
}