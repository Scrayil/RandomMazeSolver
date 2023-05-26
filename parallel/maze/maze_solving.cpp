// Copyright (c) 2023. Created by Mattia Bennati, a.k.a Scrayil. All rights reserved.

#include <random>
#include <iostream>
#include <omp.h>
#include "maze_solving.h"
#include "../../utils/utils.h"


// ENUM AND STRUCTS

/// Supported movements system for particles' shifting.
enum MOVES {
    F = 0, // Used just for particles' initializations (frozen)
    N = 1,
    E = 2,
    S = 3,
    W = 4,
};

/// Supported coordinates system for particles' positioning and movements.
struct Coordinates {
    Coordinates() : row(-1), col(-1) {}
    Coordinates(int c_x, int c_y) : row(c_x), col(c_y) {}

    int row;
    int col;
};


/// Struct used to represent all the particles and handle their movements inside the maze
struct Particles {
    std::vector<Coordinates> positions;
    std::vector<MOVES> moves;
    std::vector<std::vector<Coordinates>> paths;
    int how_many;

    // F is used just for particles' initializations (frozen)
    explicit Particles(int how_many) : how_many(how_many), positions(std::vector<Coordinates>(how_many, Coordinates())), moves(std::vector<MOVES>(how_many, MOVES::F)), paths(std::vector<std::vector<Coordinates>>(how_many, std::vector<Coordinates>(0, Coordinates()))) {}

    /**
     * Allows to add new particles data to the current structure's vectors.
     *
     * This function updates the relative vectors' elements at the corresponding index.
     * @param index This represents the index of the current particle values inside the vectors.
     * @param coord
     */
    void addParticle(int index, Coordinates coord) {
        this->positions[index] = coord;
        this->paths[index].push_back(coord);
    }


    /**
     * This function is used to move the particle.
     *
     * It allows to update the particle's coordinates and path while it moves randomly and also when backtracking
     * it's previous movements.
     * Backtracking is also applied when the particle needs to follow the first particle that exited the maze.
     * @see backtrack_exited_particle.
     *
     * @param index This represents the index of the current particle values inside the vectors.
     * @param new_move This represents the next particle move to implement.
     * @param backtracking This variable indicates if the particle is going forward or backtracking the previous
     * movements contained in the track vector.
     */
    void update_coordinates(int index, MOVES new_move, bool backtracking=false) {
        switch (new_move) {
            case MOVES::N:
                this->positions[index] = Coordinates(this->positions[index].row - 1, this->positions[index].col);
                break;
            case MOVES::S:
                this->positions[index] = Coordinates(this->positions[index].row + 1, this->positions[index].col);
                break;
            case MOVES::E:
                this->positions[index] = Coordinates(this->positions[index].row, this->positions[index].col + 1);
                break;
            case MOVES::W:
                this->positions[index] = Coordinates(this->positions[index].row, this->positions[index].col - 1);
                break;
            case MOVES::F:
                std::cout<< "Unexpected error: the F move is meant only for particles' initialization!" << std::endl;
                exit(1);
        }

        int path_size = static_cast<int>(this->paths[index].size());
        // The first position is always kept
        // Case in which the particle is backtracking or the new set position is the last but one in the track!
        // (eventual random backtrack)
        // This is extremely useful as it frees up memory that would be wasted as some of the contained coordinates are
        // not relevant for the maze's solution
        if((backtracking || path_size > 1 && this->positions[index].row == this->paths[index][path_size - 2].row && this->positions[index].col == this->paths[index][path_size - 2].col))
            this->paths[index].pop_back();
            // Case in which the particle has moved onto a new cell
        else
            this->paths[index].push_back(this->positions[index]);
        this->moves[index] = new_move;
    }
};


// PROTOTYPES
std::vector<std::vector<MAZE_PATH>> p_reach_exit_randomly(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates &initial_position, Particles &particles, std::mt19937 &rng, bool show_steps, bool parallelize);
std::vector<MOVES> p_get_possible_moves(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates curr_particle_pos);
void p_backtrack_exited_particle(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<std::vector<MAZE_PATH>> &maze_copy, Coordinates &initial_position, int &size, Particles &particles, const std::vector<Coordinates>& exited_particle_path, int exited_particle_index, bool show_steps, bool parallelize);
MOVES p_get_next_move_from_path(Coordinates curr_particle_pos, Coordinates &next_coords);


// FUNCTIONS

/**
 * Used to solve the maze, by moving all the particles randomly.
 *
 * After choosing a random spawn point, generates the specified amount of particles, and starts their movements logic.
 * @see p_reach_exit_randomly.
 * @param maze This is the matrix that represents the maze's structure.
 * @param size This value represents each maze's side size.
 * @param n_particles This allows to specify the number of particles to spawn.
 * @param solution_rng This is the random number engine to use in order to generate random values.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 * @return the matrix that represents the maze's inner structure along with the solution path.
 */
std::vector<std::vector<MAZE_PATH>> p_solve(std::vector<std::vector<MAZE_PATH>> maze, int size, int n_particles, std::mt19937 solution_rng, bool show_steps, bool parallelize) {
    // Choosing a random starting position
    std::uniform_int_distribution<int> uniform_dist(size / 6, 5 * size / 6); // Guaranteed unbiased

    // Selects an initial position
    Coordinates initial_position = Coordinates(uniform_dist(solution_rng), uniform_dist(solution_rng));
    while(maze[initial_position.row][initial_position.col] != MAZE_PATH::EMPTY)
        initial_position = Coordinates(uniform_dist(solution_rng), uniform_dist(solution_rng));

    // SoAoS
    Particles particles(n_particles);

    #pragma omp parallel for if(parallelize)
    for(int i = 0; i < n_particles; i++) {
        particles.addParticle(i, initial_position);
    }

    // Initial point always shown
    // Adds the particles to the maze copy in order to show them
    std::vector<std::vector<MAZE_PATH>> maze_copy = maze;
    maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
    display_ascii_maze(maze_copy, size);

    std::cout << "Solving the maze.." << std::endl;

    // Starts the solving procedure
    return p_reach_exit_randomly(maze, size, initial_position, particles, solution_rng, show_steps, parallelize);
}

/**
 * This function is responsible for moving the particles inside the maze.
 *
 * All the particles move around randomly until one of them reaches the maze's exit.
 * Once this happen all the remaining particles start backtracking their own steps.
 * @param maze This is the matrix that represents the maze's structure.
 * @param size This value represents each maze's side size.
 * @param initial_position These are the coordinates that represents the spawn position of all the particles.
 * @param particles This is the structure that contains all the particles' vectors.
 * @param rng This is the random number engine to use in order to generate random values.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 * @return the matrix that represents the maze's inner structure along with the solution path.
 */
std::vector<std::vector<MAZE_PATH>> p_reach_exit_randomly(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates &initial_position, Particles &particles, std::mt19937 &rng, bool show_steps, bool parallelize) {
    bool exit_reached = false;
    int exited_particle_index = -1;
    std::vector<std::vector<MAZE_PATH>> maze_copy;

    while(!exit_reached) {
        if(show_steps)
            // Copies the maze to show the particles positions
            maze_copy = maze;

        #pragma omp parallel for if(parallelize)
        for(int index = 0; index < particles.how_many; index++) {
            if(!exit_reached) {
                 std::vector<MOVES> moves = p_get_possible_moves(maze, size, particles.positions[index]);
                bool same_move_allowed = false;
                for(MOVES move : moves) {
                    // Keeps going on if it can go only on opposite directions
                    if(moves.size() == 2 && particles.moves[index] == move) {
                        same_move_allowed = true;
                        particles.update_coordinates(index, move);
                        break;
                    }
                }
                // The same move wasn't available because of the walls nearby
                if(!same_move_allowed) {
                    // Choosing a random move
                    std::uniform_int_distribution<int> uniform_dist(0, static_cast<int>(moves.size()) - 1); // Guaranteed unbiased
                    MOVES new_move = moves[uniform_dist(rng)];
                    particles.update_coordinates(index, new_move);
                }

                if(show_steps) {
                    // Add the particles to the maze_copy
                    maze_copy[particles.positions[index].row][particles.positions[index].col] = MAZE_PATH::PARTICLE;
                    #pragma omp critical
                    // Shows the start everytime
                    maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
                }

                // The particle has reached the exit
                if(maze[particles.positions[index].row][particles.positions[index].col] == MAZE_PATH::EXIT) {
                    #pragma omp critical
                    {
                        exited_particle_index = index;
                        exit_reached = true;
                    };
                }
            }
        }

        if(show_steps && !maze_copy.empty())
            // Displays the particles in the maze
            display_ascii_maze(maze_copy, size);

    }

    std::vector<Coordinates> exited_particle_path = particles.paths[exited_particle_index];

    #pragma omp parallel for if(exited_particle_path.size() / omp_get_max_threads() > 100)
    // Shows the maze's path that lead to the solution
    for(Coordinates coord : exited_particle_path) {
        maze[coord.row][coord.col] = MAZE_PATH::SOLUTION;
    }
    maze[initial_position.row][initial_position.col] = MAZE_PATH::START;

    // Shows the maze with the solution path
    display_ascii_maze(maze, size);

    std::cout << "Exit reached!" << std::endl;
    std::cout << "Backtracking the exited particle.." << std::endl;

    // Backtracking the first particle that went out
    p_backtrack_exited_particle(maze, maze_copy, initial_position, size, particles, exited_particle_path, exited_particle_index, show_steps, parallelize);


    std::cout << "All particles have reached the exit!" << std::endl;

    return maze;
}


/**
 * This function is used to determine the available moves for the current particle.
 *
 * The moves are selected based onto the state of the cells nearby in the maze.
 * If a near cell does not contain a wall, then it represent a possible move.
 * @param maze This is the matrix that represents the maze's structure.
 * @param size This value represents each maze's side size.
 * @param curr_particle_pos This is the position of the current particle for which the moves are being evaluated.
 * @return A vector of moves representing the next possible shifts of the current particle.
 */
std::vector<MOVES> p_get_possible_moves(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates curr_particle_pos) {
    std::vector<MOVES> moves;
    moves.reserve(4);

    if(-1 < curr_particle_pos.row - 1 < size && maze[curr_particle_pos.row - 1][curr_particle_pos.col] != MAZE_PATH::WALL)
        moves.push_back(MOVES::N);

    if(-1 < curr_particle_pos.row + 1 < size && maze[curr_particle_pos.row + 1][curr_particle_pos.col] != MAZE_PATH::WALL)
        moves.push_back(MOVES::S);

    if(-1 < curr_particle_pos.col - 1 < size && maze[curr_particle_pos.row][curr_particle_pos.col - 1] != MAZE_PATH::WALL)
        moves.push_back(MOVES::W);

    if(-1 < curr_particle_pos.col + 1 < size && maze[curr_particle_pos.row][curr_particle_pos.col + 1] != MAZE_PATH::WALL)
        moves.push_back(MOVES::E);

    return moves;
}

/**
 * This function is called once a particle has managed to exit the maze.
 *
 * The remaining particles backtrack their own previous steps until they end up onto the solution's path.
 * Once they are on the right track, their path is replaced by the remaining steps followed by the exited particle and
 * they starts to backtrack them until they exit.
 * @param maze This is the matrix that represents the maze's structure.
 * @param maze_copy This matrix is used only for printing purposes to not alter the maze itself.
 * @param initial_position These are the coordinates that represents the spawn position of all the particles.
 * @param size This value represents each maze's side size.
 * @param particles This is the structure that contains all the particles' vectors.
 * @param exited_particle_path This vector contains the steps required to reach the exit starting from the initial position.
 * @param exited_particle_index This integer number represents the exited particle's index related to the particles' vector.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 */
void p_backtrack_exited_particle(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<std::vector<MAZE_PATH>> &maze_copy, Coordinates &initial_position, int &size, Particles &particles, const std::vector<Coordinates>& exited_particle_path, int exited_particle_index, bool show_steps, bool parallelize) {
    int n_particles = static_cast<int>(particles.how_many);
    std::vector<bool> particles_on_track_map;
    particles_on_track_map.reserve(n_particles);
    std::vector<bool> exited_particles_map;
    exited_particles_map.reserve(n_particles);

    #pragma omp parallel for if(parallelize)
    // Initializes the 2 maps elements to false
    for(int index = 0; index < n_particles; index++) {
        particles_on_track_map[index]= false;
        exited_particles_map[index] = false;
    }

    particles_on_track_map[exited_particle_index] = true;
    exited_particles_map[exited_particle_index] = true;

    int n_exited_particles = 1;
    maze_copy.clear();

    while(n_exited_particles < particles.how_many) {
        if(show_steps) {
            // Resets the maze to show the steps
            maze_copy = maze;
            maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
        }

        #pragma omp parallel for if(parallelize)
        // Backtracking the particles movements until they are on the solution path
        // After that they follow the first exited particle's movements
        for(int particle_index = 0; particle_index < n_particles; particle_index++) {
            if(!exited_particles_map[particle_index]) {
                // If the particle wasn't on the solution path in the previous iteration checks if it is now
                if(!particles_on_track_map[particle_index]) {
                    for(int path_index = 0; path_index < exited_particle_path.size(); path_index++) {
                        Coordinates coord = exited_particle_path[path_index];
                        // The particle is now onto the right track
                        if(particles.positions[particle_index].row == coord.row && particles.positions[particle_index].col == coord.col) {
                            particles_on_track_map[particle_index] = true;
                            // Since indexes start from 0 an index of 5 means that we need to skip 6 elements, so + 1
                            // + 2 in the end as we need to skip the current position (already equal to the path index's one)
                            particles.paths[particle_index].clear();
                            int remaining_path_size = static_cast<int>(exited_particle_path.size());
                            particles.paths[particle_index].reserve(remaining_path_size);

                            // Assigns the remaining correct steps to the current particle, in reverse order (useful for
                            // using the "update_coordinates" function as it is)
                            for(int index = remaining_path_size - 1; index > path_index; index--) {
                                particles.paths[particle_index].push_back(exited_particle_path[index]);
                            }
                            break;
                        }
                    }
                }

                // Following the particle's steps back
                if(!particles.paths[particle_index].empty()) {
                    Coordinates next_coords = particles.paths[particle_index].back();
                    if(next_coords.row == particles.positions[particle_index].row && next_coords.col == particles.positions[particle_index].col) {
                        particles.paths[particle_index].pop_back();
                        next_coords = particles.paths[particle_index].back();
                    }
                    particles.update_coordinates(particle_index, p_get_next_move_from_path(particles.positions[particle_index], next_coords), true);

                    // Displays the particle's steps
                    if(show_steps) {
                        maze_copy[particles.positions[particle_index].row][particles.positions[particle_index].col] = MAZE_PATH::PARTICLE;
                    }
                } else if (!exited_particles_map[particle_index]){
                    #pragma omp critical
                    n_exited_particles += 1;
                    exited_particles_map[particle_index] = true;
                }
            } else {
                // Displays the particle's position
                if(show_steps) {
                    maze_copy[particles.positions[particle_index].row][particles.positions[particle_index].col] = MAZE_PATH::PARTICLE;
                }
            }
        }

        if(show_steps)
            display_ascii_maze(maze_copy, size);
    }
//    for(Particle particle : particles) {
//        std::cout << particle.pos.row << " nn " << particle.pos.col << std::endl;
//        if(particle.pos.row != exited_particle_path.back().row || particle.pos.col != exited_particle_path.back().col) {
//            std::cout << "Wrong" << std::endl;
//            exit(0);
//        }
//    }
}


/**
 * This function is used while backtracking in order to get the next move for the current particle while backtracking.
 *
 * It checks the difference between the current particle's position and it's new coordinates in order to discover
 * it's next move.
 * @param curr_particle_pos This is the position of the current particle for which the move is being evaluated.
 * @param next_coords This are the next particle's coordinates.
 * @return The move that the particle has to perform in order to keep backtracking and reach the exit.
 */
MOVES p_get_next_move_from_path(Coordinates curr_particle_pos, Coordinates &next_coords) {
    if(next_coords.row == curr_particle_pos.row)
        // The next position is to the right
        if(next_coords.col > curr_particle_pos.col)
            return MOVES::E;
        // The next position i on the left
        else
            return MOVES::W;
    else
        // The next position is below
        if(next_coords.row > curr_particle_pos.row)
            return MOVES::S;
        // The next position is above
        else
            return MOVES::N;
}