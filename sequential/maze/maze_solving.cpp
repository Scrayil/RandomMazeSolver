//
// Created by scrayil on 12/05/23.
//

#include <random>
#include <iostream>
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
    Coordinates(int c_x, int c_y) : row(c_x), col(c_y) {}

    int row;
    int col;
};


/// Struct used to represent a particle and handle it's movements inside the maze
struct Particle {
    // F is used just for particles' initializations (frozen)
    explicit Particle(Coordinates coords) : pos(coords), move(MOVES::F) {
        this->path.push_back(coords);
    }

    /**
     * This function is used to move the particle.
     *
     * It allows to update the particle's coordinates and path while it moves randomly and also when backtracking
     * it's previous movements.
     * Backtracking is also applied when the particle needs to follow the first particle that exited the maze.
     * @see backtrack_exited_particle.
     *
     * @param new_move This represents the next particle move to implement.
     * @param backtracking This variable indicates if the particle is going forward or backtracking the previous
     * movements contained in the track vector.
     */
    void update_coordinates(MOVES new_move, bool backtracking=false) {
        switch (new_move) {
            case MOVES::N:
                this->pos = Coordinates(this->pos.row - 1, this->pos.col);
                break;
            case MOVES::S:
                this->pos = Coordinates(this->pos.row + 1, this->pos.col);
                break;
            case MOVES::E:
                this->pos = Coordinates(this->pos.row, this->pos.col + 1);
                break;
            case MOVES::W:
                this->pos = Coordinates(this->pos.row, this->pos.col - 1);
                break;
            case MOVES::F:
                std::cout<< "Unexpected error: the F move is meant only for particles' initialization!" << std::endl;
                exit(1);
        }

        int path_size = static_cast<int>(this->path.size());
        // The first position is always kept
        // Case in which the particle is backtracking or the new set position is the last but one in the track!
        // (eventual random backtrack)
        // This is extremely useful as it frees up memory that would be wasted as some of the contained coordinates are
        // not relevant for the maze's solution
        if((backtracking || path_size > 1 && this->pos.row == this->path[path_size - 2].row && this->pos.col == this->path[path_size - 2].col))
            this->path.pop_back();
        // Case in which the particle has moved onto a new cell
        else
            this->path.push_back(this->pos);
        this->move = new_move;
    }

    Coordinates pos;
    std::vector<Coordinates> path;
    MOVES move;
};


// PROTOTYPES
std::vector<std::vector<MAZE_PATH>> reach_exit_randomly(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates &initial_position, std::vector<Particle> &particles, std::mt19937 &rng, bool show_steps);
std::vector<MOVES> get_possible_moves(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Particle &curr_particle);
void backtrack_exited_particle(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<std::vector<MAZE_PATH>> &maze_copy, Coordinates &initial_position, int &size, std::vector<Particle> &particles, const std::vector<Coordinates>& exited_particle_path, int exited_particle_index, bool show_steps);
MOVES get_next_move_from_path(Particle &particle, Coordinates &next_coords);


// FUNCTIONS

/**
 * Used to solve the maze, by moving all the particles randomly.
 *
 * After choosing a random spawn point, generates the specified amount of particles, and starts their movements logic.
 * @see reach_exit_randomly.
 * @param maze This is the matrix that represents the maze's structure.
 * @param size This value represents each maze's side size.
 * @param n_particles This allows to specify the number of particles to spawn.
 * @param solution_rng This is the random number engine to use in order to generate random values.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @return the matrix that represents the maze's inner structure along with the solution path.
 */
std::vector<std::vector<MAZE_PATH>> solve(std::vector<std::vector<MAZE_PATH>> maze, int size, int n_particles, std::mt19937 solution_rng, bool show_steps) {
    // Choosing a random starting position
    std::uniform_int_distribution<int> uniform_dist(size / 6, 5 * size / 6); // Guaranteed unbiased

    // Selects an initial position
    Coordinates initial_position = Coordinates(uniform_dist(solution_rng), uniform_dist(solution_rng));
    while(maze[initial_position.row][initial_position.col] != MAZE_PATH::EMPTY)
        initial_position = Coordinates(uniform_dist(solution_rng), uniform_dist(solution_rng));

    // AoS
    std::vector<Particle> particles;
    particles.reserve(n_particles);

    for(int i = 0; i < n_particles; i++) {
        Particle particle = Particle(initial_position);
        particles.push_back(particle);
    }

    // Initial point always shown
    // Adds the particles to the maze copy in order to show them
    std::vector<std::vector<MAZE_PATH>> maze_copy = maze;
    maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
    display_ascii_maze(maze_copy, size);

    std::cout << "Solving the maze.." << std::endl;

    // Starts the solving procedure
    return reach_exit_randomly(maze, size, initial_position, particles, solution_rng, show_steps);
}

/**
 * This function is responsible for moving the particles inside the maze.
 *
 * All the particles move around randomly until one of them reaches the maze's exit.
 * Once this happen all the remaining particles start backtracking their own steps.
 * @param maze This is the matrix that represents the maze's structure.
 * @param size This value represents each maze's side size.
 * @param initial_position These are the coordinates that represents the spawn position of all the particles.
 * @param particles This is the vector that contains all the particles.
 * @param rng This is the random number engine to use in order to generate random values.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 * @return the matrix that represents the maze's inner structure along with the solution path.
 */
std::vector<std::vector<MAZE_PATH>> reach_exit_randomly(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates &initial_position, std::vector<Particle> &particles, std::mt19937 &rng, bool show_steps) {
    bool exit_reached = false;
    int exited_particle_index = -1;
    std::vector<std::vector<MAZE_PATH>> maze_copy;

    while(!exit_reached) {
        if(show_steps)
            // Copies the maze to show the particles positions
            maze_copy = maze;

        for(int index = 0; index < particles.size(); index++) {
            Particle curr_particle = particles[index];

            std::vector<MOVES> moves = get_possible_moves(maze, size, curr_particle);
            bool same_move_allowed = false;
            for(MOVES move : moves) {
                // Keeps going on if it can go only on opposite directions
                if(moves.size() == 2 && curr_particle.move == move) {
                    same_move_allowed = true;
                    curr_particle.update_coordinates(move);
                    break;
                }
            }
            // The same move wasn't available because of the walls nearby
            if(!same_move_allowed) {
                // Choosing a random move
                std::uniform_int_distribution<int> uniform_dist(0, static_cast<int>(moves.size()) - 1); // Guaranteed unbiased
                MOVES new_move = moves[uniform_dist(rng)];
                curr_particle.update_coordinates(new_move);
            }

            // Updates the particle in the vector
            particles[index] = curr_particle;

            if(show_steps) {
                // Add the particles to the maze_copy
                maze_copy[curr_particle.pos.row][curr_particle.pos.col] = MAZE_PATH::PARTICLE;
                // Shows the start everytime
                maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
            }

            // The particle has reached the exit
            if(maze[curr_particle.pos.row][curr_particle.pos.col] == MAZE_PATH::EXIT) {
                exited_particle_index = index;
                exit_reached = true;
                break;
            }
        }

        if(show_steps && !maze_copy.empty())
            // Displays the particles in the maze
            display_ascii_maze(maze_copy, size);

    }

    std::vector<Coordinates> exited_particle_path = particles[exited_particle_index].path;

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
    backtrack_exited_particle(maze, maze_copy, initial_position, size, particles, exited_particle_path, exited_particle_index, show_steps);


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
 * @param curr_particle This is the particle for which the moves are being evaluated.
 * @return A vector of moves representing the next possible shifts of the current particle.
 */
std::vector<MOVES> get_possible_moves(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Particle &curr_particle) {
    std::vector<MOVES> moves;
    moves.reserve(4);

    if(-1 < curr_particle.pos.row - 1 < size && maze[curr_particle.pos.row - 1][curr_particle.pos.col] != MAZE_PATH::WALL)
        moves.push_back(MOVES::N);

    if(-1 < curr_particle.pos.row + 1 < size && maze[curr_particle.pos.row + 1][curr_particle.pos.col] != MAZE_PATH::WALL)
        moves.push_back(MOVES::S);

    if(-1 < curr_particle.pos.col - 1 < size && maze[curr_particle.pos.row][curr_particle.pos.col - 1] != MAZE_PATH::WALL)
        moves.push_back(MOVES::W);

    if(-1 < curr_particle.pos.col + 1 < size && maze[curr_particle.pos.row][curr_particle.pos.col + 1] != MAZE_PATH::WALL)
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
 * @param particles This is the vector that contains all the particles.
 * @param exited_particle_path This vector contains the steps required to reach the exit starting from the initial position.
 * @param exited_particle_index This integer number represents the exited particle's index related to the particles' vector.
 * @param show_steps Flag used to determine if each movement step must be shown on screen.
 */
void backtrack_exited_particle(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<std::vector<MAZE_PATH>> &maze_copy, Coordinates &initial_position, int &size, std::vector<Particle> &particles, const std::vector<Coordinates>& exited_particle_path, int exited_particle_index, bool show_steps) {
    int n_particles = static_cast<int>(particles.size());
    std::vector<bool> particles_on_track_map;
    particles_on_track_map.reserve(n_particles);
    std::vector<bool> exited_particles_map;
    exited_particles_map.reserve(n_particles);

    // Initializes the 2 maps elements to false
    for(int index = 0; index < n_particles; index++) {
        particles_on_track_map.push_back(false);
        exited_particles_map.push_back(false);
    }

    particles_on_track_map[exited_particle_index] = true;
    exited_particles_map[exited_particle_index] = true;

    int n_exited_particles = 1;
    maze_copy.clear();

    while(n_exited_particles < particles.size()) {
        if(show_steps)
            // Resets the maze to show the steps
            maze_copy = maze;

        // Backtracking the particles movements until they are on the solution path
        // After that they follow the first exited particle's movements
        for(int particle_index = 0; particle_index < particles.size(); particle_index++) {
            if(!exited_particles_map[particle_index]) {
                Particle particle = particles[particle_index];
                // If the particle wasn't on the solution path in the previous iteration checks if it is now
                if(!particles_on_track_map[particle_index]) {
                    for(int path_index = 0; path_index < exited_particle_path.size(); path_index++) {
                        Coordinates coord = exited_particle_path[path_index];
                        // The particle is now onto the right track
                        if(particle.pos.row == coord.row && particle.pos.col == coord.col) {
                            particles_on_track_map[particle_index] = true;
                            // Since indexes start from 0 an index of 5 means that we need to skip 6 elements, so + 1
                            // + 2 in the end as we need to skip the current position (already equal to the path index's one)
                            particle.path.clear();
                            int remaining_path_size = static_cast<int>(exited_particle_path.size());
                            particle.path.reserve(remaining_path_size);

                            // Assigns the remaining correct steps to the current particle, in reverse order (useful for
                            // using the "update_coordinates" function as it is)
                            for(int index = remaining_path_size - 1; index > path_index; index--) {
                                particle.path.push_back(exited_particle_path[index]);
                            }

                            particles[particle_index] = particle;
                            break;
                        }
                    }
                }

                // Following the particle's steps back
                if(!particle.path.empty()) {
                    Coordinates next_coords = particle.path.back();
                    if(next_coords.row == particle.pos.row && next_coords.col == particle.pos.col) {
                        particle.path.pop_back();
                        next_coords = particle.path.back();
                    }
                    particle.update_coordinates(get_next_move_from_path(particle, next_coords), true);
                    particles[particle_index] = particle;

                    // Displays the particle's steps
                    if(show_steps && !maze_copy.empty()) {
                        maze_copy[particle.pos.row][particle.pos.col] = MAZE_PATH::PARTICLE;
                        maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
                        display_ascii_maze(maze_copy, size);
                    }
                } else if (!exited_particles_map[particle_index]){
                    n_exited_particles += 1;
                    exited_particles_map[particle_index] = true;
                }
            }
        }
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
 * @param particle This is the particle for which the move is being evaluated.
 * @param next_coords This are the next particle's coordinates.
 * @return The move that the particle has to perform in order to keep backtrcking and reach the exit.
 */
MOVES get_next_move_from_path(Particle &particle, Coordinates &next_coords) {
    if(next_coords.row == particle.pos.row)
        // The next position is to the right
        if(next_coords.col > particle.pos.col)
            return MOVES::E;
        // The next position i on the left
        else
            return MOVES::W;
    else
        // The next position is below
        if(next_coords.row > particle.pos.row)
            return MOVES::S;
        // The next position is above
        else
            return MOVES::N;
}